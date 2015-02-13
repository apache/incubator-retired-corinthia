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

#ifndef DocFormats_CSSSelector_h
#define DocFormats_CSSSelector_h

#include "DFXMLNames.h"
#include <DocFormats/DFXMLForward.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           StyleFamily                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    StyleFamilyUnknown,
    StyleFamilyParagraph,
    StyleFamilyCharacter,
    StyleFamilyTable,
    StyleFamilySpecial,
} StyleFamily;

StyleFamily StyleFamilyFromHTMLTag(Tag tag);

typedef struct SelectorList SelectorList;

struct SelectorList {
    char *selector;
    SelectorList *next;
};

char *CSSMakeSelector(const char *elementName, const char *className);
char *CSSMakeTagSelector(Tag tag, const char *className);
char *CSSMakeNodeSelector(DFNode *node);

Tag CSSSelectorGetTag(const char *selector);
int CSSSelectorHasClassName(const char *selector);
char *CSSSelectorCopyElementName(const char *selector);
char *CSSSelectorCopyClassName(const char *selector);

int CSSSelectorHeadingLevel(const char *selector); // 0 for all non-heading styles, 1-6 otherwise
int CSSSelectorIsHeading(const char *selector);
StyleFamily CSSSelectorFamily(const char *selector);

#endif
