#!/bin/bash
# Grab all submits from contestants' home directories located on the
# local machine and copy them to solutions/.

[ -n "$1" ] || { echo "Usage: mo-grab <tasks>" ; exit 1 ; }
[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

for user in `bin/mo-get-users` ; do
	echo -n "$user:"
	H=`eval echo ~$user`/.submit
	if [ -d $H ] ; then
		[ -d solutions/$user ] || mkdir -p solutions/$user
		for t in $@ ; do
			rm -rf solutions/$user/$t
			if [ -d $H/$t ] ; then
				echo -n " $t"
				cp -dr $H/$t solutions/$user/$t
			fi
		done
		echo
	else
		echo " ---"
	fi
done
chown -R $EVAL_USER solutions
chmod -R a+r solutions
