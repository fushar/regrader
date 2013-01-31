/*
 *  A Simple Testing Client
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

static int port = 8888;

static gnutls_certificate_credentials_t cert_cred;

#define TLS_CHECK(name) if (err < 0) die(#name " failed: %s", gnutls_strerror(err))

static void
tls_init(void)
{
  int err;

  msg(L_INFO, "Initializing TLS");
  gnutls_global_init();
  err = gnutls_certificate_allocate_credentials(&cert_cred);
  TLS_CHECK(gnutls_certificate_allocate_credentials);
  err = gnutls_certificate_set_x509_trust_file(cert_cred, "ca-cert.pem", GNUTLS_X509_FMT_PEM);
  if (!err)
    die("No CA certificate found");
  if (err < 0)
    die("Unable to load X509 trust file: %s", gnutls_strerror(err));
  err = gnutls_certificate_set_x509_key_file(cert_cred, "client-cert.pem", "client-key.pem", GNUTLS_X509_FMT_PEM);
  if (err < 0)
    die("Unable to load X509 key file: %s", gnutls_strerror(err));
}

static const char *
tls_verify_cert(gnutls_session_t s)
{
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
  /* XXX: Neither we check host name */

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
  while (strcmp(purp, GNUTLS_KP_TLS_WWW_SERVER));

  DBG("Verified OK");
  return NULL;
}

static void
tls_log_params(gnutls_session_t s)
{
  const char *proto = gnutls_protocol_get_name(gnutls_protocol_get_version(s));
  const char *kx = gnutls_kx_get_name(gnutls_kx_get(s));
  const char *cert = gnutls_certificate_type_get_name(gnutls_certificate_type_get(s));
  const char *comp = gnutls_compression_get_name(gnutls_compression_get(s));
  const char *cipher = gnutls_cipher_get_name(gnutls_cipher_get(s));
  const char *mac = gnutls_mac_get_name(gnutls_mac_get(s));
  msg(L_DEBUG, "TLS params: proto=%s kx=%s cert=%s comp=%s cipher=%s mac=%s",
    proto, kx, cert, comp, cipher, mac);
}

int main(int argc UNUSED, char **argv UNUSED)
{
  tls_init();

  int sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sk < 0)
    die("socket: %m");

  msg(L_INFO, "Connecting to port %d", port);
  struct sockaddr_in sa;
  bzero(&sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(0x7f000001);
  sa.sin_port = htons(port);
  if (connect(sk, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    die("Cannot connect: %m");

  msg(L_INFO, "Waiting for initial message");
  char mesg[256];
  int i = 0;
  do
    {
      if (i >= (int)sizeof(mesg))
	die("Response too long");
      int c = read(sk, mesg+i, sizeof(mesg)-i);
      if (c <= 0)
	die("Connection broken");
      i += c;
    }
  while (mesg[i-1] != '\n');
  mesg[i-1] = 0;
  if (mesg[0] != '+')
    die("%s", mesg);
  msg(L_INFO, "%s", mesg);

  gnutls_session_t s;
  gnutls_init(&s, GNUTLS_CLIENT);
  gnutls_set_default_priority(s);
  gnutls_credentials_set(s, GNUTLS_CRD_CERTIFICATE, cert_cred);
  gnutls_transport_set_ptr(s, (gnutls_transport_ptr_t) sk);

  msg(L_INFO, "Handshaking");
  int err = gnutls_handshake(s); TLS_CHECK(gnutls_handshake);
  tls_log_params(s);
  const char *cert_err = tls_verify_cert(s);
  if (cert_err)
    die("Certificate verification failed: %s", cert_err);

  msg(L_INFO, "Session established");
  for (;;)
    {
      char buf[1024];
      do
	{
	  if (!fgets(buf, sizeof(buf), stdin))
	    goto done;
	  int len = strlen(buf);
	  err = gnutls_record_send(s, buf, len); TLS_CHECK(gnutls_record_send);
	}
      while (buf[0] != '\n');
      int last = 0;
      for (;;)
	{
	  err = gnutls_record_recv(s, buf, sizeof(buf)); TLS_CHECK(gnutls_record_recv);
	  if (!err)
	    {
	      msg(L_INFO, "Connection closed");
	      break;
	    }
	  fwrite(buf, 1, err, stdout);
	  for (int i=0; i<err; i++)
	    {
	      if (buf[i] == '\n' && last == '\n')
		goto next;
	      last = buf[i];
	    }
	}
next:
      fflush(stdout);
    }

done:
  gnutls_bye(s, GNUTLS_SHUT_RDWR);
  close(sk);
  gnutls_deinit(s);

  return 0;
}
