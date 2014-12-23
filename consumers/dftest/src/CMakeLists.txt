cmake_minimum_required(VERSION 2.8)

set(SOURCES
    main.c)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(.)
include_directories(SYSTEM ../../../DocFormats/api/headers)
include_directories(../../../DocFormats/core/headers)
include_directories(../../../DocFormats/core/src/lib)
include_directories(../../../DocFormats/unittest)
link_directories(${LIB_DIRS})

add_executable(dftest ${SOURCES})
target_link_libraries(dftest DocFormats ${LIBS})

source_group(\\ FILES ${SOURCES})
