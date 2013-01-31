#!/bin/bash
# Grab all submits from the submit server and copy them to solutions/.

[ -n "$1" ] || { echo "Usage: mo-grab-remote <tasks>" ; exit 1 ; }
[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

rsync -a --delete mo-submit@mo100:solutions/ submits
for user in `bin/mo-get-users` ; do
	echo -n "$user:"
	mkdir -p solutions/$user
	for t in $@ ; do
		rm -rf solutions/$user/$t
		D=submits/$user/$t
		if [ -d $D ] ; then
			echo -n " $t"
			cp -a $D solutions/$user/$t
		fi
	done
	echo
done
