# Makefile for Moe submitter
# (c) 2008 Martin Mares <mj@ucw.cz>

SUBCF=$(LIBGCRYPT_CFLAGS) $(LIBGNUTLS_CFLAGS) $(GNUTLS_CFLAGS)
SUBLF=$(LIBGCRYPT_LIBS) $(LIBGNUTLS_LIBS) $(GNUTLS_LIBS)

DIRS+=submit
SDIR=$(o)/submit

### The submit server ###

PROGS+=$(addprefix $(SDIR)/,submitd privkey connect)

$(SDIR)/submitd: $(addprefix $(SDIR)/,submitd.o commands.o tasks.o) $(LIBUCW) $(LIBSH)
$(SDIR)/submitd: CFLAGS+=$(SUBCF)
$(SDIR)/submitd: LIBS+=$(SUBLF)

$(SDIR)/connect: $(SDIR)/connect.o $(LIBUCW)
$(SDIR)/connect: CFLAGS+=$(SUBCF)
$(SDIR)/connect: LIBS+=$(SUBLF)

$(SDIR)/privkey: $(SDIR)/privkey.o $(LIBUCW)
$(SDIR)/privkey: CFLAGS+=$(SUBCF)
$(SDIR)/privkey: LIBS+=$(SUBLF)

CONFIGS+=submitd
run/cf/submitd: $(SDIR)/submitd.cf

### Submitter perl module ###

EXTRA_RUNDIRS+=lib/perl5/MO
SUBMIT_PERL_MODS=$(o)/submit/Submit.pm
PROGS+=$(SUBMIT_PERL_MODS)

$(SUBMIT_PERL_MODS): PERL_MODULE_DIR=MO

### The submit client and utilities ###

PROGS+=$(addprefix $(SDIR)/,contest create-certs remote-status remote-submit show-submits)

$(SDIR)/contest: $(s)/submit/contest.pl
$(SDIR)/create-certs: $(s)/submit/create-certs.sh
$(SDIR)/remote-status: $(s)/submit/remote-status.pl
$(SDIR)/remote-submit: $(s)/submit/remote-submit.pl
$(SDIR)/show-submits: $(s)/submit/show-submits.pl

DATAFILES+=$(addprefix $(SDIR)/,ca-cert.tpl client-cert.tpl server-cert.tpl)
