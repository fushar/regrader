# Makefile for MO-P contest environment
# (c) 2008 Martin Mares <mj@ucw.cz>

DIRS+=mop/public
PROGS+=$(addprefix $(o)/mop/public/,check compile status submit)

$(o)/mop/public/check: $(s)/mop/public/check.sh
$(o)/mop/public/compile: $(s)/mop/public/compile.sh
$(o)/mop/public/status: $(s)/mop/public/status.sh
$(o)/mop/public/submit: $(s)/mop/public/submit.sh
