/*
 *	A simple library for Moe judges
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 */

#include <sys/types.h>

/* GCC extensions */

#ifdef __GNUC__
#define NONRET __attribute__((noreturn))
#else
#define NONRET
#endif

/* utils.c: Utility functions */

void die(char *msg, ...) NONRET;		/* Dies with exit code 2 (judge error) */
void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);

/* io.c: Simple buffered I/O streams */

struct stream {
  char *name;
  int fd;
  unsigned char *pos, *stop, *end;
  unsigned char buf[];
};

struct stream *sopen_read(char *name);
struct stream *sopen_write(char *name);
struct stream *sopen_fd(char *name, int fd);
void sflush(struct stream *s);
void sclose(struct stream *s);

int sgetc_slow(struct stream *s);
int speekc_slow(struct stream *s);

static inline int sgetc(struct stream *s)
{
  return (s->pos < s->stop) ? *s->pos++ : sgetc_slow(s);
}

static inline int speekc(struct stream *s)
{
  return (s->pos < s->stop) ? *s->pos : speekc_slow(s);
}

static inline void sputc(struct stream *s, int c)
{
  if (s->pos >= s->stop)
    sflush(s);
  *s->pos++ = c;
}

static inline void sungetc(struct stream *s)
{
  s->pos--;
}

/* token.c: Tokenization of input */

struct tokenizer {
  struct stream *stream;
  unsigned int bufsize;		// Allocated buffer size
  unsigned int maxsize;		// Maximal allowed token size
  unsigned int flags;		// TF_xxx
  unsigned char *token;		// Current token (in the buffer)
  unsigned int toksize;		// ... and its size
  int line;			// ... and line number at its end
};

enum tokenizer_flags {
  TF_REPORT_LINES = 1,		// Report an empty token at the end of each line
};

void tok_init(struct tokenizer *t, struct stream *s);
void tok_cleanup(struct tokenizer *t);
void tok_err(struct tokenizer *t, char *msg, ...) NONRET;
char *get_token(struct tokenizer *t);

// Parsing functions
int to_int(struct tokenizer *t, int *x);
int to_uint(struct tokenizer *t, unsigned int *x);
int to_long(struct tokenizer *t, long int *x);
int to_ulong(struct tokenizer *t, unsigned long int *x);
int to_double(struct tokenizer *t, double *x);
int to_long_double(struct tokenizer *t, long double *x);

// get_token() + parse or die
int get_int(struct tokenizer *t);
unsigned int get_uint(struct tokenizer *t);
long int get_long(struct tokenizer *t);
unsigned long int get_ulong(struct tokenizer *t);
double get_double(struct tokenizer *t);
long double get_long_double(struct tokenizer *t);
void get_nl(struct tokenizer *t);
