cmake_minimum_required(VERSION 2.8)
include_directories(/usr/include/libxml2)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "-std=c99 -D_GNU_SOURCE -g")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set_property( GLOBAL PROPERTY USE_FOLDERS ON)


include_directories(../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../DocFormats/3rdparty/external/w3c-tidy-html5/src)
include_directories(../DocFormats/3rdparty/external/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ./api/headers)
include_directories(.)
include_directories(./platform/headers)
include_directories(./core/src/common)
include_directories(./core/src/css)
include_directories(./core/src/html)
include_directories(./core/src/lib)
include_directories(./core/src/names)
include_directories(./core/src/xml)
include_directories(./filters/latex/src)
include_directories(./filters/odf/src)
include_directories(./filters/ooxml/src/common)
include_directories(./filters/ooxml/src/word)
include_directories(./filters/ooxml/src/word/formatting)
include_directories(./filters/ooxml/src/word/lenses)
include_directories(./unittest)

set(GroupRoot
    DFCommon.h
    DFTypes.h)

set(GroupAPI
    api/headers/DocFormats/DFError.h
    api/headers/DocFormats/DFStorage.h
    api/headers/DocFormats/DFXMLForward.h
    api/headers/DocFormats/DocFormats.h
    api/headers/DocFormats/Formats.h
    api/headers/DocFormats/Operations.h
    api/src/Formats.c
    api/src/Operations.c
    api/tests/APITests.c)

set(GroupCoreCommon
    core/src/common/DFBDT.c
    core/src/common/DFBDT.h
    core/src/common/DFClassNames.h
    core/src/common/DFTable.c
    core/src/common/DFTable.h)

set(GroupPlatform
    platform/headers/DFPlatform.h
    platform/src/Apple.c
    platform/src/Linux.c
    platform/src/Unix.c
    platform/src/Win32.c
    platform/tests/PlatformTests.c)

set(GroupCoreCSS
    core/src/css/CSS.c
    core/src/css/CSS.h
    core/src/css/CSSClassNames.c
    core/src/css/CSSClassNames.h
    core/src/css/CSSLength.c
    core/src/css/CSSLength.h
    core/src/css/CSSParser.c
    core/src/css/CSSParser.h
    core/src/css/CSSProperties.c
    core/src/css/CSSProperties.h
    core/src/css/CSSSelector.c
    core/src/css/CSSSelector.h
    core/src/css/CSSSheet.c
    core/src/css/CSSSheet.h
    core/src/css/CSSStyle.c
    core/src/css/CSSStyle.h
    core/src/css/CSSSyntax.c
    core/src/css/CSSSyntax.h
    core/tests/css/CSSTests.c)

set(GroupCoreHTML
    core/src/html/DFHTDocument.c
    core/src/html/DFHTDocument.h
    core/src/html/DFHTML.c
    core/src/html/DFHTML.h
    core/src/html/DFHTMLNormalization.c
    core/src/html/DFHTMLNormalization.h
    core/src/html/DFHTMLTables.c
    core/src/html/DFHTMLTables.h
    core/src/html/DFTidyHelper.c
    core/src/html/DFTidyHelper.h
    core/src/html/DFTidyWrapper.c
    core/src/html/DFTidyWrapper.h
    core/tests/html/HTMLTests.c)

set(GroupLaTeX
    filters/latex/src/HTMLToLaTeX.c
    filters/latex/src/HTMLToLaTeX.h
    filters/latex/tests/LaTeXTests.c)

set(GroupCoreLib
    core/src/lib/DFAllocator.c
    core/src/lib/DFAllocator.h
    core/src/lib/DFArray.c
    core/src/lib/DFArray.h
    core/src/lib/DFBuffer.c
    core/src/lib/DFBuffer.h
    core/src/lib/DFCallback.c
    core/src/lib/DFCallback.h
    core/src/lib/DFCharacterSet.c
    core/src/lib/DFCharacterSet.h
    core/src/lib/DFError.c
    core/src/lib/DFFilesystem.c
    core/src/lib/DFFilesystem.h
    core/src/lib/DFHashTable.c
    core/src/lib/DFHashTable.h
    core/src/lib/DFString.c
    core/src/lib/DFString.h
    core/src/lib/DFStorage.c
    core/src/lib/DFZipFile.c
    core/src/lib/DFZipFile.h
    core/tests/lib/LibTests.c)

set(GroupCoreNames
    core/src/names/DFXMLNames.c
    core/src/names/DFXMLNames.h
    core/src/names/DFXMLNamespaces.c
    core/src/names/DFXMLNamespaces.h)

set(GroupUnitTest
    unittest/DFUnitTest.h
    unittest/DFUnitTest.c)

set(GroupODF
    filters/odf/src/ODF.c
    filters/odf/src/ODF.h
    filters/odf/src/ODFManifest.c
    filters/odf/src/ODFManifest.h
    filters/odf/src/ODFPackage.c
    filters/odf/src/ODFPackage.h
    filters/odf/src/ODFSheet.c
    filters/odf/src/ODFSheet.h)

set(GroupOOXMLCommon
    filters/ooxml/src/common/OOXMLTypedefs.h
    filters/ooxml/src/common/OPC.c
    filters/ooxml/src/common/OPC.h)

set(GroupOOXMLWord
    filters/ooxml/src/word/Word.c
    filters/ooxml/src/word/Word.h
    filters/ooxml/src/word/WordCaption.c
    filters/ooxml/src/word/WordCaption.h
    filters/ooxml/src/word/WordConverter.c
    filters/ooxml/src/word/WordConverter.h
    filters/ooxml/src/word/WordGC.c
    filters/ooxml/src/word/WordGC.h
    filters/ooxml/src/word/WordLists.c
    filters/ooxml/src/word/WordLists.h
    filters/ooxml/src/word/WordNotes.c
    filters/ooxml/src/word/WordNotes.h
    filters/ooxml/src/word/WordNumbering.c
    filters/ooxml/src/word/WordNumbering.h
    filters/ooxml/src/word/WordObjects.c
    filters/ooxml/src/word/WordObjects.h
    filters/ooxml/src/word/WordPackage.c
    filters/ooxml/src/word/WordPackage.h
    filters/ooxml/src/word/WordSection.c
    filters/ooxml/src/word/WordSection.h
    filters/ooxml/src/word/WordSettings.c
    filters/ooxml/src/word/WordSettings.h
    filters/ooxml/src/word/WordSheet.c
    filters/ooxml/src/word/WordSheet.h
    filters/ooxml/src/word/WordStyles.c
    filters/ooxml/src/word/WordStyles.h
    filters/ooxml/src/word/WordTheme.c
    filters/ooxml/src/word/WordTheme.h
    filters/ooxml/src/word/WordWhitespace.c
    filters/ooxml/src/word/WordWhitespace.h
    filters/ooxml/tests/word/WordTests.c)

set(GroupOOXMLWordFormatting
    filters/ooxml/src/word/formatting/WordCommonPr.c
    filters/ooxml/src/word/formatting/WordCommonPr.h
    filters/ooxml/src/word/formatting/WordNumPr.c
    filters/ooxml/src/word/formatting/WordNumPr.h
    filters/ooxml/src/word/formatting/WordPPr.c
    filters/ooxml/src/word/formatting/WordPPr.h
    filters/ooxml/src/word/formatting/WordRPr.c
    filters/ooxml/src/word/formatting/WordRPr.h
    filters/ooxml/src/word/formatting/WordTblPr.c
    filters/ooxml/src/word/formatting/WordTblPr.h)

set(GroupOOXMLWordLenses
    filters/ooxml/src/word/lenses/WordBlockLevel.c
    filters/ooxml/src/word/lenses/WordBody.c
    filters/ooxml/src/word/lenses/WordBookmark.c
    filters/ooxml/src/word/lenses/WordBookmark.h
    filters/ooxml/src/word/lenses/WordChange.c
    filters/ooxml/src/word/lenses/WordDocument.c
    filters/ooxml/src/word/lenses/WordDrawing.c
    filters/ooxml/src/word/lenses/WordDrawing.h
    filters/ooxml/src/word/lenses/WordEquation.c
    filters/ooxml/src/word/lenses/WordField.c
    filters/ooxml/src/word/lenses/WordField.h
    filters/ooxml/src/word/lenses/WordHyperlink.c
    filters/ooxml/src/word/lenses/WordLenses.c
    filters/ooxml/src/word/lenses/WordLenses.h
    filters/ooxml/src/word/lenses/WordParagraph.c
    filters/ooxml/src/word/lenses/WordParagraphContent.c
    filters/ooxml/src/word/lenses/WordRun.c
    filters/ooxml/src/word/lenses/WordRunContent.c
    filters/ooxml/src/word/lenses/WordSmartTag.c
    filters/ooxml/src/word/lenses/WordTable.c)

set(GroupCoreXML
    core/src/xml/DFDOM.c
    core/src/xml/DFDOM.h
    core/src/xml/DFMarkupCompatibility.c
    core/src/xml/DFMarkupCompatibility.h
    core/src/xml/DFNameMap.c
    core/src/xml/DFNameMap.h
    core/src/xml/DFXML.c
    core/src/xml/DFXML.h
    core/tests/xml/XMLTests.c)

add_library(DocFormats
    ${GroupRoot}
    ${GroupAPI}
    ${GroupCoreCommon}
    ${GroupCoreCSS}
    ${GroupCoreHTML}
    ${GroupLaTeX}
    ${GroupCoreLib}
    ${GroupCoreNames}
    ${GroupODF}
    ${GroupPlatform}
    ${GroupUnitTest}
    ${GroupOOXMLCommon}
    ${GroupOOXMLWord}
    ${GroupOOXMLWordFormatting}
    ${GroupOOXMLWordLenses}
    ${GroupCoreXML})

source_group(\\ FILES ${GroupRoot})
source_group(api FILES ${GroupAPI})
source_group(core\\common FILES ${GroupCoreCommon})
source_group(core\\css FILES ${GroupCoreCSS})
source_group(core\\html FILES ${GroupCoreHTML})
source_group(core\\lib FILES ${GroupCoreLib})
source_group(core\\names FILES ${GroupCoreNames})
source_group(core\\xml FILES ${GroupCoreXML})
source_group(platform FILES ${GroupPlatform})
source_group(unittest FILES ${GroupUnitTest})
source_group(filters\\latex FILES ${GroupLaTeX})
source_group(filters\\odf FILES ${GroupODF})
source_group(filters\\ooxml\\common FILES ${GroupOOXMLCommon})
source_group(filters\\ooxml\\word FILES ${GroupOOXMLWord})
source_group(filters\\ooxml\\word\\formatting FILES ${GroupOOXMLWordFormatting})
source_group(filters\\ooxml\\word\\lenses FILES ${GroupOOXMLWordLenses})
