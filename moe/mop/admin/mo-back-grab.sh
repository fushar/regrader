#!/bin/sh
# Find all submits in the local copy of contestants' home directories
# (as created by mo-backup) and copy them to solutions/.

if [ -z "$1" ] ; then
	echo "Directory name expected"
	exit 1
fi
rm -rf solutions/mo*
for m in `cd $1 ; echo *` ; do
	echo -n "$m:"
	for d in $1/$m/mo??/mo?? ; do
		u=`basename $d`
		if [ $u != mo00 -a -d $d/.submit ] ; then
			echo -n " $u"
			if [ -d solutions/$u ] ; then
				echo -n "<DUP!!!>"
			else
				cp -a $d/.submit solutions/$u
			fi
		fi
	done
	echo
done
