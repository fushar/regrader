# The Evaluator -- Public Compilation Script
# (c) 2001 Martin Mares <mj@ucw.cz>

set -e
[ -n "$MO_ROOT" -a -d "$MO_ROOT" ] || { echo >&2 "MO_ROOT not set, giving up." ; exit 1 ; }
pushd $MO_ROOT >/dev/null
. lib/libeval.sh
. cf/eval
. cf/mop
popd >/dev/null

[ -n "$1" ] || die "Usage: compile (<problem> | <file> [<options>])"
if [ "${1%%.*}" == "$1" ] ; then
	# Compiling problem
	PROBLEM=$1
	public-setup
	. $PDIR/config
	locate-source
	if compile ; then
		mv $TDIR/$PROBLEM .
	else
		echo >&2
		sed <check-log >&2 '1,/^Compiler output:/d;/^Compiler output files:/,$d;/^Exited /d'
	fi
else
	SRC=$1
	[ -f $SRC ] || die "$SRC doesn't exist"
	EXE=${1%%.*}
	SRCEXT=${1/*./}
	shift
	EXTRA_CFLAGS="$@"
	CCMD=EXT_${SRCEXT}_COMP
	[ -n "${!CCMD}" ] || die "Don't know how to compile $SRC"
	CCMD="`eval echo ${!CCMD}`"
	echo "$CCMD"
	$CCMD
fi
