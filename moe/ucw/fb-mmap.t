# Test for fb-mmap.c

Run:	dd bs=1024 count=1024 if=/dev/zero of=mmap.in 2>/dev/null && ../obj/ucw/fb-mmap-t mmap.in mmap.out && rm mmap.in mmap.out
