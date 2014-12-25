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
cmake_minimum_required(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



###
## group source objects
###
set(SOURCES
    Commands.c
    Commands.h
    FunctionTests.h
    StringTests.h
    main.c
)



###
# Common include for all platform files
###
include_directories()
include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../../DocFormats/api/headers)
include_directories(.)
include_directories(../../../DocFormats/headers)
include_directories(../../../DocFormats/unittest)
include_directories(../../../DocFormats/platform/headers)
include_directories(../../../DocFormats/core/src/common)
include_directories(../../../DocFormats/core/src/css)
include_directories(../../../DocFormats/core/src/html)
include_directories(../../../DocFormats/core/src/lib)
include_directories(../../../DocFormats/core/src/names)
include_directories(../../../DocFormats/core/src/xml)
include_directories(../../../DocFormats/core/tests/html)
include_directories(../../../DocFormats/core/tests/common)
include_directories(../../../DocFormats/filters/latex/src)
include_directories(../../../DocFormats/filters/odf/src)
include_directories(../../../DocFormats/filters/ooxml/src/common)
include_directories(../../../DocFormats/filters/ooxml/src/word)
include_directories(../../../DocFormats/filters/ooxml/src/word/formatting)
include_directories(../../../DocFormats/filters/ooxml/src/word/lenses)
include_directories(../../../DocFormats/filters/ooxml/tests/word)
include_directories(/usr/include/libxml2)
link_directories(${LIB_DIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SOURCES ${SOURCES} FunctionTests.m StringTests.m)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")



###
# executable (release artifact)
###
add_executable(dfutil ${SOURCES})
target_link_libraries(dfutil DocFormats ${LIBS})
source_group(src FILES ${SOURCES})
set_property(TARGET dfutil PROPERTY FOLDER consumers)
