# Tests for base64 and base224 modules

Name:	Base64 encode
Run:	../obj/ucw/basecode -e
Input:	Here are some test data
Output:	SGVyZSBhcmUgc29tZSB0ZXN0IGRhdGEK

Name:	Base64 decode
Run:	../obj/ucw/basecode -d
Input:	SGVyZSBhcmUgc29tZSB0ZXN0IGRhdGEK
Output:	Here are some test data

Name:	Base224 encode & decode
Run:	../obj/ucw/basecode -E | ../obj/ucw/basecode -D
Input:	Some more test data for 224 encoding
Output:	Some more test data for 224 encoding
