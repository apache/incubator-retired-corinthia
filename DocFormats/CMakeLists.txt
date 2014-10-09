cmake_minimum_required(VERSION 2.8)
include_directories(/usr/include/libxml2)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "-std=c99 -D_GNU_SOURCE -g")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set_property( GLOBAL PROPERTY USE_FOLDERS ON)


include_directories(../3rdparty/w3c-tidy-html5/include)
include_directories(../3rdparty/w3c-tidy-html5/src)
include_directories(../3rdparty/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(.)
include_directories(./css)
include_directories(./html)
include_directories(./latex)
include_directories(./lib)
include_directories(./names)
include_directories(./odf)
include_directories(./word)
include_directories(./word/formatting)
include_directories(./word/lenses)
include_directories(./xml)

set(GroupRoot
    DFBDT.c
    DFBDT.h
    DFClassNames.h
    DFCommon.h
    DFPlatform.h
    DFTable.c
    DFTable.h
    DFTypes.h
    DocFormats.h)

set(GroupPlatform
    platform/Apple.c
    platform/Linux.c
    platform/Unix.c
    platform/Win32.c)

set(GroupCSS
    css/CSS.c
    css/CSS.h
    css/CSSClassNames.c
    css/CSSClassNames.h
    css/CSSLength.c
    css/CSSLength.h
    css/CSSParser.c
    css/CSSParser.h
    css/CSSProperties.c
    css/CSSProperties.h
    css/CSSSelector.c
    css/CSSSelector.h
    css/CSSSheet.c
    css/CSSSheet.h
    css/CSSStyle.c
    css/CSSStyle.h
    css/CSSSyntax.c
    css/CSSSyntax.h)

set(GroupHTML
    html/DFHTDocument.c
    html/DFHTDocument.h
    html/DFHTML.c
    html/DFHTML.h
    html/DFHTMLNormalization.c
    html/DFHTMLNormalization.h
    html/DFHTMLTables.c
    html/DFHTMLTables.h
    html/DFTidyHelper.c
    html/DFTidyHelper.h
    html/DFTidyWrapper.c
    html/DFTidyWrapper.h)

set(GroupLaTeX
    latex/HTMLToLaTeX.c
    latex/HTMLToLaTeX.h)

set(GroupLib
    lib/DFAllocator.c
    lib/DFAllocator.h
    lib/DFArray.c
    lib/DFArray.h
    lib/DFBuffer.c
    lib/DFBuffer.h
    lib/DFCallback.c
    lib/DFCallback.h
    lib/DFCharacterSet.c
    lib/DFCharacterSet.h
    lib/DFError.c
    lib/DFError.h
    lib/DFFilesystem.c
    lib/DFFilesystem.h
    lib/DFHashTable.c
    lib/DFHashTable.h
    lib/DFStore.c
    lib/DFStore.h
    lib/DFString.c
    lib/DFString.h
    lib/DFZipFile.c
    lib/DFZipFile.h)

set(GroupNames
    names/DFXMLNames.c
    names/DFXMLNames.h
    names/DFXMLNamespaces.c
    names/DFXMLNamespaces.h)

set(GroupODF
    odf/ODF.c
    odf/ODF.h
    odf/ODFManifest.c
    odf/ODFManifest.h
    odf/ODFPackage.c
    odf/ODFPackage.h
    odf/ODFSheet.c
    odf/ODFSheet.h)

set(GroupWord
    word/OPC.c
    word/OPC.h
    word/Word.c
    word/Word.h
    word/WordCaption.c
    word/WordCaption.h
    word/WordConverter.c
    word/WordConverter.h
    word/WordGC.c
    word/WordGC.h
    word/WordLists.c
    word/WordLists.h
    word/WordNotes.c
    word/WordNotes.h
    word/WordNumbering.c
    word/WordNumbering.h
    word/WordObjects.c
    word/WordObjects.h
    word/WordPackage.c
    word/WordPackage.h
    word/WordSection.c
    word/WordSection.h
    word/WordSettings.c
    word/WordSettings.h
    word/WordSheet.c
    word/WordSheet.h
    word/WordStyles.c
    word/WordStyles.h
    word/WordTheme.c
    word/WordTheme.h
    word/WordWhitespace.c
    word/WordWhitespace.h)

set(GroupWordFormatting
    word/formatting/WordCommonPr.c
    word/formatting/WordCommonPr.h
    word/formatting/WordNumPr.c
    word/formatting/WordNumPr.h
    word/formatting/WordPPr.c
    word/formatting/WordPPr.h
    word/formatting/WordRPr.c
    word/formatting/WordRPr.h
    word/formatting/WordTblPr.c
    word/formatting/WordTblPr.h)

set(GroupWordLenses
    word/lenses/WordBlockLevel.c
    word/lenses/WordBody.c
    word/lenses/WordBookmark.c
    word/lenses/WordBookmark.h
    word/lenses/WordChange.c
    word/lenses/WordDocument.c
    word/lenses/WordDrawing.c
    word/lenses/WordDrawing.h
    word/lenses/WordEquation.c
    word/lenses/WordField.c
    word/lenses/WordField.h
    word/lenses/WordHyperlink.c
    word/lenses/WordLenses.c
    word/lenses/WordLenses.h
    word/lenses/WordParagraph.c
    word/lenses/WordParagraphContent.c
    word/lenses/WordRun.c
    word/lenses/WordRunContent.c
    word/lenses/WordSmartTag.c
    word/lenses/WordTable.c)

set(GroupXML
    xml/DFDOM.c
    xml/DFDOM.h
    xml/DFMarkupCompatibility.c
    xml/DFMarkupCompatibility.h
    xml/DFNameMap.c
    xml/DFNameMap.h
    xml/DFXML.c
    xml/DFXML.h
    xml/DFXMLForward.h)

add_library(DocFormats
    ${GroupRoot}
    ${GroupCSS}
    ${GroupHTML}
    ${GroupLaTeX}
    ${GroupLib}
    ${GroupNames}
    ${GroupODF}
    ${GroupPlatform}
    ${GroupWord}
    ${GroupWordFormatting}
    ${GroupWordLenses}
    ${GroupXML})

source_group(\\ FILES ${GroupRoot})
source_group(css FILES ${GroupCSS})
source_group(html FILES ${GroupHTML})
source_group(latex FILES ${GroupLaTeX})
source_group(lib FILES ${GroupLib})
source_group(names FILES ${GroupNames})
source_group(odf FILES ${GroupODF})
source_group(platform FILES ${GroupPlatform})
source_group(word FILES ${GroupWord})
source_group(word\\formatting FILES ${GroupWordFormatting})
source_group(word\\lenses FILES ${GroupWordLenses})
source_group(xml FILES ${GroupXML})
