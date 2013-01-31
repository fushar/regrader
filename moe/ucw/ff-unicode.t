# Tests for the Unicode module

Name:	bput_utf8
Run:	../obj/ucw/ff-unicode-t bput_utf8
In:	0041 0048 004f 004a
Out:	41 48 4f 4a

Name:   bget_utf8_32
Run:    ../obj/ucw/ff-unicode-t bget_utf8_32
In:     fe 83 81
Out:    fffc

Name:   bput_utf16_be
Run:    ../obj/ucw/ff-unicode-t bput_utf16_be
In:     0041 004a 2a5f feff 0000 10ffff ffff 10000
Out:    00 41 00 4a 2a 5f fe ff 00 00 db ff df ff ff ff d8 00 dc 00

Name:   bput_utf16_le
Run:    ../obj/ucw/ff-unicode-t bput_utf16_le
In:     0041 004a 2a5f feff 0000 10ffff ffff 10000
Out:    41 00 4a 00 5f 2a ff fe 00 00 ff db ff df ff ff 00 d8 00 dc

Name:   bget_utf16_be (1)
Run:    ../obj/ucw/ff-unicode-t bget_utf16_be
In:     00 41 00 4a 2a 5f fe ff 00 00 db ff df ff ff ff d8 00 dc 00
Out:    0041 004a 2a5f feff 0000 10ffff ffff 10000

Name:   bget_utf16_be (2)
Run:    ../obj/ucw/ff-unicode-t bget_utf16_be
In:     dc 1a 2a 5f d8 01 d8 01 2a 5f d8 01
Out:    fffc 2a5f fffc 2a5f fffc

Name:   bget_utf16_le (1)
Run:    ../obj/ucw/ff-unicode-t bget_utf16_le
In:     41 00 4a 00 5f 2a ff fe 00 00 ff db ff df ff ff 00 d8 00 dc
Out:    0041 004a 2a5f feff 0000 10ffff ffff 10000

Name:   bget_utf16_le (2)
Run:    ../obj/ucw/ff-unicode-t bget_utf16_le
In:     1a dc 5f 2a 01 d8 01 d8 5f 2a 01 d8
Out:    fffc 2a5f fffc 2a5f fffc
