#!/usr/bin/env bash

echo "Downloading moe..."
wget -O moe-5a83cb2.tar.gz "http://www.ucw.cz/gitweb/?p=moe.git;a=snapshot;h=5a83cb289dacc0e3c93203138867988887d0c2a9;sf=tgz"
tar -xf moe-5a83cb2.tar.gz
mv moe-5a83cb2 moe
rm moe-5a83cb2.tar.gz
echo "moe successfully downloaded."
