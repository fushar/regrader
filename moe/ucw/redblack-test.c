/*
 *	Test of red-black trees
 *
 *	(c) 2002, Robert Spalek <robert@ucw.cz>
 */

#include "ucw/lib.h"
#include "ucw/getopt.h"
#include "ucw/fastbuf.h"
#include <stdio.h>
#include <stdlib.h>

struct my1_node
{
	int key;
	int x;
};

static void my_dump_key(struct fastbuf *fb, struct my1_node *n)
{
	char tmp[20];
	sprintf(tmp, "key=%d ", n->key);
	bputs(fb, tmp);
}

static void my_dump_data(struct fastbuf *fb, struct my1_node *n)
{
	char tmp[20];
	sprintf(tmp, "x=%d ", n->x);
	bputs(fb, tmp);
}

#define TREE_NODE struct my1_node
#define TREE_PREFIX(x) my_##x
#define TREE_KEY_ATOMIC key
#define TREE_WANT_CLEANUP
#define TREE_WANT_LOOKUP
#define TREE_WANT_DELETE
#define TREE_WANT_ITERATOR
#define TREE_WANT_DUMP
#define TREE_CONSERVE_SPACE
#include "redblack.h"

static void my_check_order(struct fastbuf *fb, struct my_tree *t)
{
	int last_key = 0x80000000;
	TREE_FOR_ALL(my, t, n)
	{
		ASSERT(n->key >= last_key);
		last_key = n->key;
		if (fb)
		{
			char tmp[30];
			sprintf(tmp, "%d -> %d\n", n->key, n->x);
			bputs(fb, tmp);
		}
	}
	TREE_END_FOR;
	if (fb)
		bflush(fb);
}

struct my2_node
{
	char key[1];
};

static void my2_dump_key(struct fastbuf *fb, struct my2_node *n)
{
	bputs(fb, "key=");
	bputs(fb, n->key);
	bputc(fb, ' ');
}

static void my2_dump_data(struct fastbuf *fb UNUSED, struct my2_node *n UNUSED)
{
}

#define TREE_NODE struct my2_node
#define TREE_PREFIX(x) my2_##x
#define TREE_KEY_ENDSTRING key
#define TREE_NOCASE
#define TREE_WANT_CLEANUP
#define TREE_WANT_NEW
#define TREE_WANT_SEARCH
#define TREE_WANT_REMOVE
#define TREE_WANT_FIND_NEXT
#define TREE_WANT_ITERATOR
#define TREE_WANT_DUMP
#define TREE_STATIC
#define TREE_CONSERVE_SPACE
#include "redblack.h"

static void random_string(char *txt, uns max_len)
{
	uns len = random() % max_len;
	uns j;
	for (j=0; j<len; j++)
		txt[j] = random() % 96 + 32;
	txt[len] = 0;
}

static char *options = CF_SHORT_OPTS "vn:a";

static char *help = "\
Usage: test1.bin <options>\n\
Options:\n"
CF_USAGE
"-v\tSet verbose mode\n\
-n num\tNumber of inserted nodes\n\
-a\tProbe some ASSERTs\n\
";

static void NONRET
usage(void)
{
	fputs(help, stderr);
	exit(1);
}

int
main(int argc, char **argv)
{
	int verbose = 0, number = 1000, asserts = 0;
	int opt;
	struct fastbuf *fb, *dump_fb;
	struct my_tree t;
	struct my2_tree t2;
	int i;
	cf_def_file = NULL;
	log_init(argv[0]);
	while ((opt = cf_getopt(argc, argv, options, CF_NO_LONG_OPTS, NULL)) >= 0)
		switch (opt)
		{
			case 'v':
				verbose++;
				break;
			case 'n':
				number = atoi(optarg);
				break;
			case 'a':
				asserts++;
				break;
			default:
				usage();
				break;
		}
	if (optind < argc)
		usage();
	fb = bfdopen(1, 4096);
	if (verbose > 1)
		dump_fb = fb;
	else
		dump_fb = NULL;

	my_init(&t);
	for (i=0; i<number; i++)
		my_lookup(&t, random() % 1000000)->x = i;
	my_dump(dump_fb, &t);
	my_check_order(dump_fb, &t);
	if (asserts)
	{
		my_new(&t, 1);
		my_new(&t, 1);
	}
	my_cleanup(&t);
	if (verbose > 0)
		bputs(fb, "Load test passed\n");

	my_init(&t);
	for (i=0; i<100; i++)
	{
		my_new(&t, i)->x = i;
		my_dump(dump_fb, &t);
	}
	for (i=0; i<100; i++)
	{
		int a = i/10, b = i%10, j = a*10 + (b + a) % 10;
		int res UNUSED = my_delete(&t, j);
		ASSERT(res);
		my_dump(dump_fb, &t);
	}
	my_cleanup(&t);
	if (verbose > 0)
		bputs(fb, "Sequential adding and deleting passed\n");

	my_init(&t);
	for (i=0; i<997; i++)
	{
		my_new(&t, i*238 % 997)->x = i;
		my_dump(NULL, &t);
	}
	my_dump(dump_fb, &t);
	i = 0;
	TREE_FOR_ALL(my, &t, n)
	{
		ASSERT(n->key == i);
		i++;
	}
	TREE_END_FOR;
	ASSERT(i == 997);
	for (i=0; i<997; i++)
	{
		int res UNUSED = my_delete(&t, i*111 % 997);
		ASSERT(res);
		my_dump(NULL, &t);
	}
	my_dump(dump_fb, &t);
	my_cleanup(&t);
	if (verbose > 0)
		bputs(fb, "Complete tree passed\n");

	my2_init(&t2);
	for (i=0; i<number; i++)
	{
		char txt[30];
		random_string(txt, 30);
		my2_new(&t2, txt);
	}
	my2_dump(dump_fb, &t2);
	TREE_FOR_ALL(my2, &t2, n)
	{
		my2_node *tmp;
		int count = 0;
		for (tmp=n; tmp; tmp = my2_find_next(tmp))
			count++;
		if (dump_fb)
		{
			char txt[20];
			bputs(dump_fb, n->key);
			sprintf(txt, ": %d\n", count);
			bputs(dump_fb, txt);
		}
	}
	TREE_END_FOR;
	while (t2.count > 0)
	{
		char txt[30];
		my2_node *n;
		random_string(txt, 30);
		n = my2_search(&t2, txt);
		ASSERT(n);
		my2_remove(&t2, n);
	}
	my2_dump(dump_fb, &t2);
	my2_cleanup(&t2);
	if (verbose > 0)
		bputs(fb, "String test passed\n");

	bclose(fb);
	return 0;
}
