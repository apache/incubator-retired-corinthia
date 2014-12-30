Download external developer libraries to here, and you dont need to modify CMakeList.txt

in order to compile on windows you need:

Zlib developer files:
    http://zlib.net/zlib128-dll.zip

Iconv developer files:
    ftp://ftp.zlatkovic.com/libxml/iconv-1.9.2.win32.zip

libxml2 developer files:
    ftp://ftp.zlatkovic.com/libxml/libxml2-2.7.8.win32.zip

SDL2 developer files:
    https://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip

SDL2_image developer files:
    https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.0-VC.zip

We are working on reducing these dependencies.

On a Unix system (and possibly under cygwin), you can run the
fetch_downloads.sh script to download these.
