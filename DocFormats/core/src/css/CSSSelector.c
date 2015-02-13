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

#include "DFPlatform.h"
#include "CSSSelector.h"
#include "DFNameMap.h"
#include "DFString.h"
#include "DFDOM.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           StyleFamily                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

StyleFamily StyleFamilyFromHTMLTag(Tag tag)
{
    switch (tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_DIV:
        case HTML_BLOCKQUOTE:
        case HTML_PRE:
            return StyleFamilyParagraph;
        case HTML_SPAN:
            return StyleFamilyCharacter;
        case HTML_TABLE:
            return StyleFamilyTable;
        case HTML_BODY:
        case HTML_FIGURE:
        case HTML_FIGCAPTION:
        case HTML_CAPTION:
            return StyleFamilySpecial;
        default:
            return StyleFamilyUnknown;
    }
}

char *CSSMakeSelector(const char *elementName, const char *className)
{
    assert(elementName != NULL);
    if (className != NULL)
        return DFFormatString("%s.%s",elementName,className);
    else
        return strdup(elementName);
}

char *CSSMakeTagSelector(Tag tag, const char *className)
{
    const TagDecl *decl = DFBuiltinMapNameForTag(tag);
    assert(decl != NULL);
    return CSSMakeSelector(decl->localName,className);
}

char *CSSMakeNodeSelector(DFNode *node)
{
    const char *elementName = DFTagName(node->doc,node->tag);
    const char *className = DFGetAttribute(node,HTML_CLASS);
    return CSSMakeSelector(elementName,className);
}

Tag CSSSelectorGetTag(const char *selector)
{
    if (selector == NULL)
        return 0;
    char *elementName = CSSSelectorCopyElementName(selector);
    Tag tag = DFBuiltinMapTagForName("http://www.w3.org/1999/xhtml",elementName);
    free(elementName);
    return tag;
}

static int findDotPos(const char *str)
{
    for (int pos = 0; str[pos]; pos++) {
        if (str[pos] == '.')
            return pos;
    }
    return -1;
}

int CSSSelectorHasClassName(const char *selector)
{
    if (selector == NULL)
        return 0;
    return (findDotPos(selector) >= 0);
}

char *CSSSelectorCopyElementName(const char *selector)
{
    if (selector == NULL)
        return NULL;
    int dotPos = findDotPos(selector);
    if (dotPos < 0)
        return strdup(selector);

    char *result = (char *)malloc(dotPos+1);
    memcpy(result,selector,dotPos);
    result[dotPos] = '\0';
    return result;
}

char *CSSSelectorCopyClassName(const char *selector)
{
    if (selector == NULL)
        return NULL;
    int dotPos = findDotPos(selector);
    if (dotPos < 0)
        return NULL;
    int start = dotPos + 1;
    int len = (int)strlen(selector) - start;
    char *result = (char *)malloc(len+1);
    memcpy(result,&selector[start],len);
    result[len] = '\0';
    return result;
}

int CSSSelectorHeadingLevel(const char *selector)
{
    switch (CSSSelectorGetTag(selector)) {
        case HTML_H1: return 1;
        case HTML_H2: return 2;
        case HTML_H3: return 3;
        case HTML_H4: return 4;
        case HTML_H5: return 5;
        case HTML_H6: return 6;
        default: return 0;
    }
}

int CSSSelectorIsHeading(const char *selector)
{
    switch (CSSSelectorGetTag(selector)) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
            return 1;
        default:
            return 0;
    }
}

StyleFamily CSSSelectorFamily(const char *selector)
{
    if (selector == NULL)
        return StyleFamilyUnknown;
    return StyleFamilyFromHTMLTag(CSSSelectorGetTag(selector));
}
