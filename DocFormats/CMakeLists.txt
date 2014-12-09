cmake_minimum_required(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(unittest)
add_subdirectory(filters/ooxml)
add_subdirectory(filters/latex)
add_subdirectory(filters/odf)

include_directories(/usr/include/libxml2)
include_directories(../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../DocFormats/3rdparty/external/w3c-tidy-html5/src)
include_directories(../DocFormats/3rdparty/external/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ./api/headers)
include_directories(.)
include_directories(./platform/headers)
include_directories(./core/src/common)
include_directories(./core/src/css)
include_directories(./core/src/html)
include_directories(./core/src/lib)
include_directories(./core/src/names)
include_directories(./core/src/xml)
include_directories(./filters/latex/src)
include_directories(./filters/odf/src)
include_directories(./filters/ooxml/src/common)
include_directories(./filters/ooxml/src/word)
include_directories(./filters/ooxml/src/word/formatting)
include_directories(./filters/ooxml/src/word/lenses)
include_directories(./unittest)

set(GroupRoot
    DFCommon.h
    DFTypes.h
    DocFormats.c
    $<TARGET_OBJECTS:api>
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:platform>
    $<TARGET_OBJECTS:unittest>
    $<TARGET_OBJECTS:ooxml>
    $<TARGET_OBJECTS:latex>
    $<TARGET_OBJECTS:odf>
)

add_library(DocFormats
    ${GroupRoot})

source_group(\\ FILES ${GroupRoot})
