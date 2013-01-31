#!/bin/bash
# Create a directory with the public scripts from mop/public/.

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

echo "Populating $MO_ROOT/public"
H=`pwd`
cd $MO_ROOT/public
rm -rf bin lib cf

mkdir cf
for a in eval mop ; do
	sed '/^\(TEST_USER\|MO_ROOT\)=/s/^/#/' <$H/cf/$a >cf/$a
done

mkdir bin
cp -aL $H/bin/{check,submit,compile,status,box,iwrapper} bin/

mkdir lib
cp -aL $H/lib/libeval.sh lib/

if [ -n "$REMOTE_SUBMIT" ] ; then
	cp -aL $H/bin/{contest,remote-submit,remote-status} bin/
	cp -aL $H/lib/perl5 lib/
fi

mkdir -p problems

if [ `id -u` == 0 ] ; then
	chown -R $EVAL_USER.$EVAL_GROUP .
	chmod 755 .
fi
