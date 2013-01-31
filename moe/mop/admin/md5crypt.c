/*
 * md5crypt based on lib/md5crypt.c from Linux shadow package.
 * adapted by Martin Mares <mj@ucw.cz> in June 2004
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dknet.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include "ucw/lib.h"
#include "ucw/md5.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static unsigned char itoa64[] =		/* 0 ... 63 => ascii - 64 */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void
to64(char *s, unsigned int v, int n)
{
  while (--n >= 0)
    {
      *s++ = itoa64[v&0x3f];
      v >>= 6;
    }
}

static char *
libshadow_md5_crypt(const char *pw, const char *salt)
{
	static char	*magic = "$1$";	/*
						 * This string is magic for
						 * this algorithm.  Having
						 * it this way, we can get
						 * get better later on
						 */
	static char     passwd[120], *p;
	static const char *sp,*ep;
	unsigned char	final[16];
	int sl,pl,i,j;
	md5_context ctx,ctx1;
	unsigned long l;

	/* Refine the Salt first */
	sp = salt;

	/* If it starts with the magic string, then skip that */
	if(!strncmp(sp,magic,strlen(magic)))
		sp += strlen(magic);

	/* It stops at the first '$', max 8 chars */
	for(ep=sp;*ep && *ep != '$' && ep < (sp+8);ep++)
		continue;

	/* get the length of the true salt */
	sl = ep - sp;

	md5_init(&ctx);

	/* The password first, since that is what is most unknown */
	md5_update(&ctx,pw,strlen(pw));

	/* Then our magic string */
	md5_update(&ctx,magic,strlen(magic));

	/* Then the raw salt */
	md5_update(&ctx,sp,sl);

	/* Then just as many characters of the MD5(pw,salt,pw) */
	md5_init(&ctx1);
	md5_update(&ctx1,pw,strlen(pw));
	md5_update(&ctx1,sp,sl);
	md5_update(&ctx1,pw,strlen(pw));
	memcpy(final, md5_final(&ctx1), 16);
	for(pl = strlen(pw); pl > 0; pl -= 16)
		md5_update(&ctx,final,pl>16 ? 16 : pl);

	/* Don't leave anything around in vm they could use. */
	memset(final,0,sizeof final);

	/* Then something really weird... */
	for (j=0,i = strlen(pw); i ; i >>= 1)
		if(i&1)
		    md5_update(&ctx, final+j, 1);
		else
		    md5_update(&ctx, pw+j, 1);

	/* Now make the output string */
	strcpy(passwd,magic);
	strncat(passwd,sp,sl);
	strcat(passwd,"$");

	memcpy(final, md5_final(&ctx), 16);

	/*
	 * and now, just to make sure things don't run too fast
	 * On a 60 Mhz Pentium this takes 34 msec, so you would
	 * need 30 seconds to build a 1000 entry dictionary...
	 */
	for(i=0;i<1000;i++) {
		md5_init(&ctx1);
		if(i & 1)
			md5_update(&ctx1,pw,strlen(pw));
		else
			md5_update(&ctx1,final,16);

		if(i % 3)
			md5_update(&ctx1,sp,sl);

		if(i % 7)
			md5_update(&ctx1,pw,strlen(pw));

		if(i & 1)
			md5_update(&ctx1,final,16);
		else
			md5_update(&ctx1,pw,strlen(pw));
		memcpy(final, md5_final(&ctx1), 16);
	}

	p = passwd + strlen(passwd);

	l = (final[ 0]<<16) | (final[ 6]<<8) | final[12]; to64(p,l,4); p += 4;
	l = (final[ 1]<<16) | (final[ 7]<<8) | final[13]; to64(p,l,4); p += 4;
	l = (final[ 2]<<16) | (final[ 8]<<8) | final[14]; to64(p,l,4); p += 4;
	l = (final[ 3]<<16) | (final[ 9]<<8) | final[15]; to64(p,l,4); p += 4;
	l = (final[ 4]<<16) | (final[10]<<8) | final[ 5]; to64(p,l,4); p += 4;
	l =                    final[11]                ; to64(p,l,2); p += 2;
	*p = '\0';

	/* Don't leave anything around in vm they could use. */
	memset(final,0,sizeof final);

	return passwd;
}

int main(void)
{
  char pass[256], salt[10], rand[8];
  int fd = open("/dev/urandom", O_RDONLY);
  int n;
  if (fd < 0)
    {
      fprintf(stderr, "unable to open /dev/urandom: %m\n");
      return 1;
    }
  while (fgets(pass, sizeof(pass), stdin))
    {
      char *c = strchr(pass, '\n');
      if (c)
	*c = 0;
      if (read(fd, rand, sizeof(rand)) != sizeof(rand))
	{
	  fprintf(stderr, "Error reading /dev/urandom: %m\n");
	  return 1;
	}
      for (n=0; n<2; n++)
	to64(salt+4*n, *(u32 *)(rand+4*n), 4);
      salt[8] = 0;
      printf("%s\n", libshadow_md5_crypt(pass, salt));
    }
  return 0;
}
