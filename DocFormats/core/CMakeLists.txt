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
## group common objects incl test
###
set(GroupHeaders
    headers/DFCore.h
    headers/DFCommon.h
    headers/DFTypes.h)

set(GroupSrcCommon
    src/common/DFBDT.c
    src/common/DFBDT.h
    src/common/DFClassNames.h
    src/common/DFTable.c
    src/common/DFTable.h)

set(GroupTestsCommon
    tests/common/BDTTests.c
    tests/common/BDTTests.h)



###
## group CSS objects incl test
###
set(GroupSrcCSS
    src/css/CSS.c
    src/css/CSS.h
    src/css/CSSLength.c
    src/css/CSSLength.h
    src/css/CSSParser.c
    src/css/CSSParser.h
    src/css/CSSProperties.c
    src/css/CSSProperties.h
    src/css/CSSSelector.c
    src/css/CSSSelector.h
    src/css/CSSSheet.c
    src/css/CSSSheet.h
    src/css/CSSStyle.c
    src/css/CSSStyle.h
    src/css/CSSSyntax.c
    src/css/CSSSyntax.h)

set(GroupTestsCSS
    tests/css/CSSTests.c)



###
## group HTML objects incl test
###
set(GroupSrcHTML
    src/html/DFHTDocument.c
    src/html/DFHTDocument.h
    src/html/DFHTML.c
    src/html/DFHTML.h
    src/html/DFHTMLNormalization.c
    src/html/DFHTMLNormalization.h
    src/html/DFHTMLTables.c
    src/html/DFHTMLTables.h
    src/html/DFTidyHelper.c
    src/html/DFTidyHelper.h
    src/html/DFTidyWrapper.c
    src/html/DFTidyWrapper.h)

set(GroupTestsHTML
    tests/html/HTMLPlain.c
    tests/html/HTMLPlain.h
    tests/html/HTMLTests.c)



###
## group LIB objects incl test
###
set(GroupSrcLib
    src/lib/DFAllocator.c
    src/lib/DFAllocator.h
    src/lib/DFArray.c
    src/lib/DFArray.h
    src/lib/DFBuffer.c
    src/lib/DFBuffer.h
    src/lib/DFCallback.c
    src/lib/DFCallback.h
    src/lib/DFCharacterSet.c
    src/lib/DFCharacterSet.h
    src/lib/DFError.c
    src/lib/DFFilesystem.c
    src/lib/DFFilesystem.h
    src/lib/DFHashTable.c
    src/lib/DFHashTable.h
    src/lib/DFString.c
    src/lib/DFString.h
    src/lib/DFStorage.c
    src/lib/DFZipFile.c
    src/lib/DFZipFile.h
    src/lib/TextPackage.c
    src/lib/TextPackage.h)

set(GroupTestsLib
    tests/lib/LibTests.c)



###
## group NAMES objects incl test
###
set(GroupSrcNames
    src/names/DFXMLNames.c
    src/names/DFXMLNames.h
    src/names/DFXMLNamespaces.c
    src/names/DFXMLNamespaces.h)

set(GroupTestsNames
    tests/names/NamesTests.c)



###
## group XML objects incl test
###
set(GroupSrcXML
    src/xml/DFChanges.c
    src/xml/DFChanges.h
    src/xml/DFDOM.c
    src/xml/DFDOM.h
    src/xml/DFMarkupCompatibility.c
    src/xml/DFMarkupCompatibility.h
    src/xml/DFNameMap.c
    src/xml/DFNameMap.h
    src/xml/DFXML.c
    src/xml/DFXML.h)

set(GroupTestsXML
    tests/xml/XMLTests.c)



###
# Common include for all core files
###
include_directories()
include_directories(/usr/include/libxml2)
include_directories(../../DocFormats/platform/3rdparty/w3c-tidy-html5/include)
include_directories(../../DocFormats/platform/3rdparty//w3c-tidy-html5/src)
include_directories(../../DocFormats/platform/3rdparty/minizip)
include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../api/headers)
include_directories(../platform/headers)
include_directories(src/common)
include_directories(src/css)
include_directories(src/html)
include_directories(src/lib)
include_directories(src/names)
include_directories(src/xml)
include_directories(../unittest)
include_directories(headers)



###
# Prebuild library (needed to use different compile options)
###
add_library(core OBJECT
    ${GroupHeaders}
    ${GroupSrcCommon}
    ${GroupSrcCSS}
    ${GroupSrcHTML}
    ${GroupSrcLib}
    ${GroupSrcNames}
    ${GroupSrcXML}
    ${GroupTestsCommon}
    ${GroupTestsCSS}
    ${GroupTestsHTML}
    ${GroupTestsLib}
    ${GroupTestsNames}
    ${GroupTestsXML})

source_group(headers       FILES ${GroupHeaders})
source_group(src\\common   FILES ${GroupSrcCommon})
source_group(src\\css      FILES ${GroupSrcCSS})
source_group(src\\html     FILES ${GroupSrcHTML})
source_group(src\\lib      FILES ${GroupSrcLib})
source_group(src\\names    FILES ${GroupSrcNames})
source_group(src\\xml      FILES ${GroupSrcXML})
source_group(tests\\common FILES ${GroupTestsCommon})
source_group(tests\\css    FILES ${GroupTestsCSS})
source_group(tests\\html   FILES ${GroupTestsHTML})
source_group(tests\\lib    FILES ${GroupTestsLib})
source_group(tests\\names  FILES ${GroupTestsNames})
source_group(tests\\xml    FILES ${GroupTestsXML})
set_property(TARGET core PROPERTY FOLDER DocFormats)
