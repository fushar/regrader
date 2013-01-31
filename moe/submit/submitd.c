/*
 *  The Submit Daemon
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/getopt.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "submitd.h"

/*** CONFIGURATION ***/

static char *log_name;
static uns port = 8888;
static uns dh_bits = 1024;
static uns max_conn = 10;
static uns session_timeout;
uns max_versions;
static char *ca_cert_name = "?";
static char *server_cert_name = "?";
static char *server_key_name = "?";
char *history_format;
static clist access_rules;
static uns trace_tls;
uns max_request_size;
uns max_attachment_size;
uns trace_commands;

static struct cf_section ip_node_conf = {
  CF_TYPE(struct ip_node),
  CF_ITEMS {
    CF_USER("IP", PTR_TO(struct ip_node, addrmask), &ip_addrmask_type),
    CF_END
  }
};

static struct cf_section access_conf = {
  CF_TYPE(struct access_rule),
  CF_ITEMS {
    CF_LIST("IP", PTR_TO(struct access_rule, ip_list), &ip_node_conf),
    CF_UNS("Admin", PTR_TO(struct access_rule, allow_admin)),
    CF_UNS("PlainText", PTR_TO(struct access_rule, plain_text)),
    CF_UNS("MaxConn", PTR_TO(struct access_rule, max_conn)),
    CF_END
  }
};

static struct cf_section submitd_conf = {
  CF_ITEMS {
    CF_STRING("LogFile", &log_name),
    CF_UNS("Port", &port),
    CF_UNS("DHBits", &dh_bits),
    CF_UNS("MaxConn", &max_conn),
    CF_UNS("SessionTimeout", &session_timeout),
    CF_UNS("MaxRequestSize", &max_request_size),
    CF_UNS("MaxAttachSize", &max_attachment_size),
    CF_UNS("MaxVersions", &max_versions),
    CF_STRING("CACert", &ca_cert_name),
    CF_STRING("ServerCert", &server_cert_name),
    CF_STRING("ServerKey", &server_key_name),
    CF_STRING("History", &history_format),
    CF_LIST("Access", &access_rules, &access_conf),
    CF_UNS("TraceTLS", &trace_tls),
    CF_UNS("TraceCommands", &trace_commands),
    CF_END
  }
};

/*** CONNECTIONS ***/

static clist connections;
static uns last_conn_id;
static uns num_conn;

static void
conn_init(void)
{
  clist_init(&connections);
}

static struct conn *
conn_new(void)
{
  struct conn *c = xmalloc_zero(sizeof(*c));
  c->id = ++last_conn_id;
  clist_add_tail(&connections, &c->n);
  num_conn++;
  return c;
}

static void
conn_free(struct conn *c)
{
  xfree(c->ip_string);
  xfree(c->cert_name);
  clist_remove(&c->n);
  num_conn--;
  xfree(c);
}

static struct access_rule *
lookup_rule(u32 ip)
{
  CLIST_FOR_EACH(struct access_rule *, r, access_rules)
    CLIST_FOR_EACH(struct ip_node *, n, r->ip_list)
      if (ip_addrmask_match(&n->addrmask, ip))
	return r;
  return NULL;
}

static uns
conn_count(u32 ip)
{
  uns cnt = 0;
  CLIST_FOR_EACH(struct conn *, c, connections)
    if (c->ip == ip)
      cnt++;
  return cnt;
}

/*** TLS ***/

static gnutls_certificate_credentials_t cert_cred;
static gnutls_dh_params_t dh_params;

#define TLS_CHECK(name) if (err < 0) die(#name " failed: %s", gnutls_strerror(err))

static void
tls_init(void)
{
  int err;

  gnutls_global_init();
  err = gnutls_certificate_allocate_credentials(&cert_cred);
  TLS_CHECK(gnutls_certificate_allocate_credentials);
  err = gnutls_certificate_set_x509_trust_file(cert_cred, ca_cert_name, GNUTLS_X509_FMT_PEM);
  if (!err)
    die("No CA certificate found");
  if (err < 0)
    die("Unable to load X509 trust file: %s", gnutls_strerror(err));
  err = gnutls_certificate_set_x509_key_file(cert_cred, server_cert_name, server_key_name, GNUTLS_X509_FMT_PEM);
  if (err < 0)
    die("Unable to load X509 key file: %s", gnutls_strerror(err));

  err = gnutls_dh_params_init(&dh_params); TLS_CHECK(gnutls_dh_params_init);
  err = gnutls_dh_params_generate2(dh_params, dh_bits); TLS_CHECK(gnutls_dh_params_generate2);
  gnutls_certificate_set_dh_params(cert_cred, dh_params);
}

static gnutls_session_t
tls_new_session(int sk)
{
  gnutls_session_t s;
  int err;

  err = gnutls_init(&s, GNUTLS_SERVER); TLS_CHECK(gnutls_init);
  err = gnutls_set_default_priority(s); TLS_CHECK(gnutls_set_default_priority);
  gnutls_credentials_set(s, GNUTLS_CRD_CERTIFICATE, cert_cred);
  gnutls_certificate_server_set_request(s, GNUTLS_CERT_REQUEST);
  gnutls_dh_set_prime_bits(s, dh_bits);
  gnutls_transport_set_ptr(s, (gnutls_transport_ptr_t) sk);
  return s;
}

static const char *
tls_verify_cert(struct conn *c)
{
  gnutls_session_t s = c->tls;
  uns status, num_certs;
  int err;
  gnutls_x509_crt_t cert;
  const gnutls_datum_t *certs;

  DBG("Verifying peer certificates");
  err = gnutls_certificate_verify_peers2(s, &status);
  if (err < 0)
    return gnutls_strerror(err);
  DBG("Verify status: %04x", status);
  if (status & GNUTLS_CERT_INVALID)
    return "Certificate is invalid";
  /* XXX: We do not handle revokation. */
  if (gnutls_certificate_type_get(s) != GNUTLS_CRT_X509)
    return "Certificate is not X509";

  err = gnutls_x509_crt_init(&cert);
  if (err < 0)
    return "gnutls_x509_crt_init() failed";
  certs = gnutls_certificate_get_peers(s, &num_certs);
  if (!certs)
    return "No peer certificate found";
  DBG("Got certificate list with %d peers", num_certs);

  err = gnutls_x509_crt_import(cert, &certs[0], GNUTLS_X509_FMT_DER);
  if (err < 0)
    return "Cannot import certificate";
  /* XXX: We do not check expiration and activation since the keys are generated for a single contest only anyway. */

  char dn[256];
  size_t dn_len = sizeof(dn);
  err = gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_COMMON_NAME, 0, 0, dn, &dn_len);
  if (err < 0)
    return "Cannot retrieve common name";
  if (trace_tls)
    msg(L_INFO, "Cert CN: %s", dn);
  c->cert_name = xstrdup(dn);

  /* Check certificate purpose */
  char purp[256];
  int purpi = 0;
  do
    {
      size_t purp_len = sizeof(purp);
      uns crit;
      err = gnutls_x509_crt_get_key_purpose_oid(cert, purpi++, purp, &purp_len, &crit);
      if (err == GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE)
	return "Not a client certificate";
      TLS_CHECK(gnutls_x509_crt_get_key_purpose_oid);
    }
  while (strcmp(purp, GNUTLS_KP_TLS_WWW_CLIENT));

  DBG("Verified OK");
  return NULL;
}

static void
tls_log_params(struct conn *c)
{
  if (!trace_tls)
    return;
  gnutls_session_t s = c->tls;
  const char *proto = gnutls_protocol_get_name(gnutls_protocol_get_version(s));
  const char *kx = gnutls_kx_get_name(gnutls_kx_get(s));
  const char *cert = gnutls_certificate_type_get_name(gnutls_certificate_type_get(s));
  const char *comp = gnutls_compression_get_name(gnutls_compression_get(s));
  const char *cipher = gnutls_cipher_get_name(gnutls_cipher_get(s));
  const char *mac = gnutls_mac_get_name(gnutls_mac_get(s));
  msg(L_DEBUG, "TLS params: proto=%s kx=%s cert=%s comp=%s cipher=%s mac=%s",
    proto, kx, cert, comp, cipher, mac);
}

/*** FASTBUFS OVER SOCKETS AND TLS ***/

void NONRET				// Fatal protocol violation
client_error(char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  vmsg(L_ERROR_R, msg, args);
  exit(0);
}

static int
sk_fb_refill(struct fastbuf *f)
{
  struct conn *c = SKIP_BACK(struct conn, rx_fb, f);
  int cnt = read(c->sk, f->buffer, f->bufend - f->buffer);
  if (cnt < 0)
    client_error("Read error: %m");
  f->bptr = f->buffer;
  f->bstop = f->buffer + cnt;
  return cnt;
}

static void
sk_fb_spout(struct fastbuf *f)
{
  struct conn *c = SKIP_BACK(struct conn, tx_fb, f);
  int len = f->bptr - f->buffer;
  if (!len)
    return;
  int cnt = careful_write(c->sk, f->buffer, len);
  if (cnt <= 0)
    client_error("Write error");
  f->bptr = f->buffer;
}

static void
init_sk_fastbufs(struct conn *c)
{
  struct fastbuf *rf = &c->rx_fb, *tf = &c->tx_fb;

  rf->buffer = xmalloc(1024);
  rf->bufend = rf->buffer + 1024;
  rf->bptr = rf->bstop = rf->buffer;
  rf->name = "socket";
  rf->refill = sk_fb_refill;

  tf->buffer = xmalloc(1024);
  tf->bufend = tf->buffer + 1024;
  tf->bptr = tf->bstop = tf->buffer;
  tf->name = rf->name;
  tf->spout = sk_fb_spout;
}

static int
tls_fb_refill(struct fastbuf *f)
{
  struct conn *c = SKIP_BACK(struct conn, rx_fb, f);
  DBG("TLS: Refill");
  int cnt = gnutls_record_recv(c->tls, f->buffer, f->bufend - f->buffer);
  DBG("TLS: Received %d bytes", cnt);
  if (cnt < 0)
    client_error("TLS read error: %s", gnutls_strerror(cnt));
  f->bptr = f->buffer;
  f->bstop = f->buffer + cnt;
  return cnt;
}

static void
tls_fb_spout(struct fastbuf *f)
{
  struct conn *c = SKIP_BACK(struct conn, tx_fb, f);
  int len = f->bptr - f->buffer;
  if (!len)
    return;
  int cnt = gnutls_record_send(c->tls, f->buffer, len);
  DBG("TLS: Sent %d bytes", cnt);
  if (cnt <= 0)
    client_error("TLS write error: %s", gnutls_strerror(cnt));
  f->bptr = f->buffer;
}

static void
init_tls_fastbufs(struct conn *c)
{
  struct fastbuf *rf = &c->rx_fb, *tf = &c->tx_fb;

  ASSERT(rf->buffer && tf->buffer);	// Already set up for the plaintext connection
  rf->refill = tls_fb_refill;
  tf->spout = tls_fb_spout;
}

/*** CLIENT LOOP (runs in a child process) ***/

static void
sigalrm_handler(int sig UNUSED)
{
  // We do not try to do any gracious shutdown to avoid races
  client_error("Timed out");
}

static void
client_loop(struct conn *c)
{
  setproctitle("submitd: client %s", c->ip_string);
  log_pid = c->id;
  init_sk_fastbufs(c);

  signal(SIGPIPE, SIG_IGN);
  struct sigaction sa = {
    .sa_handler = sigalrm_handler
  };
  if (sigaction(SIGALRM, &sa, NULL) < 0)
    die("Cannot setup SIGALRM handler: %m");

  if (c->rule->plain_text)
    {
      bputsn(&c->tx_fb, "+OK");
      bflush(&c->tx_fb);
    }
  else
    {
      bputsn(&c->tx_fb, "+TLS");
      bflush(&c->tx_fb);
      c->tls = tls_new_session(c->sk);
      int err = gnutls_handshake(c->tls);
      if (err < 0)
	client_error("TLS handshake failed: %s", gnutls_strerror(err));
      tls_log_params(c);
      const char *cert_err = tls_verify_cert(c);
      if (cert_err)
	client_error("TLS certificate failure: %s", cert_err);
      init_tls_fastbufs(c);
    }

  alarm(session_timeout);
  if (!process_init(c))
    msg(L_ERROR, "Protocol handshake failed");
  else
    {
      setproctitle("submitd: client %s (%s)", c->ip_string, c->user);
      for (;;)
	{
	  alarm(session_timeout);
	  if (!process_command(c))
	    break;
	}
    }

  if (c->tls)
    gnutls_bye(c->tls, GNUTLS_SHUT_WR);
  close(c->sk);
  if (c->tls)
    gnutls_deinit(c->tls);
}

/*** MAIN LOOP ***/

static void
sigchld_handler(int sig UNUSED)
{
  /* We do not need to do anything, just interrupt the accept syscall */
}

static void
reap_child(pid_t pid, int status)
{
  char buf[EXIT_STATUS_MSG_SIZE];
  if (format_exit_status(buf, status))
    msg(L_ERROR, "Child %d %s", (int)pid, buf);

  CLIST_FOR_EACH(struct conn *, c, connections)
    if (c->pid == pid)
      {
	msg(L_INFO, "Connection %d closed", c->id);
	conn_free(c);
	return;
      }
  msg(L_ERROR, "Cannot find connection for child process %d", (int)pid);
}

static int listen_sk;

static void
sk_init(void)
{
  listen_sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listen_sk < 0)
    die("socket: %m");
  int one = 1;
  if (setsockopt(listen_sk, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    die("setsockopt(SO_REUSEADDR): %m");

  struct sockaddr_in sa;
  bzero(&sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons(port);
  if (bind(listen_sk, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    die("Cannot bind to port %d: %m", port);
  if (listen(listen_sk, 1024) < 0)
    die("Cannot listen on port %d: %m", port);
}

static void
sk_accept(void)
{
  struct sockaddr_in sa;
  int salen = sizeof(sa);
  int sk = accept(listen_sk, (struct sockaddr *) &sa, &salen);
  if (sk < 0)
    {
      if (errno == EINTR)
	return;
      die("accept: %m");
    }

  char ipbuf[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &sa.sin_addr, ipbuf, sizeof(ipbuf));
  u32 addr = ntohl(sa.sin_addr.s_addr);
  uns port = ntohs(sa.sin_port);
  char *err;

  struct access_rule *rule = lookup_rule(addr);
  if (!rule)
    {
      err = "Unauthorized";
      goto reject;
    }

  if (num_conn >= max_conn)
    {
      err = "Too many connections";
      goto reject;
    }

  if (conn_count(addr) >= rule->max_conn)
    {
      err = "Too many connections from this address";
      goto reject;
    }

  struct conn *c = conn_new();
  msg(L_INFO, "Connection from %s:%d (id %d, %s, %s)",
	ipbuf, port, c->id,
	(rule->plain_text ? "plain-text" : "TLS"),
	(rule->allow_admin ? "admin" : "user"));
  c->ip = addr;
  c->ip_string = xstrdup(ipbuf);
  c->sk = sk;
  c->rule = rule;

  c->pid = fork();
  if (c->pid < 0)
    {
      conn_free(c);
      err = "Server overloaded";
      msg(L_ERROR, "Fork failed: %m");
      goto reject2;
    }
  if (!c->pid)
    {
      close(listen_sk);
      client_loop(c);
      exit(0);
    }
  close(sk);
  return;

reject:
  msg(L_ERROR_R, "Connection from %s:%d rejected (%s)", ipbuf, port, err);
reject2: ;
  // Write an error message to the socket, but do not allow it to slow us down
  struct linger ling = { .l_onoff=0 };
  if (setsockopt(sk, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0)
    msg(L_ERROR, "Cannot set SO_LINGER: %m");
  write(sk, "-", 1);
  write(sk, err, strlen(err));
  write(sk, "\n", 1);
  close(sk);
}

int main(int argc, char **argv)
{
  setproctitle_init(argc, argv);
  cf_def_file = "cf/submitd";
  cf_declare_section("SubmitD", &submitd_conf, 0);
  cf_declare_section("Tasks", &tasks_conf, 0);

  int opt;
  if ((opt = cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL)) >= 0)
    die("This program has no options");

  log_file(log_name);

  msg(L_INFO, "Initializing TLS");
  tls_init();

  conn_init();
  sk_init();
  msg(L_INFO, "Listening on port %d", port);

  struct sigaction sa = {
    .sa_handler = sigchld_handler
  };
  if (sigaction(SIGCHLD, &sa, NULL) < 0)
    die("Cannot setup SIGCHLD handler: %m");

  for (;;)
    {
      setproctitle("submitd: %d connections", num_conn);
      int status;
      pid_t pid = waitpid(-1, &status, WNOHANG);
      if (pid > 0)
	reap_child(pid, status);
      else
	sk_accept();
    }
}
