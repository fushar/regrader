# Makefile for the Ulimit Perl module  (c) 2003 Tomas Valla <tom@ucw.cz>

DIRS+=ucw/perl/Ulimit/arch/auto/UCW/Ulimit
ULIMIT_DIR=ucw/perl/Ulimit

PROGS+=$(o)/ucw/perl/Ulimit/Ulimit.pm

extras:: $(o)/ucw/perl/Ulimit/arch/auto/UCW/Ulimit/Ulimit.$(SOEXT)

$(o)/ucw/perl/Ulimit/arch/auto/UCW/Ulimit/Ulimit.$(SOEXT): $(o)/$(ULIMIT_DIR)/Ulimit.xs $(o)/$(ULIMIT_DIR)/Ulimit.pm $(o)/$(ULIMIT_DIR)/Makefile
	$(M)MAKE $@
	$(Q)cd $(o)/$(ULIMIT_DIR) && $(MAKE) -f Makefile $(MAKESILENT)
	$(Q)touch $@
	$(Q)cp $@ run/$(DATADIR)/

$(o)/$(ULIMIT_DIR)/Makefile: $(o)/$(ULIMIT_DIR)/Makefile.PL
	$(M)PREPARE $@
	$(Q)cd $(o)/$(ULIMIT_DIR) && perl Makefile.PL

$(o)/$(ULIMIT_DIR)/Ulimit.xs: $(s)/$(ULIMIT_DIR)/Ulimit.xs
	$(Q)cp $^ $@

$(o)/$(ULIMIT_DIR)/Makefile.PL: $(s)/$(ULIMIT_DIR)/Makefile.PL
	$(Q)cp $^ $@

INSTALL_TARGETS+=install-perl-ucw-ulimit
install-perl-ucw-ulimit:
	install -d -m 755 $(DESTDIR)$(INSTALL_PERL_DIR)/UCW/ $(DESTDIR)$(INSTALL_LIB_DIR)
	install -m 644 $(s)/$(ULIMIT_DIR)/Ulimit.pm $(DESTDIR)$(INSTALL_PERL_DIR)/UCW/
	install -m 644 $(o)/ucw/perl/Ulimit/arch/auto/UCW/Ulimit/Ulimit.$(SOEXT) $(DESTDIR)$(INSTALL_LIB_DIR)
.PHONY: install-perl-ucw-ulimit
