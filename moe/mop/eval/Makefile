# Makefile for MO-P contest environment
# (c) 2008 Martin Mares <mj@ucw.cz>

DIRS+=mop/eval
PROGS+=$(addprefix $(o)/mop/eval/,\
	mo-ev-all mo-ev-inc mo-get-users \
	mo-score mo-report)

$(o)/mop/eval/mo-ev-all: $(s)/mop/eval/mo-ev-all.sh
$(o)/mop/eval/mo-ev-inc: $(s)/mop/eval/mo-ev-inc.sh
$(o)/mop/eval/mo-get-users: $(s)/mop/eval/mo-get-users.sh
$(o)/mop/eval/mo-report: $(s)/mop/eval/mo-report.sh
$(o)/mop/eval/mo-score: $(s)/mop/eval/mo-score.sh
