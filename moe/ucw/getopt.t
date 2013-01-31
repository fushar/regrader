# Tests for getopt

Run:	../obj/ucw/getopt-t -a -b --longc 2819 -d -a 1 2 3
Out:	option a
	option b
	option c with value `2819'
	option d with value `-a'
	3 nonoption arguments
	reset
	option a
	option b
	option c with value `2819'
	option d with value `-a'
	3 nonoption arguments

Run:	../obj/ucw/getopt-t -a -x
Out:	option a
	unknown option
	reset
	option a
	unknown option
