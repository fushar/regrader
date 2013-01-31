/* Test of large files */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"

#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	BLOCK	(1<<10)
#define	COUNT	(5<<20)
#define	TESTS	(1<<20)

int main(void)
{
	struct fastbuf *b;
	byte block[BLOCK];
	uns i;

	srand(time(NULL));
#if 0
	b = bopen("/big/robert/large-file", O_CREAT | O_TRUNC | O_RDWR, 1<<20);
	if (!b)
		die("Cannot create large-file");

	msg(L_DEBUG, "Writing %d blocks of size %d", COUNT, BLOCK);
	for (i=0; i<COUNT; i++)
	{
		memset(block, i & 0xff, BLOCK);
		bwrite(b, block, BLOCK);
		if ( i%1024 == 0 )
		{
			printf("\r%10d", i);
			fflush(stdout);
		}
	}
#else
	b = bopen("/big/robert/large-file", O_RDWR, 1<<20);
	if (!b)
		die("Cannot create large-file");
#endif
	msg(L_DEBUG, "Checking the file contents in %d tests", TESTS);
	for (i=0; i<TESTS; i++)
	{
		uns idx = random()%COUNT;
		ucw_off_t ofs = idx*BLOCK;
		bseek(b, ofs, SEEK_SET);
		bread(b, block, BLOCK);
		if (block[17] != (idx & 0xff))
			die("Invalid block %d in test %d: %x != %x", idx, i, block[17], idx & 0xff);
		if ( i%16 == 0 )
		{
			printf("\r%10d", i);
			fflush(stdout);
		}
	}
	msg(L_DEBUG, "Done");

	bclose(b);
	return 0;
}
