include_directories(/usr/include/libxml2)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/src)
include_directories(../../DocFormats/3rdparty/external/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../api/headers)
include_directories(..)
include_directories(../platform/headers)
include_directories(../core/src/common)
include_directories(../core/src/css)
include_directories(../core/src/html)
include_directories(../core/src/lib)
include_directories(../core/src/names)
include_directories(../core/src/xml)
include_directories(../filters/latex/src)
include_directories(../filters/odf/src)
include_directories(../filters/ooxml/src/common)
include_directories(../filters/ooxml/src/word)
include_directories(../filters/ooxml/src/word/formatting)
include_directories(../filters/ooxml/src/word/lenses)
include_directories(../unittest)

set(GroupHeaders
    headers/DFPlatform.h)

set(GroupSrc
    src/Apple.c
    src/Linux.c
    src/Unix.c
    src/Win32.c)

set(GroupTests
    tests/PlatformTests.c)

add_library(platform OBJECT
    ${GroupHeaders}
    ${GroupSrc}
    ${GroupTests})

source_group(headers FILES ${GroupHeaders})
source_group(src FILES ${GroupSrc})
source_group(tests FILES ${GroupTests})
