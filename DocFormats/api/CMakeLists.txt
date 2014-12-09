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

set(GroupAPI
    headers/DocFormats/DFError.h
    headers/DocFormats/DFStorage.h
    headers/DocFormats/DFXMLForward.h
    headers/DocFormats/DocFormats.h
    headers/DocFormats/Formats.h
    headers/DocFormats/Operations.h
    src/Formats.c
    src/Operations.c
    tests/APITests.c)

add_library(api OBJECT
    ${GroupAPI})

source_group(\\ FILES ${GroupAPI})
