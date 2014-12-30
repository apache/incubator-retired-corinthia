#!/bin/bash

set -e
if [ ! -d download ]; then
    mkdir download
fi

URLS="http://zlib.net/zlib128-dll.zip \
      ftp://ftp.zlatkovic.com/libxml/iconv-1.9.2.win32.zip \
      ftp://ftp.zlatkovic.com/libxml/libxml2-2.7.8.win32.zip \
      https://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip \
      https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.0-VC.zip"

cd download
for url in $URLS; do
    if [ ! -f $(basename $url) ]; then
        wget $url
    fi
done

cd ..
