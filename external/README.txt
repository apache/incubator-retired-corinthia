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
fetch_downloads.sh script to download these. The extract_downloads.sh
script places all the files in the bin, lib, and include
directories. The result should look like this the listing below.

After building Corinthia, you will need to copy all of the DLL files from
external/bin into the bin directory within your build directory,
alongside dfconvert and dftest.

bin/SDL2.dll
bin/SDL2_image.dll
bin/iconv.dll
bin/iconv.exe
bin/libjpeg-9.dll
bin/libpng16-16.dll
bin/libtiff-5.dll
bin/libwebp-4.dll
bin/libxml2.dll
bin/xmlcatalog.exe
bin/xmllint.exe
bin/zlib1.dll
include/SDL.h
include/SDL_assert.h
include/SDL_atomic.h
include/SDL_audio.h
include/SDL_bits.h
include/SDL_blendmode.h
include/SDL_clipboard.h
include/SDL_config.h
include/SDL_cpuinfo.h
include/SDL_endian.h
include/SDL_error.h
include/SDL_events.h
include/SDL_filesystem.h
include/SDL_gamecontroller.h
include/SDL_gesture.h
include/SDL_haptic.h
include/SDL_hints.h
include/SDL_image.h
include/SDL_joystick.h
include/SDL_keyboard.h
include/SDL_keycode.h
include/SDL_loadso.h
include/SDL_log.h
include/SDL_main.h
include/SDL_messagebox.h
include/SDL_mouse.h
include/SDL_mutex.h
include/SDL_name.h
include/SDL_opengl.h
include/SDL_opengles.h
include/SDL_opengles2.h
include/SDL_pixels.h
include/SDL_platform.h
include/SDL_power.h
include/SDL_quit.h
include/SDL_rect.h
include/SDL_render.h
include/SDL_revision.h
include/SDL_rwops.h
include/SDL_scancode.h
include/SDL_shape.h
include/SDL_stdinc.h
include/SDL_surface.h
include/SDL_system.h
include/SDL_syswm.h
include/SDL_test.h
include/SDL_test_assert.h
include/SDL_test_common.h
include/SDL_test_compare.h
include/SDL_test_crc32.h
include/SDL_test_font.h
include/SDL_test_fuzzer.h
include/SDL_test_harness.h
include/SDL_test_images.h
include/SDL_test_log.h
include/SDL_test_md5.h
include/SDL_test_random.h
include/SDL_thread.h
include/SDL_timer.h
include/SDL_touch.h
include/SDL_types.h
include/SDL_version.h
include/SDL_video.h
include/begin_code.h
include/close_code.h
include/iconv.h
include/libxml/DOCBparser.h
include/libxml/HTMLparser.h
include/libxml/HTMLtree.h
include/libxml/SAX.h
include/libxml/SAX2.h
include/libxml/c14n.h
include/libxml/catalog.h
include/libxml/chvalid.h
include/libxml/debugXML.h
include/libxml/dict.h
include/libxml/encoding.h
include/libxml/entities.h
include/libxml/globals.h
include/libxml/hash.h
include/libxml/list.h
include/libxml/nanoftp.h
include/libxml/nanohttp.h
include/libxml/parser.h
include/libxml/parserInternals.h
include/libxml/pattern.h
include/libxml/relaxng.h
include/libxml/schemasInternals.h
include/libxml/schematron.h
include/libxml/threads.h
include/libxml/tree.h
include/libxml/uri.h
include/libxml/valid.h
include/libxml/xinclude.h
include/libxml/xlink.h
include/libxml/xmlIO.h
include/libxml/xmlautomata.h
include/libxml/xmlerror.h
include/libxml/xmlexports.h
include/libxml/xmlmemory.h
include/libxml/xmlmodule.h
include/libxml/xmlreader.h
include/libxml/xmlregexp.h
include/libxml/xmlsave.h
include/libxml/xmlschemas.h
include/libxml/xmlschemastypes.h
include/libxml/xmlstring.h
include/libxml/xmlunicode.h
include/libxml/xmlversion.h
include/libxml/xmlwriter.h
include/libxml/xpath.h
include/libxml/xpathInternals.h
include/libxml/xpointer.h
include/zconf.h
include/zlib.h
lib/SDL2.lib
lib/SDL2_image.lib
lib/SDL2main.lib
lib/SDL2test.lib
lib/iconv.lib
lib/iconv_a.lib
lib/libxml2.lib
lib/libxml2_a.lib
lib/libxml2_a_dll.lib
lib/zdll.lib
lib/zlib.def
