# Makefile for Sherlock Build Tools

DIRS+=build

$(o)/build/genhash: $(o)/build/genhash.o

# This is a hack which compensates make's desires for propagating per-rule settings
# of variables: if some module specifies its own LIBS and it depends on genhash,
# genhash is sometimes built with the module's LIBS (if it isn't already built).
# A proper solution would be using a different rule for linking build/*, but
# as it currently concerns only genhash, it's easier to battle this way.
$(o)/build/genhash: LIBS=

INSTALL_TARGETS+=install-build
install-build:
	install -d -m 755 $(DESTDIR)$(INSTALL_SHARE_DIR)/ucw/build
	install -m 755 $(addprefix $(BUILDSYS)/,install-includes doc-defs doc-extract genconf mergedeps tester lib-deps lib-flags) $(DESTDIR)$(INSTALL_SHARE_DIR)/ucw/build
	install -m 644 $(addprefix $(BUILDSYS)/,asciidoc.conf asciidoc-xhtml.conf Makebottom Maketop) $(DESTDIR)$(INSTALL_SHARE_DIR)/ucw/build
.PHONY: install-build
