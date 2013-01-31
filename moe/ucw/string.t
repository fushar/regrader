# Tests of string routines

Run:	../obj/ucw/str-hex-t
Out:	FEEDF00D
	fe:ed:f0:0d
	feedf00d

Run:	../obj/ucw/str-esc-t '12\r\n\000\\\xff'
Out:	31 32 0d 0a 00 5c ff

Run:	../obj/ucw/str-esc-t '\100\10a\1a'
Out:	40 08 61 01 61

Run:	../obj/ucw/str-esc-t '\a\b\f\r\n\t\v\?\"'"\\'"
Out:	07 08 0c 0d 0a 09 0b 3f 22 27
