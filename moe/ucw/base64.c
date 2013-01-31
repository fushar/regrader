/*
 *	UCW Library -- Base 64 Encoding & Decoding
 *
 *	(c) 2002, Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/base64.h"

#include <string.h>

static const byte base64_table[] =
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
	};
static const byte base64_pad = '=';

uns
base64_encode(byte *dest, const byte *src, uns len)
{
	const byte *current = src;
	uns i = 0;

	while (len > 2) { /* keep going until we have less than 24 bits */
		dest[i++] = base64_table[current[0] >> 2];
		dest[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		dest[i++] = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		dest[i++] = base64_table[current[2] & 0x3f];

		current += 3;
		len -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (len != 0) {
		dest[i++] = base64_table[current[0] >> 2];
		if (len > 1) {
			dest[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			dest[i++] = base64_table[(current[1] & 0x0f) << 2];
			dest[i++] = base64_pad;
		}
		else {
			dest[i++] = base64_table[(current[0] & 0x03) << 4];
			dest[i++] = base64_pad;
			dest[i++] = base64_pad;
		}
	}
	return i;
}

/* as above, but backwards. :) */
uns
base64_decode(byte *dest, const byte *src, uns len)
{
	const byte *current = src;
	uns ch;
	uns i = 0, j = 0;
	static byte reverse_table[256];
	static uns table_built = 0;

	if (table_built == 0) {
		byte *chp;
		table_built = 1;
		for(ch = 0; ch < 256; ch++) {
			chp = strchr(base64_table, ch);
			if(chp) {
				reverse_table[ch] = chp - base64_table;
			} else {
				reverse_table[ch] = 0xff;
			}
		}
	}

	/* run through the whole string, converting as we go */
	ch = 0;
	while (len > 0) {
		len--;
		ch = *current++;
		if (ch == base64_pad) break;

		/* When Base64 gets POSTed, all pluses are interpreted as spaces.
		   This line changes them back.  It's not exactly the Base64 spec,
		   but it is completely compatible with it (the spec says that
		   spaces are invalid).  This will also save many people considerable
		   headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
		 */

		if (ch == ' ') ch = '+';

		ch = reverse_table[ch];
		if (ch == 0xff) continue;

		switch(i % 4) {
		case 0:
			dest[j] = ch << 2;
			break;
		case 1:
			dest[j++] |= ch >> 4;
			dest[j] = (ch & 0x0f) << 4;
			break;
		case 2:
			dest[j++] |= ch >>2;
			dest[j] = (ch & 0x03) << 6;
			break;
		case 3:
			dest[j++] |= ch;
			break;
		}
		i++;
	}
	return j;
}
