#!/bin/bash
# Create home directory of the user who runs the evaluator.

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

H=`pwd`
cd $MO_ROOT/eval

echo "Creating $EVAL_USER"
rm -rf eval
mkdir eval
cd eval
cp -aL $H/* .
if [ -d ~/.ssh ] ; then echo "Copying SSH configuration from ~/.ssh" ; cp -a ~/.ssh . ; fi
cd ..
chown -R $EVAL_USER.$EVAL_GROUP eval
chmod 750 eval
