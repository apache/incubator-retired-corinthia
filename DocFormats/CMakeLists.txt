cmake_minimum_required(VERSION 2.8)
include_directories(/usr/include/libxml2)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "-std=c99 -D_GNU_SOURCE -g")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")



include_directories(../3rdparty/w3c-tidy-html5/include)
include_directories(../3rdparty/w3c-tidy-html5/src)
include_directories(../3rdparty/minizip)

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
add_library(DocFormats

    DFBDT.c
    DFPlatform.c
    DFTable.c
    css/CSS.c
    css/CSSClassNames.c
    css/CSSLength.c
    css/CSSParser.c
    css/CSSProperties.c
    css/CSSSelector.c
    css/CSSSheet.c
    css/CSSStyle.c
    css/CSSSyntax.c
    html/DFHTDocument.c
    html/DFHTML.c
    html/DFHTMLNormalization.c
    html/DFHTMLTables.c
    html/DFTidyHelper.c
    html/DFTidyWrapper.c
    latex/HTMLToLaTeX.c
    lib/DFAllocator.c
    lib/DFArray.c
    lib/DFBuffer.c
    lib/DFCallback.c
    lib/DFCharacterSet.c
    lib/DFError.c
    lib/DFFilesystem.c
    lib/DFHashTable.c
    lib/DFString.c
    lib/DFZipFile.c
    names/DFXMLNames.c
    names/DFXMLNamespaces.c
    odf/ODF.c
    odf/ODFManifest.c
    odf/ODFPackage.c
    odf/ODFSheet.c
    word/OPC.c
    word/Word.c
    word/WordCaption.c
    word/WordConverter.c
    word/WordGC.c
    word/WordLists.c
    word/WordNotes.c
    word/WordNumbering.c
    word/WordObjects.c
    word/WordPackage.c
    word/WordSection.c
    word/WordSettings.c
    word/WordSheet.c
    word/WordStyles.c
    word/WordTheme.c
    word/WordWhitespace.c
    word/formatting/WordCommonPr.c
    word/formatting/WordNumPr.c
    word/formatting/WordPPr.c
    word/formatting/WordRPr.c
    word/formatting/WordTblPr.c
    word/lenses/WordBlockLevel.c
    word/lenses/WordBody.c
    word/lenses/WordBookmark.c
    word/lenses/WordChange.c
    word/lenses/WordDocument.c
    word/lenses/WordDrawing.c
    word/lenses/WordEquation.c
    word/lenses/WordField.c
    word/lenses/WordHyperlink.c
    word/lenses/WordLenses.c
    word/lenses/WordParagraph.c
    word/lenses/WordParagraphContent.c
    word/lenses/WordRun.c
    word/lenses/WordRunContent.c
    word/lenses/WordSmartTag.c
    word/lenses/WordTable.c
    xml/DFDOM.c
    xml/DFMarkupCompatibility.c
    xml/DFNameMap.c
    xml/DFXML.c
)
