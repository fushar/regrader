# Makefile for MO-P contest environment
# (c) 2008 Martin Mares <mj@ucw.cz>

DIRS+=mop

include $(s)/mop/admin/Makefile
include $(s)/mop/eval/Makefile
include $(s)/mop/public/Makefile
include $(s)/mop/score/Makefile
include $(s)/mop/calibrate/Makefile

CONFIGS+=mop userlist
run/cf/mop: $(o)/mop/mop.cf
run/cf/userlist: $(o)/mop/userlist.cf
