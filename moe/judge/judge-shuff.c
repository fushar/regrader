/*
 *	A judge comparing shuffled sequences of tokens
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "judge.h"

static int ignore_nl, ignore_empty, ignore_case;
static int shuffle_lines, shuffle_words;

typedef unsigned int uns;

/*** Token buffer ***/

struct tokpage {
  struct tokpage *next;
  char *pos, *end;
  char buf[];
};

struct tokbuf {
  // For writing:
  struct tokpage *first_page, *last_page;
  uns num_tokens, num_lines;
  // For reading:
  struct tokpage *this_page;
  char *read_pos;
};

#define TOKBUF_PAGE 65536

static void init_tokbuf(struct tokbuf *tb)
{
  memset(tb, 0, sizeof(*tb));
}

static void add_token(struct tokbuf *tb, char *token, int l)
{
  l++;
  struct tokpage *pg = tb->last_page;
  if (!pg || pg->end - pg->pos < l)
    {
      if (pg)
	pg->end = pg->pos;
      int size = TOKBUF_PAGE - sizeof(struct tokbuf);
      if (l > size/5)
	size = l;
      pg = xmalloc(sizeof(struct tokbuf) + size);
      if (tb->last_page)
	tb->last_page->next = pg;
      else
	tb->first_page = pg;
      tb->last_page = pg;
      pg->next = NULL;
      pg->pos = pg->buf;
      pg->end = pg->buf + size;
    }
  memcpy(pg->pos, token, l);
  pg->pos += l;
  tb->num_tokens++;
  if (l == 1)
    tb->num_lines++;
}

static void close_tokens(struct tokbuf *tb)
{
  if (tb->last_page)
    tb->last_page->end = tb->last_page->pos;
}

static char *get_next(struct tokbuf *tb)
{
  struct tokpage *pg = tb->this_page;
  tb->read_pos += strlen(tb->read_pos) + 1;
  if (tb->read_pos >= pg->end)
    {
      tb->this_page = pg = pg->next;
      if (!pg)
	return NULL;
      tb->read_pos = pg->buf;
    }
  return tb->read_pos;
}

static char *get_first(struct tokbuf *tb)
{
  struct tokpage *pg = tb->first_page;
  if (!pg)
    return NULL;
  tb->this_page = pg;
  tb->read_pos = pg->buf;
  return pg->buf;
}

/*** Reading and shuffling ***/

struct tok {
  char *token;
  uns hash;
};

struct line {
  struct tok *toks;
  uns len;
  uns hash;
  uns orig_line;
};

struct shouffle {
  struct tokbuf tb;
  struct tok *tok_array;
  struct line *line_array;
  uns num_lines;
};

static void read_input(struct tokenizer *t, struct tokbuf *tb)
{
  char *tok;
  int nl = 1;

  init_tokbuf(tb);
  while (tok = get_token(t))
    {
      if (tok[0])
	{
	  nl = 0;
	  if (ignore_case)
	    for (char *c=tok; *c; c++)
	      if (*c >= 'a' && *c <= 'z')
		*c = *c - 'a' + 'A';
	}
      else
	{
	  if (nl && ignore_nl)
	    continue;
	  nl = 1;
	}
      add_token(tb, tok, t->toksize);
    }
  if (!nl)
    add_token(tb, "", 0);
  close_tokens(tb);
}

static inline uns string_hash(unsigned char *x)
{
  uns h = 1;
  while (*x)
    h = (h * 0x6011) + *x++;
  return h;
}

static inline uns line_hash(struct tok *t, uns cnt)
{
  uns h = 1;
  while (cnt--)
    h = (h * 0x6011) + (t++)->hash;
  return h;
}

static int compare_toks(struct tok *x, struct tok *y)
{
  if (x->hash < y->hash)
    return -1;
  if (x->hash > y->hash)
    return 1;
  return strcmp(x->token, y->token);
}

static int compare_lines(struct line *x, struct line *y)
{
  if (x->hash < y->hash)
    return -1;
  if (x->hash > y->hash)
    return 1;

  if (x->len < y->len)
    return -1;
  if (x->len > y->len)
    return 1;

  for (uns i=0; i < x->len; i++)
    {
      int c = compare_toks(&x->toks[i], &y->toks[i]);
      if (c)
	return c;
    }
  return 0;
}

static void slurp(struct tokenizer *t, struct shouffle *s)
{
  read_input(t, &s->tb);
  s->tok_array = xmalloc(sizeof(struct tok) * s->tb.num_tokens);
  s->line_array = xmalloc(sizeof(struct line) * (s->tb.num_lines+1));
  s->num_lines = 0;

  struct tok *tok = s->tok_array;
  struct line *line = s->line_array;
  line->toks = tok;
  for (char *x = get_first(&s->tb); x; x = get_next(&s->tb))
    if (*x)
      {
	tok->token = x;
	tok->hash = string_hash(x);
	tok++;
      }
    else
      {
	line->len = tok - line->toks;
	if (shuffle_words)
	  qsort(line->toks, line->len, sizeof(struct tok), (int(*)(const void *, const void *)) compare_toks);
	line->hash = line_hash(line->toks, line->len);
	line->orig_line = ++s->num_lines;
#if 0
	for (struct tok *t=line->toks; t<tok; t++)
	  printf("<%08x|%s>", t->hash, t->token);
	printf(" -> %08x (%d)\n", line->hash, line->orig_line);
#endif
	line++;
	line->toks = tok;
      }
  if (line->toks != tok)
    die("Bug #41: unterminated line");
  if (s->num_lines != s->tb.num_lines)
    die("Bug #42: got %d lines, expected %d", s->num_lines, s->tb.num_lines);

  if (shuffle_lines)
    qsort(s->line_array, s->num_lines, sizeof(struct line), (int(*)(const void *, const void *)) compare_lines);
}

static void compare(struct shouffle *s1, struct shouffle *s2)
{
  if (s1->num_lines != s2->num_lines)
    {
      printf("Output has %d lines, expecting %d\n", s1->num_lines, s2->num_lines);
      exit(1);
    }

  uns errs = 0;
  for (uns i=0; i<s1->num_lines; i++)
    {
      struct line *l1 = &s1->line_array[i], *l2 = &s2->line_array[i];
      if (compare_lines(l1, l2))
	{
	  printf("Line %d does not match\n", l1->orig_line);
	  printf("  %08x(%d) vs. %08x(%d)\n", l1->hash, l1->orig_line, l2->hash, l2->orig_line);
	  errs++;
	}
    }
  if (errs)
    exit(1);
}

/*** Main ***/

static void usage(void)
{
  fprintf(stderr, "Usage: judge-shuff [<options>] <output> <correct>\n\
\n\
Options:\n\
-n\t\tIgnore newlines and match the whole input as a single line\n\
-e\t\tIgnore empty lines\n\
-l\t\tShuffle lines (i.e., ignore their order)\n\
-w\t\tShuffle words in each line\n\
-i\t\tIgnore case\n\
");
  exit(2);
}

int main(int argc, char **argv)
{
  struct tokenizer t1, t2;
  int opt;

  while ((opt = getopt(argc, argv, "nelwi")) >= 0)
    switch (opt)
      {
      case 'n':
	ignore_nl++;
	break;
      case 'e':
	ignore_empty++;
	break;
      case 'l':
	shuffle_lines++;
	break;
      case 'w':
	shuffle_words++;
	break;
      case 'i':
	ignore_case++;
	break;
      default:
	usage();
      }
  if (optind + 2 != argc)
    usage();

  tok_init(&t1, sopen_read(argv[optind]));
  tok_init(&t2, sopen_read(argv[optind+1]));
  if (!ignore_nl)
    t1.flags = t2.flags = TF_REPORT_LINES;

  struct shouffle s1, s2;
  slurp(&t1, &s1);
  slurp(&t2, &s2);

  compare(&s1, &s2);
  return 0;
}
