Download external developer libraries to here, and you dont need to modify CMakeList.txt

in order to compile on windows you need:

Zlib developer files from http://gnuwin32.sourceforge.net/packages/zlib.htm
Iconv developer files from http://gnuwin32.sourceforge.net/packages/libiconv.htm
libxml2 developer files from ftp://ftp.zlatkovic.com/libxml/64bit/
SDL2 developer files from https://www.libsdl.org/download-2.0.php
SDL2_image developer files from https://www.libsdl.org/projects/SDL_image/

you might also need:
unistd.h from http://sourceforge.net/p/mingw/mingw-org-wsl/ci/master/tree/include/


We are working on reducing these dependencies.
