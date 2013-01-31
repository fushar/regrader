/*
 *	UCW Library -- Line utility for encoding and decoding base64 & base224
 *
 *	(c) 2008, Michal Vaner <vorner@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ucw/lib.h"
#include "ucw/base64.h"
#include "ucw/base224.h"
#include "ucw/fastbuf.h"
#include "ucw/getopt.h"

static struct option opts[] = {
  { "encode64", 0, 0, 'e' },
  { "decode64", 0, 0, 'd' },
  { "encode224", 0, 0, 'E' },
  { "decode224", 0, 0, 'D' },
  { "prefix", 1, 0, 'p' },
  { "blocks", 1, 0, 'b' },
  { 0, 0, 0, 0 }
};

static const struct {
  uns (*function)(byte *, const byte *, uns);
  uns in_block, out_block, num_blocks;
  uns add_prefix;
} actions[] = {
  {
    base64_encode,
    BASE64_IN_CHUNK, BASE64_OUT_CHUNK, 20,
    1
  },
  {
    base64_decode,
    BASE64_OUT_CHUNK, BASE64_IN_CHUNK, 20,
    0
  },
  {
    base224_encode,
    BASE224_IN_CHUNK, BASE64_OUT_CHUNK, 6,
    1
  },
  {
    base224_decode,
    BASE224_OUT_CHUNK, BASE224_IN_CHUNK, 6,
    0
  }
};

int main(int argc, char **argv)
{
  // Choose mode
  int mode = -1;
  char *prefix = NULL;
  uns blocks = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, "edEDp:b:", opts, NULL)) >= 0)
    switch (opt)
    {
      case 'e': mode = 0; break;
      case 'd': mode = 1; break;
      case 'E': mode = 2; break;
      case 'D': mode = 3; break;
      case 'p': prefix = optarg; break;
      case 'b':
        {
          char *end;
          blocks = strtol(optarg, &end, 0);
          if ((blocks > 0) && !*end)
            break;
        }
      default: goto usage;
    }

  if (mode == -1)
  {
    usage:
    fprintf(stderr, "basecode mode [--prefix=prefix] [--blocks=number_of_blocks]\nMode is one of:\n\t--encode64 (-e)\n\t--decode64 (-d)\n\t--encode224 (-E)\n\t--decode224 (-D)\n");
    return 1;
  }
  if (!blocks)
    blocks = actions[mode].num_blocks;

  // Prepare buffers
  struct fastbuf *in = bfdopen_shared(0, 4096);
  struct fastbuf *out = bfdopen_shared(1, 4096);
  int has_offset = !actions[mode].add_prefix && prefix;
  uns offset = has_offset ? strlen(prefix) : 0;
  uns read_size = actions[mode].in_block * blocks + offset + has_offset;
  uns write_size = actions[mode].out_block * blocks;
  byte in_buff[read_size], out_buff[write_size];
  uns isize;

  // Recode it
  while (isize = bread(in, in_buff, read_size))
  {
    if (prefix)
    {
      if (actions[mode].add_prefix)
        bputs(out, prefix);
      else
        if ((isize < offset) || (in_buff[isize-1] != '\n')
            || (strncmp(prefix, in_buff, offset)))
          die("Invalid line syntax");
    }
    uns osize = actions[mode].function(out_buff, in_buff + offset, isize - offset - has_offset);
    bwrite(out, out_buff, osize);
    if (actions[mode].add_prefix && prefix)
      bputc(out, '\n');
  }

  bclose(in);
  bclose(out);
  return 0;
}
