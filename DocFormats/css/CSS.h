//
//  CSS.h
//  DocFormats
//
//  Created by Peter Kelly on 4/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_CSS_h
#define DocFormats_CSS_h

#include "CSSProperties.h"
#include "DFHashTable.h"
#include "DFArray.h"

typedef enum {
    PageSizeUnknown = 0,
    PageSizeA4Portrait,
    PageSizeA4Landscape,
    PageSizeLetterPortrait,
    PageSizeLetterLandscape,
} PageSize;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           ContentPart                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    ContentPartNormal,
    ContentPartNone,
    ContentPartString,
    ContentPartURI,
    ContentPartCounter,
    ContentPartCounters,
    ContentPartAttr,
    ContentPartOpenQuote,
    ContentPartCloseQuote,
    ContentPartNoOpenQuote,
    ContentPartNoCloseQuote,
} ContentPartType;

const char *ContentPartTypeString(ContentPartType type);

typedef struct ContentPart ContentPart;

struct ContentPart {
    size_t retainCount;
    ContentPartType type;
    char *value;
    char *arg;
};

ContentPart *ContentPartNew(ContentPartType type, const char *value, const char *arg);
ContentPart *ContentPartRetain(ContentPart *part);
void ContentPartRelease(ContentPart *part);

DFArray *CSSParseContent(const char *content);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          ListStyleType                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    ListStyleTypeDisc,
    ListStyleTypeCircle,
    ListStyleTypeSquare,
    ListStyleTypeDecimal,
    ListStyleTypeDecimalLeadingZero,
    ListStyleTypeLowerRoman,
    ListStyleTypeUpperRoman,
    ListStyleTypeLowerGreek,
    ListStyleTypeLowerLatin,
    ListStyleTypeUpperLatin,
    ListStyleTypeArmenian,
    ListStyleTypeGeorgian,
    ListStyleTypeLowerAlpha,
    ListStyleTypeUpperAlpha,
    ListStyleTypeNone,
} ListStyleType;

const char *ListStyleTypeString(ListStyleType type);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                               CSS                                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFHashTable *CSSParseProperties(const char *input);
char *CSSSerializeProperties(DFHashTable *cssProperties);
void CSSExpandProperties(DFHashTable *properties);
DFHashTable *CSSCollapseProperties(CSSProperties *expanded);

char *CSSCopyStylesheetTextFromRules(DFHashTable *rules);
char *CSSHexColor(const char *color, int includeHash);
char *CSSEncodeFontFamily(const char *input);
char *CSSDecodeFontFamily(const char *input);
int CSSIsInlineProperty(const char *name);
PageSize CSSParsePageSize(const char *input);
char *CSSEscapeIdent(const char *unescaped);
char *CSSUnescapeIdent(const char *escaped);
void CSSParseSelector(const char *input, char **result, char **suffix);

#endif
