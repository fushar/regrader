#!/bin/bash
# The Evaluator -- Public Status Script
# (c) 2004 Martin Mares <mj@ucw.cz>

set -e
[ -n "$MO_ROOT" -a -d "$MO_ROOT" ] || { echo >&2 "MO_ROOT not set, giving up." ; exit 1 ; }
pushd $MO_ROOT >/dev/null
. lib/libeval.sh
. cf/mop
popd >/dev/null

[ -z "$1" ] || die "Usage: status"

echo -e "Submitted tasks:\n"

if [ -n "$REMOTE_SUBMIT" ] ; then
	exec $MO_ROOT/bin/remote-status
fi

for PROBLEM in `cd $MO_ROOT/problems/ ; echo *` ; do
	(
	PDIR=$MO_ROOT/problems/$PROBLEM
	SUBDIR=~/.submit/$PROBLEM
	[ -f $PDIR/config ] || exit 0
	echo -n "$PROBLEM: "
	. $PDIR/config
	if [ -d $SUBDIR ] ; then
		if [ $TASK_TYPE == open-data ] ; then
			for X in $TESTS ; do
				[ -f $SUBDIR/$X.out ] && echo -n "$X " || echo -n "- "
			done
			echo
		else
			C=0
			for X in $EXTENSIONS ; do
				if [ -f $SUBDIR/$PROBLEM.$X ] ; then
					echo -n `basename $SUBDIR/$PROBLEM.$X`
					C=$(($C+1))
				fi
			done
			if [ $C == 0 ] ; then
				echo ---
			elif [ $C == 1 ] ; then
				echo
			else
				echo "INCONSISTENT (you probably modified $SUBDIR manually)"
			fi
		fi
	else
		echo ---
	fi
	)
done
