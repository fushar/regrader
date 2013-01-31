#!/bin/bash
# Create home directory of the submit server.

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop
[ -n "$REMOTE_SUBMIT" ] || { echo "Remote submit not enabled." ; exit 1 ; }

echo "Creating submit directory"

H=`pwd`
cd $MO_ROOT/eval/submit
mkdir -p certs
cp $H/certs/server* certs/
cp $H/certs/ca-cert.pem certs/

rm -rf bin cf lib
mkdir bin cf
cp $H/bin/{submitd,show-submits} bin/
cp $H/cf/{submitd,libucw} cf/
cp -aL $H/lib .

mkdir -p solutions 
for a in `cd $H && bin/mo-get-users` ; do
	mkdir -p solutions/$a
done

rm -rf tmp
mkdir -p tmp

mkdir -p log history

chown -R $REMOTE_SUBMIT_USER.$REMOTE_SUBMIT_GROUP $MO_ROOT/eval/submit
