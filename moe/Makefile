# Makefile for Moe
# (c) 2008--2012 Martin Mares <mj@ucw.cz>

VERSION=2.0

# The default target
all:: runtree programs datafiles configs

# Include configuration
s=.
-include obj/config.mk
obj/config.mk:
	@echo "You need to run configure first." && false

ifdef CONFIG_DOC
all:: docs
endif

# We will use the libucw build system
BUILDSYS=$(s)/build
include $(BUILDSYS)/Maketop

# Include makefiles of libraries we wish to use
ifdef CONFIG_UCW_LIBS
include $(s)/ucw/Makefile
include $(s)/sherlock/Makefile
# Disable built-in tests and documentation of these libraries
TESTS=
DOCS=
DOC_INDICES=
endif

include $(s)/box/Makefile
include $(s)/isolate/Makefile
include $(s)/utils/Makefile
include $(s)/eval/Makefile
include $(s)/judge/Makefile

ifdef CONFIG_SUBMIT
include $(s)/submit/Makefile
endif

ifdef CONFIG_MOP
include $(s)/mop/Makefile
endif

# And finally the default rules of the build system
include $(BUILDSYS)/Makebottom
