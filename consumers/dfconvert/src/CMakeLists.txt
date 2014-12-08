cmake_minimum_required(VERSION 2.8)

set(SOURCES
    main.c)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../../DocFormats/api/headers)
include_directories(.)
link_directories(${LIB_DIRS})

add_executable(dfconvert ${SOURCES})
target_link_libraries(dfconvert DocFormats ${LIBS})
