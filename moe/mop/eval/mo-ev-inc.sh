#!/bin/bash
# An incremental evaluator: detect which solutions were changed since
# last run and run `ev' on them.

[ -n "$1" ] || { echo "Usage: mo-ev-inc [--force] <tasks>" ; exit 1 ; }

force=0
if [ "$1" == --force ] ; then
	force=1
	shift
fi
for user in `bin/mo-get-users` ; do
	for task in "$@" ; do
		echo -n "$user/$task: "
		if [ -d solutions/$user/$task ] ; then
			N=`cd solutions/$user/$task && cat * | md5sum | head -c16` 
		else
			N=none
		fi
		if [ -f testing/$user/$task/sum ] ; then
			O=`cat testing/$user/$task/sum`
		else
			O=none
		fi
		if [ $force == 1 -a $N != none ] ; then
			O=forced
		fi
		echo -n "($O $N) "
		if [ $O == $N ] ; then
			echo OK
		elif [ $N == none ] ; then
			rm -rf testing/$user/$task
			echo DELETED
		else
			echo CHANGED
			bin/ev $user $task
			echo $N >testing/$user/$task/sum
		fi
	done
done
