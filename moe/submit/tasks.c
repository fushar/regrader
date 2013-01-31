/*
 *  The Submit Daemon: Tasks
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/fastbuf.h"
#include "ucw/stkstring.h"
#include "ucw/simple-lists.h"
#include "ucw/mempool.h"
#include "sherlock/object.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "submitd.h"

clist task_list;
static clist extensions;
static clist open_data_extensions;

static char *
tasks_conf_commit(void *p UNUSED)
{
  // We do not do any journaling here as we do not switch config files on the fly
  CLIST_FOR_EACH(struct task *, t, task_list)
    {
      clist_init(&t->parts);
      if (t->open_data)
	{
	  for (uns i=1; i<=t->open_data; i++)
	    simp_append(cf_pool, &t->parts)->s = mp_printf(cf_pool, "%02d", i);
	  t->extensions = &open_data_extensions;
	}
      else
	{
	  simp_append(cf_pool, &t->parts)->s = t->name;
	  t->extensions = &extensions;
	}
    }
  return NULL;
}

static struct cf_section task_conf = {
  CF_TYPE(struct task),
  CF_ITEMS {
    CF_STRING("Name", PTR_TO(struct task, name)),
    CF_UNS("OpenData", PTR_TO(struct task, open_data)),
    CF_UNS("MaxSize", PTR_TO(struct task, max_size)),
    CF_END
  }
};

struct cf_section tasks_conf = {
  CF_COMMIT(tasks_conf_commit),
  CF_ITEMS {
    CF_LIST("Task", &task_list, &task_conf),
    CF_LIST("Extension", &extensions, &cf_string_list_config),
    CF_LIST("OpenDataExt", &open_data_extensions, &cf_string_list_config),
    CF_END
  }
};

struct task *
task_find(char *name)
{
  CLIST_FOR_EACH(struct task *, t, task_list)
    if (!strcasecmp(t->name, name))
      return t;
  return NULL;
}

int
part_exists_p(struct task *t, char *name)
{
  CLIST_FOR_EACH(simp_node *, p, t->parts)
    if (!strcmp(p->s, name))
      return 1;
  return 0;
}

int
ext_exists_p(struct task *t, char *ext)
{
  CLIST_FOR_EACH(simp_node *, x, *t->extensions)
    if (!strcmp(x->s, ext))
      return 1;
  return 0;
}

int
user_exists_p(char *user)
{
  char *fn = stk_printf("solutions/%s", user);
  struct stat st;
  return !stat(fn, &st) && S_ISDIR(st.st_mode);
}

void
task_load_status(struct conn *c)
{
  struct fastbuf *fb = bopen_try(stk_printf("solutions/%s/status", c->user), O_RDONLY, 4096);
  c->task_status = obj_new(c->pool);
  if (fb)
    {
      obj_read(fb, c->task_status);
      bclose(fb);
    }
}

void
task_lock_status(struct conn *c)
{
  ASSERT(!c->task_lock_fd);
  if ((c->task_lock_fd = open(stk_printf("solutions/%s/status.lock", c->user), O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
    die("Cannot create task lock: %m");
  struct flock fl = {
    .l_type = F_WRLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 1
  };
  if (fcntl(c->task_lock_fd, F_SETLKW, &fl) < 0)
    die("Cannot lock status file: %m");
  task_load_status(c);
}

void
task_unlock_status(struct conn *c, uns write_back)
{
  ASSERT(c->task_lock_fd);

  if (write_back)
    {
      struct fastbuf *fb = bopen_tmp(4096);
      obj_write(fb, c->task_status, BUCKET_TYPE_PLAIN);
      brewind(fb);
      bconfig(fb, BCONFIG_IS_TEMP_FILE, 0);
      char *name = stk_printf("solutions/%s/status", c->user);
      if (rename(fb->name, name) < 0)
	die("Unable to rename %s to %s: %m", fb->name, name);
      bclose(fb);
    }

  struct flock fl = {
    .l_type = F_UNLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 1
  };
  if (fcntl(c->task_lock_fd, F_SETLKW, &fl) < 0)
    die("Cannot unlock status file: %m");
  c->task_lock_fd = 0;
}

struct odes *
task_status_find_task(struct conn *c, struct task *t, uns create)
{
  for (struct oattr *a = obj_find_attr(c->task_status, 'T' + OBJ_ATTR_SON); a; a=a->same)
    {
      struct odes *o = a->son;
      char *name = obj_find_aval(o, 'T');
      ASSERT(name);
      if (!strcmp(name, t->name))
	return o;
    }
  if (!create)
    return NULL;
  struct odes *o = obj_add_son(c->task_status, 'T' + OBJ_ATTR_SON);
  obj_set_attr(o, 'T', t->name);
  return o;
}

struct odes *
task_status_find_part(struct odes *to, char *part, uns create)
{
  for (struct oattr *a = obj_find_attr(to, 'P' + OBJ_ATTR_SON); a; a=a->same)
    {
      struct odes *o = a->son;
      char *name = obj_find_aval(o, 'P');
      ASSERT(name);
      if (!strcmp(name, part))
	return o;
    }
  if (!create)
    return NULL;
  struct odes *o = obj_add_son(to, 'P' + OBJ_ATTR_SON);
  obj_set_attr(o, 'P', part);
  return o;
}

static void
task_record_history(char *user, char *task, char *part, char *ext, uns version, char *submitted_name)
{
  if (!history_format)
    return;

  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char prefix[256];
  if (strftime(prefix, sizeof(prefix), history_format, tm) <= 0)
    {
      msg(L_ERROR, "Error formatting history prefix: too long");
      return;
    }

  char *name = stk_printf("%s%s:%s:%s:v%d.%s", prefix, user, task, (strcmp(task, part) ? part : (char*)""), version, ext);
  struct fastbuf *orig = bopen(submitted_name, O_RDONLY, 4096);
  struct fastbuf *hist = bopen(name, O_WRONLY | O_CREAT | O_EXCL, 4096);
  bbcopy_slow(orig, hist, ~0U);
  bclose(hist);
  bclose(orig);
}

void
task_submit_part(char *user, char *task, char *part, char *ext, uns version, struct fastbuf *fb)
{
  char *dir = stk_printf("solutions/%s/%s", user, task);
  char *name = stk_printf("%s/%s.%s", dir, part, ext);

  struct stat st;
  if (stat(dir, &st) < 0 && errno == ENOENT && mkdir(dir, 0777) < 0)
    die("Cannot create %s: %m", dir);

  bconfig(fb, BCONFIG_IS_TEMP_FILE, 0);
  if (rename(fb->name, name) < 0)
    die("Cannot rename %s to %s: %m", fb->name, name);

  task_record_history(user, task, part, ext, version, name);
}

void
task_delete_part(char *user, char *task, char *part, char *ext, uns version UNUSED)
{
  char *dir = stk_printf("solutions/%s/%s", user, task);
  char *name = stk_printf("%s/%s.%s", dir, part, ext);
  if (unlink(name) < 0)
    msg(L_ERROR, "Cannot delete %s: %m", name);
}
