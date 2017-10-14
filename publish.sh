#!/usr/bin/env bash

echo "Publishing Regrader..."
rm -rf dist *.tar.gz
mkdir dist
cp -R application examples index.php moe LICENSE README.md run_grader.sh .env.example dist/

# assets
mkdir -p dist/assets
cp -R assets/css assets/img assets/index.html assets/js dist/assets/

# codeigniter
APP=vendor/codeigniter/framework
mkdir -p dist/$APP
cp -R $APP/system $APP/license.txt dist/$APP/
cp -R $APP/system/language/english $APP/system/language/indonesian

# vlucas/phpdotenv
APP=vendor/vlucas/phpdotenv
mkdir -p dist/$APP
cp -R $APP/src $APP/LICENSE.txt dist/$APP/
cp .env.example dist/
cp -R vendor/autoload.php vendor/composer dist/vendor/

# ama3-anytime
APP=assets/vendor/ama3-anytime
mkdir -p dist/$APP
cp -R $APP/anytime.js $APP/jquery-migrate-1.0.0.js dist/$APP
mkdir -p dist/$APP/styles
cp -R $APP/styles/anytime.compressed.css dist/$APP/styles/

# bootstrap
APP=assets/vendor/bootstrap
mkdir -p dist/$APP
cp -R $APP/LICENSE dist/$APP/
mkdir -p dist/$APP/dist
cp -R $APP/dist/fonts dist/$APP/dist
mkdir -p dist/$APP/dist/css
cp -R $APP/dist/css/bootstrap.min.css dist/$APP/dist/css/
mkdir -p dist/$APP/dist/js
cp -R $APP/dist/js/bootstrap.min.js dist/$APP/dist/js/

# google-code-prettify
APP=assets/vendor/google-code-prettify
mkdir -p dist/$APP
cp -R $APP/COPYING dist/$APP/
mkdir -p dist/$APP/bin
cp -R $APP/bin/prettify.min.css $APP/bin/prettify.min.js dist/$APP/bin/

# ckeditor
APP=assets/vendor/ckeditor
mkdir -p dist/$APP
cp -R $APP/LICENSE.md $APP/plugins $APP/skins $APP/lang $APP/adapters $APP/ckeditor.js $APP/config.js $APP/contents.css $APP/styles.js dist/$APP/

# MathJax
APP=assets/vendor/MathJax
mkdir -p dist/$APP
cp -R $APP/LICENSE $APP/config $APP/extensions $APP/fonts $APP/jax $APP/localization $APP/MathJax.js $APP/latest.js dist/$APP/

# jquery
APP=assets/vendor/jquery
mkdir -p dist/$APP
cp -R $APP/MIT-LICENSE.txt dist/$APP/
mkdir -p dist/$APP/dist
cp -R $APP/dist/jquery.min.js dist/$APP/dist/

pushd dist > /dev/null
tar -zcf ../regrader.tar.gz .
popd > /dev/null
rm -r dist
echo "Regrader has been successfully published as regrader.tar.gz"
