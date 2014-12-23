cmake_minimum_required(VERSION 2.8)
project(Corinthia)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)

set(LIBS )

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(IMAGEIO_LIBRARY ImageIO)
    set(LIBS ${LIBS} xml2 z ${FOUNDATION_LIBRARY} ${IMAGEIO_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "-std=c99 -D_GNU_SOURCE -g")
    set(LIBS ${LIBS} m pthread xml2 z SDL2 SDL2_image)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/external/include)
    set(LIB_DIRS
        ${PROJECT_SOURCE_DIR}/external/lib)
    set(LIBS ${LIBS} libxml2 zlib libiconv SDL2 SDL2_image)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4090 /wd4996")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_subdirectory(DocFormats)
add_subdirectory(consumers/dftest/src)
add_subdirectory(consumers/dfconvert/src)
add_subdirectory(consumers/dfutil/src)
