cmake_minimum_required(VERSION 2.8)

set(SOURCES
    main.c)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(.)
include_directories(../../../DocFormats/unittest)
link_directories(${LIB_DIRS})

add_executable(dftest ${SOURCES})
target_link_libraries(dftest DocFormats ${LIBS})
