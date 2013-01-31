# Test for fb-atomic

Run:	../obj/ucw/fb-atomic-t 2>&1 | sed -e 's/^\(.\) [^ ]* [^ ]* /\1 /'
Out:	I Testing block writes
	I Testing interleaved var-size writes
	D FB_ATOMIC: Reallocating buffer for atomic file test2 with slack 10
	D FB_ATOMIC: Reallocating buffer for atomic file test2 with slack 10
