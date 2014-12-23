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
## group minizip objects
###
set(GroupMinizip
    3rdparty/minizip/crypt.h
    3rdparty/minizip/ioapi.c
    3rdparty/minizip/ioapi.h
    3rdparty/minizip/unzip.c
    3rdparty/minizip/unzip.h
    3rdparty/minizip/zip.c
    3rdparty/minizip/zip.h
   )



###
## group w3c-tidy-html5 objects
###
set(GroupHtml5
# NOT SUPPORTED            3rdparty/w3c-tidy-html5/license.html
    3rdparty/w3c-tidy-html5/include/buffio.h
    3rdparty/w3c-tidy-html5/include/platform.h
    3rdparty/w3c-tidy-html5/include/tidyenum.h
    3rdparty/w3c-tidy-html5/include/tidy.h
    3rdparty/w3c-tidy-html5/src/access.c
    3rdparty/w3c-tidy-html5/src/access.h
    3rdparty/w3c-tidy-html5/src/alloc.c
    3rdparty/w3c-tidy-html5/src/attrask.c
    3rdparty/w3c-tidy-html5/src/attrdict.c
    3rdparty/w3c-tidy-html5/src/attrdict.h
    3rdparty/w3c-tidy-html5/src/attrget.c
    3rdparty/w3c-tidy-html5/src/attrs.c
    3rdparty/w3c-tidy-html5/src/attrs.h
    3rdparty/w3c-tidy-html5/src/buffio.c
    3rdparty/w3c-tidy-html5/src/charsets.c
    3rdparty/w3c-tidy-html5/src/charsets.h
    3rdparty/w3c-tidy-html5/src/clean.c
    3rdparty/w3c-tidy-html5/src/clean.h
    3rdparty/w3c-tidy-html5/src/config.c
    3rdparty/w3c-tidy-html5/src/config.h
    3rdparty/w3c-tidy-html5/src/entities.c
    3rdparty/w3c-tidy-html5/src/entities.h
    3rdparty/w3c-tidy-html5/src/fileio.c
    3rdparty/w3c-tidy-html5/src/fileio.h
    3rdparty/w3c-tidy-html5/src/forward.h
    3rdparty/w3c-tidy-html5/src/gdoc.c
    3rdparty/w3c-tidy-html5/src/gdoc.h
    3rdparty/w3c-tidy-html5/src/iconvtc.c
    3rdparty/w3c-tidy-html5/src/iconvtc.h
    3rdparty/w3c-tidy-html5/src/istack.c
    3rdparty/w3c-tidy-html5/src/lexer.c
    3rdparty/w3c-tidy-html5/src/lexer.h
    3rdparty/w3c-tidy-html5/src/localize.c
    3rdparty/w3c-tidy-html5/src/mappedio.c
    3rdparty/w3c-tidy-html5/src/mappedio.h
    3rdparty/w3c-tidy-html5/src/message.h
    3rdparty/w3c-tidy-html5/src/parser.c
    3rdparty/w3c-tidy-html5/src/parser.h
    3rdparty/w3c-tidy-html5/src/pprint.c
    3rdparty/w3c-tidy-html5/src/pprint.h
    3rdparty/w3c-tidy-html5/src/streamio.c
    3rdparty/w3c-tidy-html5/src/streamio.h
    3rdparty/w3c-tidy-html5/src/tagask.c
    3rdparty/w3c-tidy-html5/src/tags.c
    3rdparty/w3c-tidy-html5/src/tags.h
    3rdparty/w3c-tidy-html5/src/tidy-int.h
    3rdparty/w3c-tidy-html5/src/tidylib.c
    3rdparty/w3c-tidy-html5/src/tmbstr.c
    3rdparty/w3c-tidy-html5/src/tmbstr.h
    3rdparty/w3c-tidy-html5/src/utf8.c
    3rdparty/w3c-tidy-html5/src/utf8.h
    3rdparty/w3c-tidy-html5/src/version.h
    3rdparty/w3c-tidy-html5/src/win32tc.c
    3rdparty/w3c-tidy-html5/src/win32tc.h
   )


###
## group platform code
###
set(GroupHeaders
    headers/DFPlatform.h)

set(GroupSrc
    src/Apple.c
    src/Linux.c
    src/Unix.c
    src/Win32.c)

set(GroupTests
    tests/PlatformTests.c)



###
# Common include for all platform files
###
include_directories()
include_directories(3rdparty/minizip)
include_directories(3rdparty/w3c-tidy-html5/include)
include_directories(3rdparty/w3c-tidy-html5/src)
include_directories(../../external/include)
include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(headers)
include_directories(../unittest)



###
# Prebuild library (needed to use different compile options)
###
add_library(platform OBJECT
    ${GroupMinizip}
	${GroupHtml5}
    ${GroupHeaders}
    ${GroupSrc}
    ${GroupTests})
source_group(headers             FILES ${GroupHeaders})
source_group(src                 FILES ${GroupSrc})
source_group(tests               FILES ${GroupTests})
source_group(src\\minizip        FILES ${GroupMinizip})
source_group(src\\w3c-tidy-html5 FILES ${GroupHtml5})
set_property(TARGET platform PROPERTY FOLDER DocFormats)
