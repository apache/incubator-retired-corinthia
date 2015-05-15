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
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



###
## group source objects
###
set(SOURCES
    Editor.h
    Editor.cpp
    JSInterface.h
    JSInterface.cpp
    main.cpp
    MainWindow.h
    MainWindow.cpp
    Toolbar.h
    Toolbar.cpp)
#set(UI_SOURCES
#    desktop.ui)



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
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC             ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKit REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

qt5_wrap_ui(UI_HEADERS ${UI_SOURCES})
source_group("Generated UI Headers" FILES ${UI_HEADERS})




###
# executable (release artifact)
###
add_executable(corinthia ${SOURCES} ${UI_HEADERS} ${UI_FILES})
qt5_use_modules(corinthia Widgets WebKit WebKitWidgets)
target_link_libraries(corinthia DocFormats ${LIBS})
source_group(src FILES ${SOURCES})
set_property(TARGET corinthia PROPERTY FOLDER consumers)

add_custom_command(TARGET corinthia PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/Editor/src ${CMAKE_BINARY_DIR}/share/corinthia/js)
add_custom_command(TARGET corinthia PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/consumers/corinthia/res ${CMAKE_BINARY_DIR}/share/corinthia)
