/*
 *	Insane tester of reading configuration files
 *
 *	(c) 2006 Robert Spalek <robert@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/conf.h"
#include "ucw/getopt.h"
#include "ucw/clists.h"
#include "ucw/fastbuf.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static int verbose;

struct sub_sect_1 {
  cnode n;
  char *name;
  time_t t;
  char *level;
  int confidence[2];
  double *list;
};

static struct sub_sect_1 sec1 = { {}, "Charlie", 0, "WBAFC", { 0, -1}, DARY_ALLOC(double, 3, 1e4, -1e-4, 8) };

static char *
init_sec_1(struct sub_sect_1 *s)
{
  if (s == &sec1)			// this is a static variable; skip clearing
    return NULL;
  s->name = "unknown";
  s->level = "default";
  s->confidence[0] = 5;
  s->confidence[1] = 6;
  // leave s->list==NULL
  return NULL;
}

static char *
commit_sec_1(struct sub_sect_1 *s)
{
  if (s->confidence[0] < 0 || s->confidence[0] > 10)
    return "Well, this can't be";
  return NULL;
}

static char *
time_parser(uns number, char **pars, time_t *ptr)
{
  *ptr = number ? atoi(pars[0]) : time(NULL);
  return NULL;
}

static struct cf_section cf_sec_1 = {
  CF_TYPE(struct sub_sect_1),
  CF_INIT(init_sec_1),
  CF_COMMIT(commit_sec_1),
#define F(x)	PTR_TO(struct sub_sect_1, x)
  CF_ITEMS {
    CF_STRING("name", F(name)),
    //CF_PARSER("t", F(t), time_parser, 0),
    CF_STRING("level", F(level)),
    CF_INT_ARY("confidence", F(confidence[0]), 2),		// XXX: the [0] is needed for the sake of type checking
    CF_DOUBLE_DYN("list", F(list), 100),
    CF_END
  }
#undef F
};

static uns nr1 = 15;
static int *nrs1 = DARY_ALLOC(int, 5, 5, 4, 3, 2, 1);
static int nrs2[5];
static char *str1 = "no worries";
static char **str2 = DARY_ALLOC(char *, 2, "Alice", "Bob");
static u64 u1 = 0xCafeBeefDeadC00ll;
static double d1 = -1.1;
static clist secs;
static time_t t1, t2;
static u32 ip;
static int *look = DARY_ALLOC(int, 2, 2, 1);
static u16 numbers[10] = { 2, 100, 1, 5 };
static u32 bitmap1 = 0xff;
static u32 bitmap2 = 3;

static char *
parse_u16(char *string, u16 *ptr)
{
  uns a;
  char *msg = cf_parse_int(string, &a);
  if (msg)
    return msg;
  if (a >= (1<<16))
    return "Come on, man, this doesn't fit to 16 bits";
  *ptr = a;
  return NULL;
}

static void
dump_u16(struct fastbuf *fb, u16 *ptr)
{
  bprintf(fb, "%d ", *ptr);
}

static struct cf_user_type u16_type = {
  .size = sizeof(u16),
  .name = "u16",
  .parser = (cf_parser1*) parse_u16,
  .dumper = (cf_dumper1*) dump_u16
};

static char *
init_top(void *ptr UNUSED)
{
  for (uns i=0; i<5; i++)
  {
    struct sub_sect_1 *s = xmalloc(sizeof(struct sub_sect_1));	// XXX: cannot by cf_malloc(), because it's deleted when cf_reload()'ed
    cf_init_section("slaves", &cf_sec_1, s, 1);
    s->confidence[1] = i;
    clist_add_tail(&secs, &s->n);
  }
  return NULL;
}

static char *
commit_top(void *ptr UNUSED)
{
  if (nr1 != 15)
    return "Don't touch my variable!";
  return NULL;
}

static const char * const alphabet[] = { "alpha", "beta", "gamma", "delta", NULL };
static struct cf_section cf_top = {
  CF_INIT(init_top),
  CF_COMMIT(commit_top),
  CF_ITEMS {
    CF_UNS("nr1", &nr1),
    CF_INT_DYN("nrs1", &nrs1, 1000),
    CF_INT_ARY("nrs2", nrs2, 5),
    CF_STRING("str1", &str1),
    CF_STRING_DYN("str2", &str2, 20),
    CF_U64("u1", &u1),
    CF_DOUBLE("d1", &d1),
    CF_PARSER("FirstTime", &t1, time_parser, -1),
    CF_PARSER("SecondTime", &t2, time_parser, 1),
    CF_SECTION("master", &sec1, &cf_sec_1),
    CF_LIST("slaves", &secs, &cf_sec_1),
    CF_IP("ip", &ip),
    CF_LOOKUP_DYN("look", &look, alphabet, 1000),
    CF_USER_ARY("numbers", numbers, &u16_type, 10),
    CF_BITMAP_INT("bitmap1", &bitmap1),
    CF_BITMAP_LOOKUP("bitmap2", &bitmap2, ((const char* const[]) {
	  "one", "two", "three", "four", "five", "six", "seven", "eight", 
	  "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "seventeen", 
	  "eighteen", "nineteen", "twenty", NULL	// hidden joke here
	  })),
    CF_END
  }
};

static char short_opts[] = CF_SHORT_OPTS "v";
static struct option long_opts[] = {
	CF_LONG_OPTS
	{"verbose",	0, 0, 'v'},
	{NULL,		0, 0, 0}
};

static char *help = "\
Usage: conf-test <options>\n\
\n\
Options:\n"
CF_USAGE
"-v\t\t\tBe verbose\n\
";

static void NONRET
usage(char *msg, ...)
{
  va_list va;
  va_start(va, msg);
  if (msg)
    vfprintf(stderr, msg, va);
  fputs(help, stderr);
  exit(1);
}

int
main(int argc, char *argv[])
{
  log_init(argv[0]);
  cf_declare_section("top", &cf_top, 0);
  cf_def_file = "ucw/conf-test.cf";

  int opt;
  while ((opt = cf_getopt(argc, argv, short_opts, long_opts, NULL)) >= 0)
    switch (opt) {
      case 'v': verbose++; break;
      default: usage("unknown option %c\n", opt);
    }
  if (optind < argc)
    usage("too many parameters (%d more)\n", argc-optind);

  /*
  cf_load("non-existent file");
  //cf_reload("non-existent file");
  cf_load("non-existent file");
  cf_set("top.d1 -1.1; top.master b");
  cf_reload(NULL);
  cf_reload(NULL);
  */

  struct fastbuf *out = bfdopen(1, 1<<14);
  cf_dump_sections(out);
  bclose(out);

  return 0;
}
