#!/bin/bash
# Copy submit certificates to contestants' machines. Each machine gets
# only the certs of the contestants who should use it.

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

while IFS="	" read LOGIN FULL MACH ; do
	if [ -z "$1" -o "$1" == "$LOGIN" ] ; then
		echo "$LOGIN -> $MACH"
		D=$MO_ROOT/users/$LOGIN/$LOGIN/
		rsync -av $D/.mo root@$MACH:$D/
	fi </dev/null
done <$CT_USER_LIST
