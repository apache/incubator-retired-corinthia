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

set(GroupCoreCommon
    src/common/DFBDT.c
    src/common/DFBDT.h
    src/common/DFClassNames.h
    src/common/DFTable.c
    src/common/DFTable.h)

set(GroupCoreCSS
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
    src/css/CSSSyntax.h
    tests/css/CSSTests.c)

set(GroupCoreHTML
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
    src/html/DFTidyWrapper.h
    tests/html/HTMLTests.c)

set(GroupCoreLib
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
    tests/lib/LibTests.c)

set(GroupCoreNames
    src/names/DFXMLNames.c
    src/names/DFXMLNames.h
    src/names/DFXMLNamespaces.c
    src/names/DFXMLNamespaces.h)

set(GroupCoreXML
    src/xml/DFDOM.c
    src/xml/DFDOM.h
    src/xml/DFMarkupCompatibility.c
    src/xml/DFMarkupCompatibility.h
    src/xml/DFNameMap.c
    src/xml/DFNameMap.h
    src/xml/DFXML.c
    src/xml/DFXML.h
    tests/xml/XMLTests.c)

add_library(core OBJECT
    ${GroupCoreCommon}
    ${GroupCoreCSS}
    ${GroupCoreHTML}
    ${GroupCoreLib}
    ${GroupCoreNames}
    ${GroupCoreXML})

source_group(common FILES ${GroupCoreCommon})
source_group(css FILES ${GroupCoreCSS})
source_group(html FILES ${GroupCoreHTML})
source_group(lib FILES ${GroupCoreLib})
source_group(names FILES ${GroupCoreNames})
source_group(xml FILES ${GroupCoreXML})
