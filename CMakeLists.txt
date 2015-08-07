#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.



###
## global definitions
###
cmake_minimum_required(VERSION 3.1)
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
    set(INCLUDE_DIRS /usr/include/libxml2)
    set(LIBS ${LIBS} xml2 z ${FOUNDATION_LIBRARY} ${IMAGEIO_LIBRARY})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "-std=c99 -D_GNU_SOURCE -g -fPIC")
    set(INCLUDE_DIRS /usr/include/libxml2)
    set(LIBS ${LIBS} m pthread xml2 z SDL2 SDL2_image)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(INCLUDE_DIRS ${CMAKE_BINARY_DIR}/target_include)
    set(LIB_DIRS     ${CMAKE_BINARY_DIR}/target_lib)
    set(CMAKE_C_FLAGS "/DWIN64 /D_WINDOWS /W3 /wd4090 /wd4996")
    if(${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
      set(LIBS ${LIBS} libxml2_64 zlibwapi64 iconv64 SDL2_64 SDL2_image_64)
    else(${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
      set(LIBS ${LIBS} libxml2 zlibwapi iconv SDL2 SDL2_image)
    endif(${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")



###
## run subprojects
###
add_subdirectory(DocFormats)
add_subdirectory(consumers/dftest/src)
add_subdirectory(consumers/dfconvert/src)
