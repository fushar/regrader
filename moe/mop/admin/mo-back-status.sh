#!/bin/sh
# Find all submits in the local copy of contestants' home directories
# (as created by mo-backup) and print their status.

if [ -z "$1" ] ; then
	echo "Directory name expected"
	exit 1
fi
for m in `cd $1 ; echo *` ; do
	echo -n "$m:"
	for d in $1/$m/mo??/mo?? ; do
		u=`basename $d`
		if [ $u != mo00 -a `ls $d | wc -l` -gt 0 ] ; then
			echo -n " $u"
			if [ -d $d/.submit ] ; then
				echo -n '('
				( cd $d/.submit ; echo -n * )
				echo -n ')'
			fi
		fi
	done
	echo
done
