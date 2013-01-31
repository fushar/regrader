# Tests for url.c

Name:	Absolute
Run:	../obj/ucw/url-t 'ftp://example.com/other'
Out:	deesc: ftp://example.com/other
	split: @ftp@(null)@(null)@example.com@-1@/other
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @ftp@(null)@(null)@example.com@21@/other
	canonicalize: @ftp@(null)@(null)@example.com@21@/other
	pack: ftp://example.com/other
	enesc: ftp://example.com/other

Name:	Simple
Run:	../obj/ucw/url-t 'object'
Out:	deesc: object
	split: @(null)@(null)@(null)@(null)@-1@object
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/object
	canonicalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/object
	pack: http://mj@www.hell.org/123/sub_dir;param/object
	enesc: http://mj@www.hell.org/123/sub_dir;param/object

Name:	Toplevel
Run:	../obj/ucw/url-t '/object'
Out:	deesc: /object
	split: @(null)@(null)@(null)@(null)@-1@/object
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/object
	canonicalize: @http@mj@(null)@www.hell.org@80@/object
	pack: http://mj@www.hell.org/object
	enesc: http://mj@www.hell.org/object

Name:	Domain
Run:	../obj/ucw/url-t '//www.example.com'
Out:	deesc: //www.example.com
	split: @(null)@(null)@(null)@www.example.com@-1@
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@(null)@(null)@www.example.com@80@
	canonicalize: @http@(null)@(null)@www.example.com@80@/
	pack: http://www.example.com/
	enesc: http://www.example.com/

Name:	Levels
Run:	../obj/ucw/url-t '../a/b;paramb/c/.././x?a#frag'
Out:	deesc: ../a/b;paramb/c/.././x?a#frag
	split: @(null)@(null)@(null)@(null)@-1@../a/b;paramb/c/.././x?a#frag
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/123/a/b;paramb/x?a#frag
	canonicalize: @http@mj@(null)@www.hell.org@80@/123/a/b;paramb/x?a
	pack: http://mj@www.hell.org/123/a/b;paramb/x?a
	enesc: http://mj@www.hell.org/123/a/b;paramb/x?a

Name:	Query
Run:	../obj/ucw/url-t '?query'
Out:	deesc: ?query
	split: @(null)@(null)@(null)@(null)@-1@?query
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query
	canonicalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query
	pack: http://mj@www.hell.org/123/sub_dir;param/index.html;param?query
	enesc: http://mj@www.hell.org/123/sub_dir;param/index.html;param?query

Name:	Fragments
Run:	../obj/ucw/url-t '#../?a' 'http://example.com/?query#@?/x'
Out:	deesc: #../?a
	split: @(null)@(null)@(null)@(null)@-1@#../?a
	base: @http@(null)@(null)@example.com@80@/?query#@?/x
	normalize: @http@(null)@(null)@example.com@80@/?query#../?a
	canonicalize: @http@(null)@(null)@example.com@80@/?query
	pack: http://example.com/?query
	enesc: http://example.com/?query

Name:	Deescape
Run:	../obj/ucw/url-t '/%20%25'
Out:	deesc: / %
	split: @(null)@(null)@(null)@(null)@-1@/ %
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/ %
	canonicalize: @http@mj@(null)@www.hell.org@80@/ %
	pack: http://mj@www.hell.org/ %
	enesc: http://mj@www.hell.org/%20%25

Name:	Dots
Run:	../obj/ucw/url-t '..a/./x.?a/x'
Out:	deesc: ..a/./x.?a/x
	split: @(null)@(null)@(null)@(null)@-1@..a/./x.?a/x
	base: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/index.html;param?query&zzz/sub;query+#fragment?
	normalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/..a/x.?a/x
	canonicalize: @http@mj@(null)@www.hell.org@80@/123/sub_dir;param/..a/x.?a/x
	pack: http://mj@www.hell.org/123/sub_dir;param/..a/x.?a/x
	enesc: http://mj@www.hell.org/123/sub_dir;param/..a/x.?a/x
