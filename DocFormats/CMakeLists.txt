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
## run subprojects
###
add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(unittest)
add_subdirectory(filters/ooxml)
add_subdirectory(filters/latex)
add_subdirectory(filters/odf)



###
## group main objects
###
set(GroupSrcRoot
    DocFormats.c)



###
## group main headers
###
set(GroupHeaderRoot
    headers/DFCommon.h
    headers/DFCore.h
    headers/DFPlatform.h
	headers/DFTypes.h)


###
## group subproject objects
###
set(GroupObjRoot
    $<TARGET_OBJECTS:api>
    $<TARGET_OBJECTS:core>
    $<TARGET_OBJECTS:unittest>
    $<TARGET_OBJECTS:ooxml>
    $<TARGET_OBJECTS:latex>
    $<TARGET_OBJECTS:odf>
    $<TARGET_OBJECTS:platform>)



###
# library (release artifact)
###
add_library(DocFormats
    ${GroupSrcRoot}
    ${GroupHeaderRoot}
    ${GroupObjRoot})

source_group(src     FILES ${GroupSrcRoot})
source_group(headers FILES ${GroupHeaderRoot})
set_property(TARGET DocFormats PROPERTY FOLDER DocFormats)
