/*
 *	LiZaRd -- Fast compression method based on Lempel-Ziv 77
 *
 *	(c) 2004, Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/threads.h"
#include "ucw/lizard.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

struct lizard_buffer {
  uns len;
  void *ptr;
};

struct lizard_buffer *
lizard_alloc(void)
{
  struct lizard_buffer *buf = xmalloc(sizeof(struct lizard_buffer));
  buf->len = 0;
  buf->ptr = NULL;
  handle_signal(SIGSEGV);
  return buf;
}

void
lizard_free(struct lizard_buffer *buf)
{
  unhandle_signal(SIGSEGV);
  if (buf->ptr)
    munmap(buf->ptr, buf->len + CPU_PAGE_SIZE);
  xfree(buf);
}

static void
lizard_realloc(struct lizard_buffer *buf, uns max_len)
  /* max_len needs to be aligned to CPU_PAGE_SIZE */
{
  if (max_len <= buf->len)
    return;
  if (max_len < 2*buf->len)				// to ensure logarithmic cost
    max_len = 2*buf->len;

  if (buf->ptr)
    munmap(buf->ptr, buf->len + CPU_PAGE_SIZE);
  buf->len = max_len;
  buf->ptr = mmap(NULL, buf->len + CPU_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (buf->ptr == MAP_FAILED)
    die("mmap(anonymous, %d bytes): %m", (uns)(buf->len + CPU_PAGE_SIZE));
  if (mprotect(buf->ptr + buf->len, CPU_PAGE_SIZE, PROT_NONE) < 0)
    die("mprotect: %m");
}

static jmp_buf safe_decompress_jump;
static int
sigsegv_handler(int signal UNUSED)
{
  longjmp(safe_decompress_jump, 1);
  return 1;
}

byte *
lizard_decompress_safe(const byte *in, struct lizard_buffer *buf, uns expected_length)
  /* Decompresses in into buf, sets *ptr to the data, and returns the
   * uncompressed length.  If an error has occured, -1 is returned and errno is
   * set.  The buffer buf is automatically reallocated.  SIGSEGV is caught in
   * case of buffer-overflow.  The function is not re-entrant because of a
   * static longjmp handler.  */
{
  uns lock_offset = ALIGN_TO(expected_length + 3, CPU_PAGE_SIZE);	// +3 due to the unaligned access
  if (lock_offset > buf->len)
    lizard_realloc(buf, lock_offset);
  volatile ucw_sighandler_t old_handler = set_signal_handler(SIGSEGV, sigsegv_handler);
  byte *ptr;
  if (!setjmp(safe_decompress_jump))
  {
    ptr = buf->ptr + buf->len - lock_offset;
    int len = lizard_decompress(in, ptr);
    if (len != (int) expected_length)
    {
      ptr = NULL;
      errno = EINVAL;
    }
  }
  else
  {
    msg(L_ERROR, "SIGSEGV caught in lizard_decompress()");
    ptr = NULL;
    errno = EFAULT;
  }
  set_signal_handler(SIGSEGV, old_handler);
  return ptr;
}
