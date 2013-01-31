#!/bin/bash
# List all contestants according to $CT_USER_LIST

if [ "$1" = --help ] ; then
	echo "Usage: mo-get-users [--full]"
fi
[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

if [ "$1" = --full ] ; then
	cut -d '	' -f 1,2 <$CT_USER_LIST
else
	cut -d '	' -f 1 <$CT_USER_LIST
fi
