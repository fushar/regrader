# Tests for bitops modules

Run:	../obj/ucw/bit-ffs-t
In:	1
	2
	3
	4
	5
	6
	12345678
	23030300
	23030000
	23000000
	40000000
	80000000
Out:	0
	1
	0
	2
	0
	1
	3
	8
	16
	24
	30
	31

Run:	../obj/ucw/bit-fls-t
In:	1
	2
	3
	4
	5
	6
	12345678
	23030303
	03030303
	00030303
	00000303
	0fedcba9
Out:	0
	1
	1
	2
	2
	2
	28
	29
	25
	17
	9
	27
