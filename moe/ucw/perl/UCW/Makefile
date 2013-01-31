# More Perl modules

DIRS+=ucw/perl/UCW
EXTRA_RUNDIRS+=lib/perl5/UCW
UCW_PERL_MODULES=$(addsuffix .pm,Config Log CGI Configure)
PROGS+=$(addprefix $(o)/ucw/perl/UCW/,$(UCW_PERL_MODULES))

include $(s)/ucw/perl/UCW/Configure/Makefile

INSTALL_TARGETS+=install-perl-ucw
install-perl-ucw:
	install -d -m 755 $(DESTDIR)$(INSTALL_PERL_DIR)/UCW
	install -m 644 $(addprefix $(s)/ucw/perl/UCW/,$(UCW_PERL_MODULES)) $(DESTDIR)$(INSTALL_PERL_DIR)/UCW
.PHONY: install-perl-ucw
