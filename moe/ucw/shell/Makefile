# Support routines for shell scripts

DIRS+=ucw/shell
UCW_SHELL_PROGS=$(addprefix $(o)/ucw/shell/,config logger)
PROGS+=$(UCW_SHELL_PROGS)
DATAFILES+=$(o)/ucw/shell/libucw.sh

$(o)/ucw/shell/config: $(o)/ucw/shell/config.o $(LIBUCW)
$(o)/ucw/shell/logger: $(o)/ucw/shell/logger.o $(LIBUCW)

TESTS+=$(addprefix $(o)/ucw/shell/,config.test)

$(o)/ucw/shell/config.test: $(o)/ucw/shell/config

INSTALL_TARGETS+=install-ucw-shell
install-ucw-shell:
	install -d -m 755 $(DESTDIR)$(INSTALL_BIN_DIR)
	install -m 755 $(UCW_SHELL_PROGS) $(DESTDIR)$(INSTALL_BIN_DIR)

.PHONY: install-ucw-shell
