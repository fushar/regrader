#!/usr/bin/env bash

echo "Building Regrader..."
composer install --prefer-dist
bower install
./download-moe.sh
cd moe
./configure
make
