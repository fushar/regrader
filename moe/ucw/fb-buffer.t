# Tests for fb-buffer.c

Name:	Read
Run:	../obj/ucw/fb-buffer-t r
Out:	Two
	lines

Name:	Write
Run:	../obj/ucw/fb-buffer-t w
Out:	Hello world

Name:	Overflow
Run:	../obj/ucw/fb-buffer-t o 2>&1 | grep 'buffer overflow on write'
