#!/bin/sh
# Time limit calibrator: Gathering of timing data
# (c) 2010 Martin Mares <mj@ucw.cz>

set -e
[ -n "$1" ]
t="$1"
mkdir -p cal/$t
for a in `cd solutions/authors/$t && echo * | sed 's/\.[^ ]*//g'` ; do
	bin/ev authors $t $a
	cp testing/authors/$t/points cal/$t/$a.points
	grep syscalls testing/authors/$t/*.log >cal/$t/$a.log
done
