/*
 *  The Submit Daemon: High-Level Part of the Protocol
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/mempool.h"
#include "ucw/simple-lists.h"
#include "ucw/stkstring.h"
#include "ucw/fastbuf.h"
#include "sherlock/object.h"
#include "sherlock/objread.h"

#include <fcntl.h>
#include <time.h>

#include "submitd.h"

/*** REQUESTS AND REPLIES ***/

static void NONRET
read_error_cb(struct obj_read_state *st UNUSED, char *msg)
{
  client_error("Request parse error: %s", msg);
}

static int
read_request(struct conn *c)
{
  if (c->pool)
    mp_flush(c->pool);
  else
    c->pool = mp_new(1024);
  c->request = obj_new(c->pool);
  c->reply = obj_new(c->pool);

  struct obj_read_state st;
  obj_read_start(&st, c->request);
  st.error_callback = read_error_cb;
  char line[1024];
  uns size = 0;
  for (;;)
    {
      int l = bgets_nodie(&c->rx_fb, line, sizeof(line));
      if (l < 0)
	client_error("Request line too long");
      if (!l)
	{
	  if (!size)
	    return 0;
	  else
	    client_error("Truncated request");
	}
      if (l == 1)
	break;
      size += l;
      if (size >= max_request_size)
	client_error("Request too long");
      obj_read_attr(&st, line[0], line+1);
    }
  obj_read_end(&st);
  return 1;
}

static void
write_reply(struct conn *c)
{
  if (!obj_find_attr(c->reply, '-') && !obj_find_attr(c->reply, '+'))
    obj_set_attr(c->reply, '+', "OK");
  if (trace_commands)
    {
      char *m;
      if (m = obj_find_aval(c->reply, '-'))
	msg(L_DEBUG, ">> -%s", m);
      else if (m = obj_find_aval(c->reply, '+'))
	msg(L_DEBUG, ">> +%s", m);
      else
	msg(L_DEBUG, ">> ???");
    }
  obj_write(&c->tx_fb, c->reply, BUCKET_TYPE_PLAIN);
  bputc(&c->tx_fb, '\n');
  bflush(&c->tx_fb);
}

static void
err(struct conn *c, char *msg)
{
  obj_set_attr(c->reply, '-', msg);
}

/*** STATUS ***/

static void
copy_attrs(struct odes *dest, struct odes *src)
{
  for (struct oattr *a = src->attrs ; a; a=a->next)
    if (a->attr < OBJ_ATTR_SON)
      for (struct oattr *aa = a; aa; aa=aa->same)
	obj_add_attr(dest, aa->attr, aa->val);
}

static void
cmd_status(struct conn *c)
{
  uns verbose = obj_find_anum(c->request, 'V', 0);
  task_load_status(c);

  CLIST_FOR_EACH(struct task *, t, task_list)
    {
      struct odes *to = task_status_find_task(c, t, 1);
      struct odes *tr = obj_add_son(c->reply, 'T' + OBJ_ATTR_SON);
      copy_attrs(tr, to);
      CLIST_FOR_EACH(simp_node *, x, *t->extensions)
	obj_add_attr(tr, 'A', x->s);
      CLIST_FOR_EACH(simp_node *, p, t->parts)
	{
	  struct odes *po = task_status_find_part(to, p->s, 1);
	  struct odes *pr = obj_add_son(tr, 'P' + OBJ_ATTR_SON);
	  copy_attrs(pr, po);
	  uns current_ver = obj_find_anum(po, 'V', 0);
	  for (struct oattr *v = obj_find_attr(po, 'V' + OBJ_ATTR_SON); v; v=v->same)
	    {
	      struct odes *vo = v->son;
	      uns ver = obj_find_anum(vo, 'V', 0);
	      if (ver == current_ver || verbose)
		obj_add_son_ref(pr, 'V' + OBJ_ATTR_SON, vo);
	    }
	}
    }
}

/*** Contest timeout checks ***/

static int
load_time_limit(char *name)
{
  struct fastbuf *f = bopen_try(name, O_RDONLY, 1024);
  if (!f)
    return -1;
  char buf[256];
  int h, m;
  if (bgets_nodie(f, buf, sizeof(buf)) < 0 ||
      sscanf(buf, "%d:%d", &h, &m) != 2 ||
      h < 0 || h > 23 || m < 0 || m > 59)
    {
      msg(L_ERROR, "Invalid timeout in %s", name);
      bclose(f);
      return -1;
    }
  bclose(f);
  return 100*h + m;
}

static int
contest_over(struct conn *c)
{
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  int tstamp = tm->tm_hour*100 + tm->tm_min;
  int local_limit = load_time_limit(stk_printf("solutions/%s/TIMEOUT", c->user));
  int global_limit = load_time_limit("solutions/TIMEOUT");
  if (trace_commands > 1)
    msg(L_DEBUG, "Time check: current %d, global limit %d, user limit %d", tstamp, global_limit, local_limit);
  if (local_limit >= 0)
    return (tstamp >= local_limit);
  return (global_limit >= 0 && tstamp >= global_limit);
}

/*** SUBMIT ***/

static struct fastbuf *
read_attachment(struct conn *c, uns max_size)
{
  uns size = obj_find_anum(c->request, 'S', 0);
  if (size > max_size)
    {
      err(c, "Submission too large");
      return NULL;
    }
  obj_set_attr(c->reply, '+', "Go on");
  write_reply(c);
  obj_set_attr(c->reply, '+', NULL);

  // This is less efficient than bbcopy(), but we want our own error handling.
  struct fastbuf *fb = bopen_tmp(4096);
  char buf[4096];
  uns remains = size;
  while (remains)
    {
      uns cnt = bread(&c->rx_fb, buf, MIN(remains, (uns)sizeof(buf)));
      if (!cnt)
	{
	  bclose(fb);
	  client_error("Truncated attachment");
	}
      bwrite(fb, buf, cnt);
      remains -= cnt;
    }
  brewind(fb);
  return fb;
}

static void
cmd_submit(struct conn *c)
{
  if (contest_over(c))
    {
      err(c, "The contest is over, no more submits allowed");
      return;
    }

  char *tname = obj_find_aval(c->request, 'T');
  if (!tname)
    {
      err(c, "No task specified");
      return;
    }
  struct task *task = task_find(tname);
  if (!task)
    {
      err(c, "No such task");
      return;
    }

  char *pname = obj_find_aval(c->request, 'P');
  if (!pname)
    {
      simp_node *s = clist_head(&task->parts);
      ASSERT(s);
      pname = s->s;
    }
  else if (!part_exists_p(task, pname))
    {
      err(c, "No such task part");
      return;
    }

  char *ext = obj_find_aval(c->request, 'X');
  if (!ext || !ext_exists_p(task, ext))
    {
      err(c, "Missing or invalid extension");
      return;
    }

  uns max_size = task->max_size ? : max_attachment_size;
  struct fastbuf *fb = read_attachment(c, max_size);
  if (!fb)
    return;

  task_lock_status(c);
  struct odes *tasko = task_status_find_task(c, task, 1);
  struct odes *parto = task_status_find_part(tasko, pname, 1);
  uns current_ver = obj_find_anum(parto, 'V', 0);
  if (current_ver >= max_versions)
    {
      err(c, "Maximum number of submits of this task exceeded");
      bclose(fb);
      task_unlock_status(c, 0);
      return;
    }
  uns last_ver = 0;
  uns replaced_ver = 0;
  for (struct oattr *a = obj_find_attr(parto, 'V' + OBJ_ATTR_SON); a; a=a->same)
    {
      uns ver = obj_find_anum(a->son, 'V', 0);
      char *ext = obj_find_aval(a->son, 'X');
      ASSERT(ver && ext);
      last_ver = MAX(last_ver, ver);
      if (ver == current_ver)
        {
	  task_delete_part(c->user, tname, pname, ext, ver);
	  obj_set_attr(a->son, 'S', "replaced");
	  replaced_ver = current_ver;
	}
    }
  struct odes *vero = obj_add_son(parto, 'V' + OBJ_ATTR_SON);
  obj_set_attr_num(vero, 'V', ++last_ver);
  obj_set_attr_num(vero, 'T', time(NULL));
  obj_set_attr_num(vero, 'L', obj_find_anum(c->request, 'S', 0));
  obj_set_attr(vero, 'S', "submitted");
  obj_set_attr(vero, 'X', ext);
  task_submit_part(c->user, tname, pname, ext, last_ver, fb);
  obj_set_attr_num(parto, 'V', last_ver);
  task_unlock_status(c, 1);

  msg(L_INFO, "User %s submitted task %s%s (version %d%s)",
	c->user, tname,
	(strcmp(tname, pname) ? stk_printf("/%s", pname) : ""),
	last_ver,
	(replaced_ver ? stk_printf(", replaced %d", replaced_ver) : ""));
}

/*** COMMAND MUX ***/

static void
execute_command(struct conn *c)
{
  char *cmd = obj_find_aval(c->request, '!');
  if (!cmd)
    {
      err(c, "Missing command");
      return;
    }
  if (trace_commands)
    msg(L_DEBUG, "<< %s", cmd);
  if (!strcasecmp(cmd, "SUBMIT"))
    cmd_submit(c);
  else if (!strcasecmp(cmd, "STATUS"))
    cmd_status(c);
  else if (!strcasecmp(cmd, "NOP"))
    ;
  else
    err(c, "Unknown command");
}

int
process_command(struct conn *c)
{
  if (!read_request(c))
    return 0;
  execute_command(c);
  write_reply(c);
  return 1;
}

/*** INITIAL HANDSHAKE ***/

static void
execute_init(struct conn *c)
{
  char *user = obj_find_aval(c->request, 'U');
  if (!user)
    {
      err(c, "Missing user");
      return;
    }
  if (!c->cert_name ||
      !strcmp(user, c->cert_name) ||
      c->rule->allow_admin && !strcmp(c->cert_name, "admin"))
    {
      if (!user_exists_p(user))
	{
	  err(c, "Unknown user");
	  return;
	}
      msg(L_INFO, "Logged in %s", user);
    }
  else
    {
      err(c, "Permission denied");
      msg(L_ERROR, "Unauthorized attempt to log in as %s", user);
      return;
    }
  c->user = xstrdup(user);
}

int
process_init(struct conn *c)
{
  if (!read_request(c))
    return 0;
  execute_init(c);
  write_reply(c);
  return !obj_find_attr(c->reply, '-');
}
