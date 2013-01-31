# The UCW Library -- Shell Functions
# (c) 2005 Martin Mares <mj@ucw.cz>
#
# This software may be freely distributed and used according to the terms
# of the GNU Lesser General Public License.

UCW_CF=
while [ "${1:0:2}" = "-C" -o "${1:0:2}" = "-S" ] ; do
	if [ -z "${1:2:1}" ] ; then
		UCW_CF="$UCW_CF $1 $2"
		shift 2
	else
		UCW_CF="$UCW_CF $1"
		shift 1
	fi
done

function log # msg
{
	bin/logger $UCW_PROGNAME I "$1"
}

function errlog # msg
{
	bin/logger $UCW_PROGNAME E "$1"
}

function warnlog # msg
{
	bin/logger $UCW_PROGNAME E "$1"
}

function die # msg
{
	bin/logger $UCW_PROGNAME ! "$1"
	exit 1
}

function parse-config # section vars...
{
	eval `bin/config$UCW_CF "$@"`
}
