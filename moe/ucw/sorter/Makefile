# Makefile for the UCW Sorter (c) 2007 Martin Mares <mj@ucw.cz>

DIRS+=ucw/sorter

LIBUCW_MODS+=$(addprefix sorter/, config govern sbuck array)
LIBUCW_SORTER_INCLUDES=$(addprefix sorter/, array.h array-simple.h common.h s-fixint.h \
	s-internal.h s-multiway.h s-radix.h s-twoway.h sorter.h)
LIBUCW_INCLUDES+=$(LIBUCW_SORTER_INCLUDES)

ifdef CONFIG_DEBUG_TOOLS
PROGS+=$(o)/ucw/sorter/sort-test
endif

$(o)/ucw/sorter/sort-test: $(o)/ucw/sorter/sort-test.o $(LIBUCW)

INSTALL_TARGETS+=install-ucw-sorter
install-ucw-sorter:
	install -d -m 755 $(DESTDIR)$(INSTALL_INCLUDE_DIR)/ucw/sorter
	install -m 644 $(addprefix run/include/ucw/,$(LIBUCW_SORTER_INCLUDES)) $(DESTDIR)$(INSTALL_INCLUDE_DIR)/ucw/sorter/

.PHONY: install-ucw-sorter
