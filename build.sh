#!/usr/bin/env bash

echo "Building regrader..."
composer install --prefer-dist
bower install
./download-moe.sh
