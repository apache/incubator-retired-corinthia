cmake_minimum_required(VERSION 2.8)
project(DocFormats)

set(LIBS tidy minizip)

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
        ${PROJECT_SOURCE_DIR}/libs/zlib/include
        ${PROJECT_SOURCE_DIR}/libs/iconv/include
        ${PROJECT_SOURCE_DIR}/libs/libxml2/include
        ${PROJECT_SOURCE_DIR}/libs/SDL2/include
        ${PROJECT_SOURCE_DIR}/libs/SDL2_image/include)
    set(LIB_DIRS
        ${PROJECT_SOURCE_DIR}/libs/zlib/lib
        ${PROJECT_SOURCE_DIR}/libs/iconv/lib
        ${PROJECT_SOURCE_DIR}/libs/libxml2/lib
        ${PROJECT_SOURCE_DIR}/libs/SDL2/lib/x86
        ${PROJECT_SOURCE_DIR}/libs/SDL2_image/lib/x86)
    set(LIBS ${LIBS} libxml2 zdll iconv SDL2 SDL2_image)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4090 /wd4996")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_subdirectory(DocFormats/3rdparty/external/minizip)
add_subdirectory(DocFormats/3rdparty/external/w3c-tidy-html5)
add_subdirectory(DocFormats)
add_subdirectory(consumers/dfutil/src)
