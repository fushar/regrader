# The Evaluator -- Shell Function Library
# (c) 2001--2008 Martin Mares <mj@ucw.cz>

# General settings
shopt -s dotglob

# Logging functions.
# File handles used: fd1=log, fd2=progress

function log-init
{
	exec >>$TDIR/log
	HAVE_LOG=1
}

function pstart
{
	echo >&2 -n "$@"
}

function pcont
{
	echo >&2 -n "$@"
}

function pend
{
	echo >&2 "$@"
}

function die
{
	# Report an internal error
	echo >&2 "$@"
	[ -n "$HAVE_LOG" ] && echo "Fatal error: $@"
	exit 2
}

function fatal
{
	# Report a fatal error in the program being tested
	echo >&2 "$@"
	[ -n "$HAVE_LOG" ] && echo "Fatal error: $@"
	exit 1
}

function try-ln
{
	ln $1 $2 2>/dev/null || cp $1 $2
}

# Expand occurrences of `$var' in a given variable

function expand-var
{
	eval echo \"${!1}\"
}

# Given a <prefix>, override each variable <x> by <prefix>_<x>

function override-vars
{
	local OR V W
	declare -a OR
	# `${!${1}_@}' does not work, so we have to use eval
	OR=($(eval echo '${!'$1'_@}'))
	for V in "${OR[@]}" ; do
		W=${V##$1_}
		eval $W='"$'$V'"'
	done
}

# Sandbox subroutines

function box-init
{
	pstart "Preparing sandbox... "

	# Default values for user/group
	EVAL_USER=${EVAL_USER:-$USER}
	EVAL_GROUP=${EVAL_GROUP:-$GROUP}
	TEST_USER=${TEST_USER:-$EVAL_USER}

	if [ -z "$TEST_USER" -o "$TEST_USER" == "$EVAL_USER" ] ; then
		pcont "running locally (INSECURE), "
		TEST_USER="$EVAL_USER"
		BOXDIR=$HDIR/box
		BOXCMD=$HDIR/bin/box
		mkdir -p $BOXDIR
	else
		pcont "used account $TEST_USER, "
		BOXDIR=$HDIR/box
		BOXCMD=$HDIR/bin/box-$TEST_USER
	fi
	[ -d $BOXDIR -a -f $BOXCMD ] || die "Sandbox set up incorrectly"
	BOXCMD="$BOXCMD -c$BOXDIR"
	echo "Sandbox directory: $BOXDIR"
	echo "Sandbox command: $BOXCMD"
	box-clean
	pend "OK"
}

function box-clean
{
	[ -n "$BOXCMD" ] || die "box-init not called"
	rm -rf $BOXDIR/*
}

# Initialization of testing directories

function dir-init
{
	pstart "Initializing... "
	[ -z "$HDIR" ] && HDIR=.
	PDIR=$HDIR/problems/$PROBLEM
	SDIR=$HDIR/solutions/$CONTESTANT/$PROBLEM
	TDIR=$HDIR/testing/$CONTESTANT/$PROBLEM
	TMPDIR=$HDIR/tmp/

	[ -d $PDIR ] || die "Problem $PROBLEM not known"
	[ -d $SDIR ] || fatal "Solution of $PROBLEM not found"
	mkdir -p $TDIR $TMPDIR
	rm -rf $TDIR $TMPDIR
	mkdir -p $TDIR $TMPDIR
	cat >$TDIR/log <<EOF
Testing solution of $PROBLEM by $CONTESTANT
Test started at `date`
Eval base directory: $HDIR
Contestant's solution directory: $SDIR
Problem directory: $PDIR
Testing directory: $TDIR
EOF
	pend "OK"
}

# Locate source file.
# If no parameter is given, locate it in SDIR and return name as SRCN and extension as SRCEXT
# Or a file name can be given and then SDIR, SRCN and SRCEXT are set.
# Beware, SDIR and SRCN can contain spaces and other strange user-supplied characters.

function locate-source
{
	pstart "Finding source... "
	local SBASE
	if [ -n "$1" ] ; then
		SDIR=`dirname "$1"`
		local S=`basename "$1"`
		SBASE=$(echo "$S" | sed 's/\.\([^.]\+\)//')
		SRCEXT=$(echo "$S" | sed '/\./!d; s/.*\.\([^.]\+\)/\1/')
		if [ -n "$SRCEXT" ] ; then
			# Full name given, so just check the extension and existence
			SRCN="$S"
			[ -f "$SDIR/$SRCN" ] || die "Cannot find source file $SDIR/$SRCN"
			SRCEXT_OK=
			for a in $EXTENSIONS ; do
				if [ $a == $SRCEXT ] ; then
					pend $SDIR/$SRCN
					echo "Explicitly set source file: $SDIR/$SRCN"
					return 0
				fi
			done
			die "Unknown extension .$SRCEXT"
		fi
	else
		SBASE=$PROBLEM
	fi
	for a in $EXTENSIONS ; do
		if [ -f "$SDIR/$SBASE.$a" ] ; then
			[ -z "$SRCN" ] || die "Multiple source files found: $SDIR/$PROBLEM.$a and $SDIR/$SRCN. Please fix."
			SRCN="$SBASE.$a"
			SRCEXT=$a
		fi
	done
	[ -n "$SRCN" ] || fatal "NOT FOUND"
	pend $SRCN
	echo "Found source file: $SDIR/$SRCN"
}

# Compilation (compile SDIR/SRCN with PDIR/COMP_EXTRAS to EXE=TDIR/PROBLEM)

function compile
{
	pstart "Compiling... "

	a="ALIAS_EXT_$SRCEXT"
	if [ -n "${!a}" ] ; then
		SRCEXT="${!a}"
		echo "Normalized file extension: $SRCEXT"
	fi
	override-vars "EXT_$SRCEXT"

	# Beware, the original SRCN can be a strange user-supplied name
	SRC=$PROBLEM.$SRCEXT
	cp "$SDIR/$SRCN" $TDIR/$SRC
	if [ -n "$COMP_EXTRAS" ] ; then
		echo "Extras: $COMP_EXTRAS"
		for a in $COMP_EXTRAS ; do cp $PDIR/$a $TDIR/ ; done
	fi

	box-clean
	for a in $SRC $COMP_EXTRAS ; do cp $TDIR/$a $BOXDIR/ ; done
	EXE=$PROBLEM
	CCMD=$(expand-var COMP)
	COMP_SANDBOX_OPTS=$(expand-var COMP_SANDBOX_OPTS)
	echo "Compiler command: $CCMD"
	echo "Compiler sandbox options: $COMP_SANDBOX_OPTS"
	if [ -n "$PRE_COMPILE_HOOK" ] ; then
		echo "Pre-compile hook: $PRE_COMPILE_HOOK"
		eval $PRE_COMPILE_HOOK
	fi

	echo "Compiler input files:"
	ls -Al $BOXDIR
	echo "Compiler output:"
	if ! $BOXCMD $COMP_SANDBOX_OPTS -- $CCMD 2>$TDIR/compile.out ; then
		COMPILE_MSG="`cat $TDIR/compile.out`"
		pend "FAILED: $COMPILE_MSG"
		echo "$COMPILE_MSG"
		return 1
	fi
	cat $TDIR/compile.out
	rm $TDIR/compile.out
	if [ -n "$POST_COMPILE_HOOK" ] ; then
		echo "Post-compile hook: $POST_COMPILE_HOOK"
		eval $POST_COMPILE_HOOK
	fi
	echo "Compiler output files:"
	ls -Al $BOXDIR
	if [ ! -f $BOXDIR/$PROBLEM ] ; then
		pend "FAILED: Missing executable file"
		echo "Missing executable file"
		return 1
	fi
	EXE=$TDIR/$PROBLEM
	cp -a $BOXDIR/$PROBLEM $EXE
	echo "Compiled OK, result copied to $EXE"
	pend "OK"
}

# Running of test program according to current task type (returns exit code and TEST_MSG)

function test-config
{
	[ -f $PDIR/$TEST.config ] && . $PDIR/$TEST.config
	override-vars "TEST_$TEST"
}

function test-run
{
	test-run-$TASK_TYPE
}

function test-result
{
	P=$1
	M=$2
	if [ -s $TDIR/$TEST.pts ] ; then
		P=`cat $TDIR/$TEST.pts`
		rm $TDIR/$TEST.pts
	fi

	# Translate signal numbers to readable strings
	SG=${M#Caught fatal signal }
	SG=${SG#Committed suicide by signal }
	if [ "$SG" != "$M" ] ; then
		SG=`kill -l $SG 2>/dev/null` || SG=
		[ -z "$SG" ] || M="$M (SIG$SG)"
	fi

	# Translate runtime errors to readable strings
	RE=${M#Exited with error status }
	if [ -n "$EXIT_CODE_HOOK" -a "$RE" != "$M" ] ; then
		NEWMSG=`$EXIT_CODE_HOOK $RE` || NEWMSG=
		if [ -n "$NEWMSG" ] ; then
			M="Runtime error $RE: $NEWMSG"
		fi
	fi

	echo "Verdict: $M"
	echo "Points: $P"
	test-verdict $P "$M"
}

function test-prolog
{
	pcont "<init> "
	box-clean
	echo "Executable file: $TDIR/$PROBLEM"
	if [ ! -x $TDIR/$PROBLEM ] ; then
		test-result 0 "Compile error"
	fi
	cp $TDIR/$PROBLEM $BOXDIR/
	IN_TYPE=${IN_TYPE:-$IO_TYPE}
	OUT_TYPE=${OUT_TYPE:-$IO_TYPE}
	case $IN_TYPE in
		file)	echo "Input file: $PROBLEM.in (from $PDIR/$TEST.in)"
			try-ln $PDIR/$TEST.in $TDIR/$TEST.in
			cp $PDIR/$TEST.in $BOXDIR/$PROBLEM.in
			[ $TASK_TYPE == interactive ] || BOX_EXTRAS="$BOX_EXTRAS -i/dev/null"
			;;
		stdio)	echo "Input file: <stdin> (from $PDIR/$TEST.in)"
			try-ln $PDIR/$TEST.in $TDIR/$TEST.in
			cp $PDIR/$TEST.in $BOXDIR/.stdin
			BOX_EXTRAS="$BOX_EXTRAS -i.stdin"
			;;
		none)	echo "Input file: <none>"
			;;
		dir)	echo "Input file: files in directory $PDIR/$TEST.in/"
			[ -d $PDIR/$TEST.in ] || die "Not a directory: $PDIR/$TEST.in"
			# TODO: recursive ln to $TDIR
			cp -r $PDIR/$TEST.in $TDIR/$TEST.in
			cp -r $PDIR/$TEST.in/* $BOXDIR/
			# Can have .stdin, but empty by default
			touch $BOXDIR/.stdin
			BOX_EXTRAS="$BOX_EXTRAS -i.stdin"
			;;
		*)	die "Unknown IN_TYPE $IN_TYPE"
			;;
	esac
	if [ -n "$EV_PEDANT" -a $IN_TYPE != none ] ; then
		pcont "<pedant> "
		if [ "$EV_PEDANT" = 1 ] ; then
			EV_PEDANT=" "
		fi
		bin/pedant <$TDIR/$TEST.in >$TDIR/$TEST.pedant $EV_PEDANT
		if [ -s $TDIR/$TEST.pedant ] ; then
			pend
			sed 's/^/\t/' <$TDIR/$TEST.pedant >&2
			pstart -e '\t'
		fi
	fi
	case $OUT_TYPE in
		file)	echo "Output file: $PROBLEM.out"
			[ $TASK_TYPE == interactive ] || BOX_EXTRAS="$BOX_EXTRAS -o/dev/null"
			;;
		stdio)	echo "Output file: <stdout>"
			BOX_EXTRAS="$BOX_EXTRAS -o.stdout"
			;;
		none)	echo "Output file: <none>"
			;;
		*)	die "Unknown OUT_TYPE $OUT_TYPE"
			;;
	esac
	echo "Timeout: $TIME_LIMIT s"
	echo "Memory: $MEM_LIMIT KB"
	if [ -n "$PRE_RUN_HOOK" ] ; then
		echo "Pre-run hook: $PRE_RUN_HOOK"
		eval $PRE_RUN_HOOK
	fi
	echo "Sandbox contents before start:"
	ls -Al $BOXDIR
}

function test-epilog
{
	if [ -n "$POST_RUN_HOOK" ] ; then
		echo "Post-run hook: $POST_RUN_HOOK"
		eval $POST_RUN_HOOK
	fi
	echo "Sandbox contents after exit:"
	ls -Al $BOXDIR
	case ${OUT_TYPE:-$IO_TYPE} in
		file)	[ -f $BOXDIR/$PROBLEM.out ] || test-result 0 "No output file"
			cp $BOXDIR/$PROBLEM.out $TDIR/$TEST.out
			;;
		stdio)	[ -f $BOXDIR/.stdout ] || test-result 0 "No output file"
			cp $BOXDIR/.stdout $TDIR/$TEST.out
			;;
	esac

	if [ -n "$OUTPUT_FILTER" -a "$OUT_TYPE" != none -a -z "$EV_NOFILTER" ] ; then
		pcont "<filter> "
		FILTER=$(expand-var OUTPUT_FILTER)
		echo "Output filter command: $FILTER"
		mv $TDIR/$TEST.out $TDIR/$TEST.raw
		if ! eval $FILTER 2>$TMPDIR/exec.out ; then
			cat $TMPDIR/exec.out
			MSG=`tail -1 $TMPDIR/exec.out`
			if [ -z "$MSG" ] ; then MSG="Filter failed" ; fi
			test-result 0 "$MSG"
		fi
		cat $TMPDIR/exec.out
	fi
}

# Running of test program with file input/output

function test-run-file
{
	test-prolog
	pcont "<run> "
	BOXOPTS=$(expand-var TEST_SANDBOX_OPTS)
	echo "Sandbox options: $BOXOPTS"
	EXECMD=$(expand-var TEST_EXEC_CMD)
	[ -z "$EXECMD" ] || echo "Exec command: $EXECMD" 
	[ -z "$EXECMD" ] && EXECMD="./$PROBLEM" 
	if ! $BOXCMD $BOXOPTS -- $EXECMD 2>$TMPDIR/exec.out ; then
		cat $TMPDIR/exec.out
		MSG=`tail -1 $TMPDIR/exec.out`
		test-result 0 "$MSG"
	fi
	cat $TMPDIR/exec.out
	test-epilog
}

# Running of interactive test programs

function test-run-interactive
{
	test-prolog
	pcont "<run> "
	BOXOPTS=$(expand-var TEST_SANDBOX_OPTS)
	echo "Sandbox options: $BOXOPTS"
	ICCMD=$(expand-var IA_CHECK)
	echo "Interactive checker: $ICCMD"
	EXECMD=$(expand-var TEST_EXEC_CMD)
	[ -z "$EXECMD" ] || echo "Exec command: $EXECMD" 
	[ -z "$EXECMD" ] && EXECMD="./$PROBLEM" 
	if ! $HDIR/bin/iwrapper $BOXCMD $BOXOPTS -- $EXECMD @@ $ICCMD 2>$TMPDIR/exec.out ; then
		cat $TMPDIR/exec.out
		MSG="`head -1 $TMPDIR/exec.out`"
		test-result 0 "$MSG"
	fi
	cat $TMPDIR/exec.out
	test-epilog
}

# "Running" of open-data problems

function test-run-open-data
{
	[ -f $SDIR/$TEST.out ] || test-result 0 "No solution"
	try-ln $SDIR/$TEST.out $TDIR/$TEST.out
}

# Syntax checks

function syntax-check
{
	[ -n "$SYNTAX_CHECK" ] || return 0
	[ -z "$EV_NOCHECK" ] || return 0
	pcont "<syntax> "
	SCHECK=$(expand-var SYNTAX_CHECK)
	echo "Syntax check command: $SCHECK"
	if ! eval $SCHECK 2>$TMPDIR/exec.out ; then
		cat $TMPDIR/exec.out
		MSG=`tail -1 $TMPDIR/exec.out`
		if [ -z "$MSG" ] ; then MSG="Wrong syntax" ; fi
		test-result 0 "$MSG"
	fi
	cat $TMPDIR/exec.out
}

# Output checks

function output-check
{
	MSG=
	if [ -n "$OUTPUT_CHECK" -a "$OUT_TYPE" != none -a -z "$EV_NOCHECK" ] ; then
		pcont "<check> "
		[ -f $PDIR/$TEST.out ] && try-ln $PDIR/$TEST.out $TDIR/$TEST.ok
		OCHECK=$(expand-var OUTPUT_CHECK)
		echo "Output check command: $OCHECK"
		if ! eval $OCHECK 2>$TMPDIR/exec.out ; then
			cat $TMPDIR/exec.out
			MSG=`tail -1 $TMPDIR/exec.out`
			if [ -z "$MSG" ] ; then MSG="Wrong answer" ; fi
			test-result 0 "$MSG"
		fi
		cat $TMPDIR/exec.out
		MSG=`tail -1 $TMPDIR/exec.out`
	fi
	if [ -z "$MSG" ] ; then MSG="OK" ; fi
	test-result $POINTS_PER_TEST "$MSG"
}

# Setup of public commands

function public-setup
{
	HDIR=$MO_ROOT
	PDIR=$MO_ROOT/problems/$PROBLEM
	SDIR=.
	TDIR=~/.test
	TMPDIR=~/.test
	[ -d $PDIR ] || die "Unknown problem $PROBLEM"

	pstart "Initializing... "
	mkdir -p $TDIR
	rm -rf $TDIR/*
	BOXDIR=~/.box
	mkdir -p $BOXDIR
	rm -rf $BOXDIR/*
	BOXCMD="$MO_ROOT/bin/box -c$BOXDIR"
	exec >check-log
	pend "OK  (see 'check-log' for details)"
}

# Locate output of open data problem, test case TEST
# Beware, SDIR and SRCN can contain spaces and other strange user-supplied characters.

function open-locate
{
	[ -f $PDIR/$TEST.in ] || die "Unknown test $TEST"
	if [ -n "$1" ] ; then
		SDIR=`dirname "$1"`
		SRCN=`basename "$1"`
	else
		SRCN=$SDIR/$PROBLEM$TEST.out
	fi
	[ -f "$SDIR/$SRCN" ] || fatal "Output file $SRCN not found"
}

# Translation of runtime error codes for various compilers

function fpc-exit-code
{
	case "$1" in
		200)	echo -n "Division by zero" ;;
		201)	echo -n "Range check error" ;;
		202)	echo -n "Stack overflow" ;;
		203)	echo -n "Heap overflow" ;;
		205)	echo -n "Floating point overflow" ;;
		215)	echo -n "Arithmetic overflow" ;;
		216)	echo -n "Segmentation fault" ;;
	esac
}

# A helper function for parsing of command-line overrides of variables

function parse-cmdline
{
	if [ "${1#*=}" != "$1" ] ; then
		local var=${1%%=*}
		local val=${1#*=}
		eval $var="'$val'"
		return 0
	else
		return 1
	fi
}
