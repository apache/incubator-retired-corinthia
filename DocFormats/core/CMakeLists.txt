include_directories(/usr/include/libxml2)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/src)
include_directories(../../DocFormats/3rdparty/external/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../api/headers)
include_directories(..)
include_directories(../platform/headers)
include_directories(src/common)
include_directories(src/css)
include_directories(src/html)
include_directories(src/lib)
include_directories(src/names)
include_directories(src/xml)
include_directories(../unittest)

set(GroupSrcCommon
    src/common/DFBDT.c
    src/common/DFBDT.h
    src/common/DFClassNames.h
    src/common/DFTable.c
    src/common/DFTable.h)

set(GroupTestsCommon)

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
    tests/html/HTMLTests.c)

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
    src/lib/DFZipFile.h)

set(GroupTestsLib
    tests/lib/LibTests.c)

set(GroupSrcNames
    src/names/DFXMLNames.c
    src/names/DFXMLNames.h
    src/names/DFXMLNamespaces.c
    src/names/DFXMLNamespaces.h)

set(GroupSrcXML
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

add_library(core OBJECT
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
    ${GroupTestsXML})

source_group(src\\common FILES ${GroupSrcCommon})
source_group(src\\css FILES ${GroupSrcCSS})
source_group(src\\html FILES ${GroupSrcHTML})
source_group(src\\lib FILES ${GroupSrcLib})
source_group(src\\names FILES ${GroupSrcNames})
source_group(src\\xml FILES ${GroupSrcXML})

source_group(tests\\common FILES ${GroupTestsCommon})
source_group(tests\\css FILES ${GroupTestsCSS})
source_group(tests\\html FILES ${GroupTestsHTML})
source_group(tests\\lib FILES ${GroupTestsLib})
source_group(tests\\xml FILES ${GroupTestsXML})
