/*
 *  The Submit Daemon
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#ifndef _SUBMITD_H
#define _SUBMITD_H

#include "ucw/clists.h"
#include "ucw/ipaccess.h"
#include "ucw/fastbuf.h"

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

struct ip_node {
  cnode n;
  struct ip_addrmask addrmask;
};

struct access_rule {
  cnode n;
  clist ip_list;
  uns allow_admin;
  uns plain_text;
  uns max_conn;
};

struct conn {
  // Set up by the master process
  cnode n;
  u32 ip;
  char *ip_string;			// (xmalloced)
  pid_t pid;
  uns id;
  struct access_rule *rule;		// Rule matched by this connection
  int sk;				// Client socket
  char *cert_name;			// Client name from the certificate (NULL if no TLS) (xmalloced)

  // Used by the child process
  gnutls_session_t tls;			// TLS session
  struct fastbuf rx_fb, tx_fb;		// Fastbufs for communication with the client (either plain-text or TLS)
  struct mempool *pool;
  struct odes *request;
  struct odes *reply;
  struct odes *task_status;
  int task_lock_fd;
  char *user;
};

extern uns max_request_size, max_attachment_size, trace_commands;
extern uns max_versions;
extern char *history_format;

/* submitd.c */

void NONRET client_error(char *msg, ...);

/* commands.c */

int process_init(struct conn *c);
int process_command(struct conn *c);

/* tasks.c */

struct task {
  cnode n;
  char *name;
  uns open_data;	// Number of parts for open-data tasks
  uns max_size;		// Maximum size (0=use global default)
  clist parts;		// List of parts of this task (simp_nodes)
  clist *extensions;	// List of allowed extensions for this task (simp_nodes)
};

extern clist task_list;
extern struct cf_section tasks_conf;

struct task *task_find(char *name);
int part_exists_p(struct task *t, char *name);
int user_exists_p(char *user);
int ext_exists_p(struct task *t, char *ext);

void task_lock_status(struct conn *c);
void task_unlock_status(struct conn *c, uns write_back);
void task_load_status(struct conn *c);

struct odes *task_status_find_task(struct conn *c, struct task *t, uns create);
struct odes *task_status_find_part(struct odes *t, char *part, uns create);

void task_submit_part(char *user, char *task, char *part, char *ext, uns version, struct fastbuf *fb);
void task_delete_part(char *user, char *task, char *part, char *ext, uns version);

#endif
