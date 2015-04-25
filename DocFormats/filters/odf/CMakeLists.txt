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
## group odf objects
###
set(GroupSrc
    src/ODF.c
    src/ODF.h
    src/ODFManifest.c
    src/ODFManifest.h
    src/ODFPackage.c
    src/ODFPackage.h
    src/ODFSheet.c
    src/ODFSheet.h)

set(GroupSrcText
    src/text/ODFText.h
    src/text/ODFText.c)

set(GroupTests
    tests/ODFTests.c)



###
# Common include for all platform files
###
include_directories()
include_directories(/usr/include/libxml2)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/src)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../api/headers)
include_directories(../../headers)
include_directories(../../core/src/common)
include_directories(../../core/src/css)
include_directories(../../core/src/html)
include_directories(../../core/src/lib)
include_directories(../../core/src/names)
include_directories(../../core/src/xml)
include_directories(../../filters/latex/src)
include_directories(../../filters/odf/src)
include_directories(../../filters/ooxml/src/common)
include_directories(../../filters/ooxml/src/word)
include_directories(../../filters/ooxml/src/word/formatting)
include_directories(../../filters/ooxml/src/word/lenses)
include_directories(../../unittest)



###
# Prebuild library (needed to use different compile options)
###
add_library(odf OBJECT
    ${GroupSrc}
    ${GroupSrcText}
    ${GroupTests})
source_group(src         FILES ${GroupSrc})
source_group(src\\text   FILES ${GroupSrcText})
source_group(tests       FILES ${GroupTests})
set_property(TARGET odf PROPERTY FOLDER DocFormats/filters)
