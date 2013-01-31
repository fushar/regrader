# Makefile for Moe evaluator
# (c) 2008 Martin Mares <mj@ucw.cz>

DIRS+=eval
PROGS+=$(addprefix $(o)/eval/,iwrapper ev)
DATAFILES+=$(o)/eval/libeval.sh

$(o)/eval/iwrapper: $(o)/eval/iwrapper.o
$(o)/eval/ev: $(s)/eval/ev.sh

CONFIGS+=eval
run/cf/eval: $(o)/eval/eval.cf
