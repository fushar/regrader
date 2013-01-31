# Tests of the SHA1 module

Name:	SHA1-1
Run:	echo -n "abc" | ../obj/ucw/sha1-t
Out:	a9993e364706816aba3e25717850c26c9cd0d89d

Name:	SHA1-2
Run:	echo -n "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" | ../obj/ucw/sha1-t
Out:	84983e441c3bd26ebaae4aa1f95129e5e54670f1

# Tests of SHA-1 HMAC specified in RFC 2202

Name:	HMAC1
Run:	../obj/ucw/sha1-hmac-t
In:	0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
	Hi There
Out:	b617318655057264e28bc0b6fb378c8ef146be00

Name:	HMAC2
Run:	../obj/ucw/sha1-hmac-t
In:	Jefe
	what do ya want for nothing?
Out:	effcdf6ae5eb2fa2d27416d5f184df9c259a7c79

Name:	HMAC3
Run:	../obj/ucw/sha1-hmac-t
In:	0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	0xdddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
Out:	125d7342b9ac11cd91a39af48aa17b4f63f175d3

Name:	HMAC4
Run:	../obj/ucw/sha1-hmac-t
In:	0x0102030405060708090a0b0c0d0e0f10111213141516171819
	0xcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
Out:	4c9007f4026250c6bc8414f9bf50c86c2d7235da

Name:	HMAC5
Run:	../obj/ucw/sha1-hmac-t
In:	0x0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c
	Test With Truncation
Out:	4c1a03424b55e07fe7f27be1d58bb9324a9a5a04

Name:	HMAC6
Run:	../obj/ucw/sha1-hmac-t
In:	0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	Test Using Larger Than Block-Size Key - Hash Key First
Out:	aa4ae5e15272d00e95705637ce8a3b55ed402112

Name:	HMAC7
Run:	../obj/ucw/sha1-hmac-t
In:	0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data
Out:	e8e99d0f45237d786d6bbaa7965c7808bbff1a91

Name:	HMAC8
Run:	../obj/ucw/sha1-hmac-t
In:	0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	Test Using Larger Than Block-Size Key - Hash Key First
Out:	aa4ae5e15272d00e95705637ce8a3b55ed402112

Name:	HMAC9
Run:	../obj/ucw/sha1-hmac-t
In:	0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	Test Using Larger Than Block-Size Key and Larger Than One Block-Size Data
Out:	e8e99d0f45237d786d6bbaa7965c7808bbff1a91
