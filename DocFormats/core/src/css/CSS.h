// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

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
