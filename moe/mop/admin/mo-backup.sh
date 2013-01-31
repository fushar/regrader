#!/bin/sh
# A trivial script to back up contestants' home directories.

if [ -z "$1" ] ; then
	D=back/`date '+%H%M'`
else
	D=$1
fi
mkdir -p $D

for m in 13 14 15 23 24 25 {3,4,5,6,7,8}{1,2,3,4,5} ; do
	m="mo$m"
	echo -n "$m: "
	mkdir -p $D/$m
	pushd $D/$m >/dev/null
	ssh root@$m 'cd /mo/users ; tar czf - . --exclude=.kde' | tar xzf -
	popd >/dev/null
	du -s $D/$m | cut -f 1
done

echo -n "submit: "
rsync -a mo-submit@mo21: $D/submit/
du -s $D/submit | cut -f 1
