#!/bin/bash

[ -f cf/mop ] || { echo "Missing config file, check cwd." ; exit 1 ; }
set -e
. cf/mop

H=`pwd`

# The eval directory
cd $MO_ROOT
rm -rf eval
mkdir eval
chgrp $EVAL_GROUP eval
chmod 755 eval
cd eval

# mo-eval home
( cd $H && bin/mo-create-eval )

# testusers
( cd eval && bin/mo-create-testusers )

# mo-submit home
if [ -n "$REMOTE_SUBMIT" ] ; then
	mkdir submit
	chmod 750 submit
	if [ -d ~/.ssh ] ; then echo "Copying SSH configuration from ~/.ssh" ; cp -a ~/.ssh submit/ ; fi
	( cd $H && bin/mo-create-submit )
fi

# create public
cd $MO_ROOT
echo "Creating public"
rm -rf public
mkdir public

# populate public
( cd eval/eval ; bin/mo-create-public )
