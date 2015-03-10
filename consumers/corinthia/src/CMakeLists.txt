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
    editWindows.h
    desktop.cpp
    main.cpp)
set(UI_SOURCES
    desktop.ui)



###
# Common include for all platform files
###
include_directories()
include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(.)
link_directories(${LIB_DIRS})



###
## Qt magic
###
set(CMAKE_PREFIX_PATH         "C:/Qt/5.4/msvc2013_64")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC             ON)

find_package(Qt5Widgets REQUIRED)
include_directories("C:/Qt/5.4/msvc2013_64/include")

qt5_wrap_ui(UI_HEADERS ${UI_SOURCES})
source_group("Generated UI Headers" FILES ${UI_HEADERS})




###
# executable (release artifact)
###
add_executable(corinthia ${SOURCES} ${UI_HEADERS} ${UI_FILES})
qt5_use_modules(corinthia Widgets)
target_link_libraries(corinthia DocFormats ${LIBS})
source_group(src FILES ${SOURCES})
set_property(TARGET corinthia PROPERTY FOLDER consumers)
