# Makefile for the UCW utilities (c) 2008 Michal Vaner <vorner@ucw.cz>

UCW_UTILS=$(addprefix $(o)/ucw/utils/,basecode daemon-helper rotate-log urltool)
PROGS+=$(UCW_UTILS)
DIRS+=ucw/utils

$(o)/ucw/utils/basecode: $(o)/ucw/utils/basecode.o $(LIBUCW)
$(o)/ucw/utils/daemon-helper: $(o)/ucw/utils/daemon-helper.o $(LIBUCW)
$(o)/ucw/utils/urltool: $(o)/ucw/utils/urltool.o $(LIBUCW)

TESTS+=$(o)/ucw/utils/basecode.test
$(o)/ucw/utils/basecode.test: $(o)/ucw/utils/basecode

INSTALL_TARGETS+=install-ucw-utils
install-ucw-utils:
	install -d -m 755 $(DESTDIR)$(INSTALL_BIN_DIR)
	install -m 755 $(UCW_UTILS) $(DESTDIR)$(INSTALL_BIN_DIR)

.PHONY: install-ucw-utils
