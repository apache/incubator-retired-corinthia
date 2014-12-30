#!/bin/bash

set -e

mkdir packages

mkdir packages/SDL2
mkdir packages/SDL2_image
mkdir packages/iconv
mkdir packages/libxml2
mkdir packages/zlib

(cd packages/SDL2 && unzip ../../download/SDL2-devel-2.0.3-VC.zip)
(cd packages/SDL2_image && unzip ../../download/SDL2_image-devel-2.0.0-VC.zip)
(cd packages/iconv && unzip ../../download/iconv-1.9.2.win32.zip)
(cd packages/libxml2 && unzip ../../download/libxml2-2.7.8.win32.zip)
(cd packages/zlib && unzip ../../download/zlib128-dll.zip)
chmod -R u+w packages/zlib

for i in bin lib include; do
    if [ ! -d $i ]; then
        mkdir $i
    fi
done

mv packages/SDL2/SDL2-2.0.3/include/* include
mv packages/SDL2/SDL2-2.0.3/lib/x86/*.lib lib
mv packages/SDL2/SDL2-2.0.3/lib/x86/*.dll bin

mv packages/SDL2_image/SDL2_image-2.0.0/include/* include
mv packages/SDL2_image/SDL2_image-2.0.0/lib/x86/*.lib lib
mv packages/SDL2_image/SDL2_image-2.0.0/lib/x86/*.dll bin

mv packages/iconv/iconv-1.9.2.win32/include/* include
mv packages/iconv/iconv-1.9.2.win32/lib/* lib
mv packages/iconv/iconv-1.9.2.win32/bin/* bin

mv packages/libxml2/libxml2-2.7.8.win32/include/* include
mv packages/libxml2/libxml2-2.7.8.win32/lib/* lib
mv packages/libxml2/libxml2-2.7.8.win32/bin/* bin

mv packages/zlib/include/* include
mv packages/zlib/lib/* lib
mv packages/zlib/*.dll bin

rm -rf packages
