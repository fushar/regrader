# Tests for the regex module

Run:	../obj/ucw/regex-t 'a.*b.*c'
In:	abc
	ajkhkbbbbbc
	Aabc
Out:	MATCH
	MATCH
	NO MATCH

Run:	../obj/ucw/regex-t -i 'a.*b.*c'
In:	aBc
	ajkhkbBBBBC
	Aabc
Out:	MATCH
	MATCH
	MATCH

Run:	../obj/ucw/regex-t -i '(ahoj|nebo)'
In:	Ahoj
	nEBo
	ahoja
	(ahoj|nebo)
Out:	MATCH
	MATCH
	NO MATCH
	NO MATCH

Run:	../obj/ucw/regex-t '\(ahoj\)'
In:	(ahoj)
	ahoj
Out:	MATCH
	NO MATCH

Run:	../obj/ucw/regex-t '(.*b)*'
In:	ababababab
	ababababababababababababababababababababababababababababa
Out:	MATCH
	NO MATCH

Run:	../obj/ucw/regex-t '(.*)((aabb)|cc)(b.*)' '\1<\3>\4'
In:	aaabbb
	aabbccb
	abcabc
	aaccbb
Out:	a<aabb>b
	aabb<>b
	NO MATCH
	aa<>bb

Run:	../obj/ucw/regex-t '.*\?(.*&)*([a-z_]*sess[a-z_]*|random|sid|S_ID|rnd|timestamp|referer)=.*'
In:	/nemecky/ubytovani/hotel.php?sort=&cislo=26&mena=EUR&typ=Hotel&luz1=ANO&luz2=ANO&luz3=&luz4=&luz5=&maxp1=99999&maxp2=99999&maxp3=99999&maxp4=99999&maxp5=99999&apart=&rada=8,9,10,11,19,22,26,27,28,29,3&cislo=26&mena=EUR&typ=Hotel&luz1=ANO&luz2=ANO&luz3=&luz4=&luz5=&maxp1=99999&maxp2=99999&maxp3=99999&maxp4=99999&maxp5=99999&apart=&rada=8,9,10,11,19,22,26,27,28,29,3&cislo=26&mena=EUR&typ=Hotel&luz1=ANO&luz2=ANO&luz3=&luz4=&luz5=&maxp1=99999&maxp2=99999&maxp3=99999&maxp4=99999&maxp5=99999&apart=&rada=8,9,10,11,19,22,26,27,28,29,3
	/test...?f=1&s=3&sid=123&q=3&
Out:	NO MATCH
	MATCH

Run:	../obj/ucw/regex-t '.*[0-9a-f]{8,16}.*'
In:	abcdabcdabcd
	aaaaaaaaaaaaaaaaaaaaaaaaaaaa
	asddajlkdkajlqwepoiequwiouio
	000001111p101010101010q12032
Out:	MATCH
	MATCH
	NO MATCH
	MATCH
