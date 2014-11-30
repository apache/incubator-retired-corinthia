cmake_minimum_required(VERSION 2.8)

set(SOURCES
    BDTTest.c
    Commands.c
    DFChanges.c
    Plain.c
    Test.c
    TestCase.c
    TestFunctions.c
    TextPackage.c
    main.c
)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../../DocFormats/api/headers)
include_directories(.)
include_directories(../../../DocFormats)
include_directories(../../../DocFormats/platform/headers)
include_directories(../../../DocFormats/core/src/common)
include_directories(../../../DocFormats/core/src/css)
include_directories(../../../DocFormats/core/src/html)
include_directories(../../../DocFormats/core/src/lib)
include_directories(../../../DocFormats/core/src/names)
include_directories(../../../DocFormats/filters/latex/src)
include_directories(../../../DocFormats/filters/odf/src)
include_directories(../../../DocFormats/filters/ooxml/src/common)
include_directories(../../../DocFormats/filters/ooxml/src/word)
include_directories(../../../DocFormats/filters/ooxml/src/word/formatting)
include_directories(../../../DocFormats/filters/ooxml/src/word/lenses)
include_directories(../../../DocFormats/core/src/xml)
include_directories(/usr/include/libxml2)
link_directories(${LIB_DIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SOURCES ${SOURCES} FunctionTests.m StringTests.m)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_executable(dfutil ${SOURCES})
target_link_libraries(dfutil DocFormats ${LIBS})
