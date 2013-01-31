#!/bin/bash
# The Evaluator -- Master Control Script
# (c) 2001--2008 Martin Mares <mj@ucw.cz>

set -e
if [ ! -f cf/eval -o ! -f lib/libeval.sh ] ; then
	echo "Unable to find evaluator files!"
	exit 1
fi
. lib/libeval.sh
. cf/eval
while parse-cmdline "$1" ; do
	shift
done

[ -n "$2" -a -z "$4" ] || die "Usage: ev [<var>=<value>] <contestant> <problem> [<program>]"
CONTESTANT=$1
PROBLEM=$2
dir-init
log-init
. $PDIR/config
box-init

# Compile the program
if [ $TASK_TYPE != open-data ] ; then
	locate-source `if [ -n "$3" ] ; then echo $SDIR/$3 ; fi`
	compile || true
fi

# Initialize the points file
PTSFILE=$TDIR/points
>$PTSFILE

function test-verdict
{
	if [ $1 == 0 ] ; then
		pend "$2"
	else
		pend "$2 ($1 points)"
	fi
	echo >>$PTSFILE "$TEST $1 $2"
	exit 0
}

# Perform the tests
[ -z "$EV_SAMPLE" ] || TESTS="$SAMPLE_TESTS $TESTS"
for TEST in $TESTS ; do
	(
	exec >$TDIR/$TEST.log
	test-config
	pstart "Test $TEST... "
	echo "Test $TEST ($POINTS_PER_TEST points)"
	test-run
	syntax-check
	output-check
	die "You must never see this message."
	)
done
