#!/bin/bash
# The Evaluator -- Public Checking Script
# (c) 2001--2008 Martin Mares <mj@ucw.cz>

set -e
[ -n "$MO_ROOT" -a -d "$MO_ROOT" ] || { echo >&2 "MO_ROOT not set, giving up." ; exit 1 ; }
pushd $MO_ROOT >/dev/null
. lib/libeval.sh
. cf/mop
popd >/dev/null

function usage
{
	die "Usage: check [-s <source-file>] <problem> [<test-number>]"
}

SRCFILE=
while getopts "s:" opt ; do
	case $opt in
		s)	SRCFILE="$OPTARG"
			;;
		*)	usage
			;;
	esac
done
shift $(($OPTIND-1))
[ -n "$1" ] || usage
PROBLEM=$1
TEST=
shift
if [ -n "$1" ] ; then
	TEST="$1"
	shift
fi
[ -z "$1" ] || usage

public-setup
. $PDIR/config

function test-verdict
{
	pend "$2"
	if [ $1 == 0 ] ; then
		exit 1
	else
		exit 0
	fi
}

if [ $TASK_TYPE == open-data ] ; then
	[ -n "$TEST" ] || die "You need to specify test number for open data problems."
	pstart "Checking $TEST: "
	test-config
	open-locate "$SRCFILE"
	try-ln "$SDIR/$SRCN" $TDIR/$TEST.out
	syntax-check
	test-result $POINTS_PER_TEST OK
else
	[ -z "$TEST" ] || die "Test number should be given only for open data problems."
	locate-source "$SRCFILE"
	compile
	RC=0
	for TEST in $SAMPLE_TESTS ; do
		(
		pstart "Checking on sample input $TEST: "
		test-config
		test-run
		syntax-check
		output-check
		) || RC=1
	done
	exit $RC
fi
