include_directories(/usr/include/libxml2)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/include)
include_directories(../../DocFormats/3rdparty/external/w3c-tidy-html5/src)
include_directories(../../DocFormats/3rdparty/external/minizip)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../api/headers)
include_directories(../../core/headers)
include_directories(../../platform/headers)
include_directories(../../core/src/common)
include_directories(../../core/src/css)
include_directories(../../core/src/html)
include_directories(../../core/src/lib)
include_directories(../../core/src/names)
include_directories(../../core/src/xml)
include_directories(../../core/tests/html)
include_directories(../../filters/latex/src)
include_directories(../../filters/odf/src)
include_directories(../../filters/ooxml/src/common)
include_directories(../../filters/ooxml/src/word)
include_directories(../../filters/ooxml/src/word/formatting)
include_directories(../../filters/ooxml/src/word/lenses)
include_directories(../../unittest)

set(GroupOOXMLCommon
    src/common/OOXMLTypedefs.h
    src/common/OPC.c
    src/common/OPC.h)

set(GroupOOXMLWord
    src/word/CSSClassNames.c
    src/word/CSSClassNames.h
    src/word/Word.c
    src/word/Word.h
    src/word/WordCaption.c
    src/word/WordCaption.h
    src/word/WordConverter.c
    src/word/WordConverter.h
    src/word/WordGC.c
    src/word/WordGC.h
    src/word/WordLists.c
    src/word/WordLists.h
    src/word/WordNotes.c
    src/word/WordNotes.h
    src/word/WordNumbering.c
    src/word/WordNumbering.h
    src/word/WordObjects.c
    src/word/WordObjects.h
    src/word/WordPackage.c
    src/word/WordPackage.h
    src/word/WordSection.c
    src/word/WordSection.h
    src/word/WordSettings.c
    src/word/WordSettings.h
    src/word/WordSheet.c
    src/word/WordSheet.h
    src/word/WordStyles.c
    src/word/WordStyles.h
    src/word/WordTheme.c
    src/word/WordTheme.h
    src/word/WordWhitespace.c
    src/word/WordWhitespace.h)

set(GroupOOXMLWordFormatting
    src/word/formatting/WordCommonPr.c
    src/word/formatting/WordCommonPr.h
    src/word/formatting/WordNumPr.c
    src/word/formatting/WordNumPr.h
    src/word/formatting/WordPPr.c
    src/word/formatting/WordPPr.h
    src/word/formatting/WordRPr.c
    src/word/formatting/WordRPr.h
    src/word/formatting/WordTblPr.c
    src/word/formatting/WordTblPr.h)

set(GroupOOXMLWordLenses
    src/word/lenses/WordBlockLevel.c
    src/word/lenses/WordBody.c
    src/word/lenses/WordBookmark.c
    src/word/lenses/WordBookmark.h
    src/word/lenses/WordChange.c
    src/word/lenses/WordDocument.c
    src/word/lenses/WordDrawing.c
    src/word/lenses/WordDrawing.h
    src/word/lenses/WordEquation.c
    src/word/lenses/WordField.c
    src/word/lenses/WordField.h
    src/word/lenses/WordHyperlink.c
    src/word/lenses/WordLenses.c
    src/word/lenses/WordLenses.h
    src/word/lenses/WordParagraph.c
    src/word/lenses/WordParagraphContent.c
    src/word/lenses/WordRun.c
    src/word/lenses/WordRunContent.c
    src/word/lenses/WordSmartTag.c
    src/word/lenses/WordTable.c)

set(GroupOOXMLWordTests
    tests/word/WordPlain.c
    tests/word/WordPlain.h
    tests/word/WordTests.c)

add_library(ooxml OBJECT
    ${GroupOOXMLCommon}
    ${GroupOOXMLWord}
    ${GroupOOXMLWordFormatting}
    ${GroupOOXMLWordLenses}
    ${GroupOOXMLWordTests})

source_group(src\\common FILES ${GroupOOXMLCommon})
source_group(src\\word FILES ${GroupOOXMLWord})
source_group(src\\word\\formatting FILES ${GroupOOXMLWordFormatting})
source_group(src\\word\\lenses FILES ${GroupOOXMLWordLenses})
source_group(tests\\word FILES ${GroupOOXMLWordTests})
