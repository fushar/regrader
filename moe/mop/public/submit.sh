#!/bin/bash
# The Evaluator -- Public Submit Script
# (c) 2001--2007 Martin Mares <mj@ucw.cz>

set -e
[ -n "$MO_ROOT" -a -d "$MO_ROOT" ] || { echo >&2 "MO_ROOT not set, giving up." ; exit 1 ; }
pushd $MO_ROOT >/dev/null
. lib/libeval.sh
. cf/mop
popd >/dev/null

function usage
{
	die "Usage: submit [--force] [-s <source-file>] <problem> [<test-number>]"
}

FORCE=0
if [ "$1" = --force ] ; then
	FORCE=1
	shift
fi
[ -n "$1" -a "$1" != "--help" ] || usage
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
PART=
shift
if [ -n "$1" ] ; then
	PART="$1"
	shift
fi
[ -z "$1" ] || usage
public-setup
. $PDIR/config

function test-verdict
{
	pend "$2"
	[ $1 == 0 ] && exit 1 || exit 0
}

FAILED=0
if [ $TASK_TYPE == open-data ] ; then
	[ -n "$PART" ] || die "You need to specify test number for open data problems."
	TEST=$PART
	pstart "Test case $TEST: "
	open-locate "$SRCFILE"
	(
		[ -f $PDIR/$TEST.config ] && . $PDIR/$TEST.config
		try-ln "$SDIR/$SRCN" $TDIR/$TEST.out
		syntax-check
		test-result $POINTS_PER_TEST OK
	) || FAILED=1
else
	[ -z "$PART" ] || die "Test number should be given only for open data problems."
	locate-source "$SRCFILE"
	compile
	for TEST in $SAMPLE_TESTS ; do
		(
		pstart "Checking on sample input $TEST: "
		[ -f $PDIR/$TEST.config ] && . $PDIR/$TEST.config
		test-run
		syntax-check
		output-check
		die "How could I get there? It's a buuuuug!"
		) || FAILED=$(($FAILED+1))
	done
fi

if [ $FAILED != 0 ] ; then
	if [ $FORCE != 0 ] ; then
		echo "Submit forced."
		pend "TESTS FAILED, but --force given, so submitting anyway."
	else
		pend "TESTS FAILED. Nothing has been submitted!"
		pend "Use submit --force if you really want to submit an obviously WRONG solution."
		exit 1
	fi
fi

if [ -n "$REMOTE_SUBMIT" ] ; then
	pstart "Submitting to the server... "
	$MO_ROOT/bin/remote-submit $PROBLEM $PART "$SDIR/$SRCN"
	pend "OK"
	exit 0
fi

pstart "Submitting... "
mkdir -p ~/.submit
if [ $TASK_TYPE == open-data ] ; then
	mkdir -p ~/.submit/$PROBLEM
	cp "$SDIR/$SRCN" ~/.submit/$PROBLEM/$PART.out
else
	rm -rf ~/.submit/$PROBLEM
	mkdir -p ~/.submit/$PROBLEM
	cp "$SDIR/$SRCN" ~/.submit/$PROBLEM/
fi
pend "OK"
