#!/usr/bin/env bash

echo "Downloading moe..."
wget -O moe-arc.tar.gz "http://www.ucw.cz/gitweb/?p=moe.git;a=snapshot;h=refs/heads/master;sf=tgz"
tar -xf moe-arc.tar.gz
dname="`ls -1 | grep -e ^moe-master`"
mv $dname moe
rm moe-arc.tar.gz
echo "moe successfully downloaded."
