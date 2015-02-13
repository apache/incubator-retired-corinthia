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

#include "CSSStyle.h"
#include "CSSSheet.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSSelector.h"
#include "DFHTML.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// FIXME: The dirty property is supposed to be set whenever any aspect if this style changes,
// including any property of one of its rules, and the addition or removal of rules.

// I'm not sure if this is actually relevant in the context of the new model under which we set
// the stylesheet as a whole, because the in the old model styles were updated individually
// (which caused serialization of the entire stylesheet anyway, just in javascript).

static void ruleChanged(void *ctx, void *object, void *data)
{
    CSSStyle *style = (CSSStyle *)ctx;
    CSSProperties *properties = (CSSProperties *)object;
    if (properties->dirty) {
        properties->dirty = 0;
        if (!style->dirty) { // Minimise callback invocations
            style->dirty = 1;
            DFCallbackInvoke(style->changeCallbacks,style,NULL);
        }
    }
}

CSSStyle *CSSStyleNew(const char *selector)
{
    CSSStyle *style = (CSSStyle *)calloc(1,sizeof(CSSStyle));
    style->retainCount = 1;
    CSSStyleSetSelector(style,selector);
    style->rules = DFHashTableNew((DFCopyFunction)CSSPropertiesRetain,(DFFreeFunction)CSSPropertiesRelease);
    CSSStyleRuleForSuffix(style,""); // Create the base rule
    return style;
}

CSSStyle *CSSStyleRetain(CSSStyle *style)
{
    if (style != NULL)
        style->retainCount++;

    return style;
}

void CSSStyleRelease(CSSStyle *style)
{
    if ((style == NULL) || (--style->retainCount > 0))
        return;

    free(style->selector);
    free(style->elementName);
    free(style->className);
    const char **keys = DFHashTableCopyKeys(style->rules);
    for (int i = 0; keys[i]; i++) {
        CSSProperties *rule = DFHashTableLookup(style->rules,keys[i]);
        DFCallbackRemove(&rule->changeCallbacks,ruleChanged,style);
    }
    free(keys);
    DFHashTableRelease(style->rules);
    assert(style->changeCallbacks == NULL);
    free(style);
}

void CSSStyleSetSelector(CSSStyle *style, const char *newSelector)
{
    // Take a copy of newSelector first, just in case it's one of the values we're about to free
    char *selector = strdup(newSelector);

    free(style->selector);
    free(style->elementName);
    free(style->className);

    style->selector = selector;
    style->elementName = CSSSelectorCopyElementName(style->selector);
    style->className = CSSSelectorCopyClassName(style->selector);
    style->tag = CSSSelectorGetTag(style->selector);
    style->headingLevel = CSSSelectorHeadingLevel(style->selector);
    style->family = CSSSelectorFamily(style->selector);
}

int CSSStyleIsCustom(CSSStyle *style)
{
    if (CSSGetDefault(CSSStyleRule(style)))
        return 0;
    if (CSSIsBuiltinSelector(style->selector))
        return 0;
    return 1;
}

char *CSSStyleCopyParent(CSSStyle *style)
{
    const char *quotedValue = CSSGet(CSSStyleRule(style),"-uxwrite-parent");
    return DFUnquote(quotedValue);
}

void CSSStyleSetParent(CSSStyle *style, const char *newParent)
{
    char *quotedParent = DFQuote(newParent);
    CSSPut(CSSStyleRule(style),"-uxwrite-parent",quotedParent);
    free(quotedParent);
}

char *CSSStyleCopyNext(CSSStyle *style)
{
    const char *quotedValue = CSSGet(CSSStyleRule(style),"-uxwrite-next");
    return DFUnquote(quotedValue);
}

void CSSStyleSetNext(CSSStyle *style, const char *newNext)
{
    char *quotedNext = DFQuote(newNext);
    CSSPut(CSSStyleRule(style),"-uxwrite-next",quotedNext);
    free(quotedNext);
}

char *CSSStyleCopyDisplayName(CSSStyle *style)
{
    const char *quotedValue = CSSGet(CSSStyleRule(style),"-uxwrite-display-name");
    return DFUnquote(quotedValue);
}

void CSSStyleSetDisplayName(CSSStyle *style, const char *newDisplayName)
{
    char *quotedDisplayName = DFQuote(newDisplayName);
    CSSPut(CSSStyleRule(style),"-uxwrite-display-name",quotedDisplayName);
    free(quotedDisplayName);
}

const char **CSSStyleCopySuffixes(CSSStyle *style)
{
    return DFHashTableCopyKeys(style->rules);
}

CSSProperties *CSSStyleRuleForSuffix(CSSStyle *style, const char *suffix)
{
    if (style == NULL)
        return NULL;;

    CSSProperties *result = DFHashTableLookup(style->rules,suffix);
    if (result == NULL) {
        result = CSSPropertiesNew();
        DFHashTableAdd(style->rules,suffix,result);
        DFCallbackAdd(&result->changeCallbacks,ruleChanged,style);
        CSSPropertiesRelease(result);
    }
    return result;
}

static const char *suffixForTableComponent(const char *tableComponent)
{
    if (tableComponent == NULL)
        return NULL;
    else if (!strcmp(tableComponent,"wholeTable"))
        return DFTableSuffixWholeTable;
    else if (!strcmp(tableComponent,"cell"))
        return DFTableSuffixCell;
    else if (!strcmp(tableComponent,"firstRow"))
        return DFTableSuffixFirstRow;
    else if (!strcmp(tableComponent,"lastRow"))
        return DFTableSuffixLastRow;
    else if (!strcmp(tableComponent,"firstCol"))
        return DFTableSuffixFirstCol;
    else if (!strcmp(tableComponent,"lastCol"))
        return DFTableSuffixLastCol;
    else if (!strcmp(tableComponent,"band1Vert"))
        return DFTableSuffixBand1Vert;
    else if (!strcmp(tableComponent,"band2Vert"))
        return DFTableSuffixBand2Vert;
    else if (!strcmp(tableComponent,"band1Horz"))
        return DFTableSuffixBand1Horz;
    else if (!strcmp(tableComponent,"band2Horz"))
        return DFTableSuffixBand2Horz;
    else if (!strcmp(tableComponent,"nwCell"))
        return DFTableSuffixNWCell;
    else if (!strcmp(tableComponent,"neCell"))
        return DFTableSuffixNECell;
    else if (!strcmp(tableComponent,"swCell"))
        return DFTableSuffixSWCell;
    else if (!strcmp(tableComponent,"seCell"))
        return DFTableSuffixSECell;
    else
        return NULL;
}

CSSProperties *CSSStyleRuleForTableComponent(CSSStyle *style, const char *tableComponent)
{
    const char *suffix = suffixForTableComponent(tableComponent);
    if (suffix == NULL)
        return NULL;
    return CSSStyleRuleForSuffix(style,suffix);
}

CSSProperties *CSSStyleRule(CSSStyle *style)
{
    return CSSStyleRuleForSuffix(style,"");
}

int CSSStyleIsEmpty(CSSStyle *style)
{
    const char **allSuffixes = CSSStyleCopySuffixes(style);
    for (int i = 0; allSuffixes[i]; i++) {
        const char *suffix = allSuffixes[i];
        CSSProperties *properties = CSSStyleRuleForSuffix(style,suffix);
        if (DFHashTableCount(properties->hashTable) > 0) {
            free(allSuffixes);
            return 0;
        }
    }
    free(allSuffixes);
    return 1;
}

CSSProperties *CSSStyleCell(CSSStyle *style)
{
    return CSSStyleRuleForSuffix(style,DFTableSuffixCell);
}

CSSProperties *CSSStyleBefore(CSSStyle *style)
{
    return CSSStyleRuleForSuffix(style,"::before");
}

void CSSStyleAddDefaultHTMLProperties(CSSStyle *style)
{
    // FIXME: Not covered by tests
    char *elementName = CSSSelectorCopyElementName(style->selector);
    char *className = CSSSelectorCopyClassName(style->selector);
    if ((className == NULL) && (strlen(elementName) == 2) && (elementName[0] == 'h')) {
        CSSProperties *rule = CSSStyleRule(style);
        if (!strcmp(elementName,"h1")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","24pt");
        }
        else if (!strcmp(elementName,"h2")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","18pt");
        }
        else if (!strcmp(elementName,"h3")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","14pt");
        }
        else if (!strcmp(elementName,"h4")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","12pt");
        }
        else if (!strcmp(elementName,"h5")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","10pt");
        }
        else if (!strcmp(elementName,"h6")) {
            if (CSSGet(rule,"font-weight") == NULL)
                CSSPut(rule,"font-weight","bold");
            if (CSSGet(rule,"font-size") == NULL)
                CSSPut(rule,"font-size","8pt");
        }
    }
    free(elementName);
    free(className);
}

void CSSStylePrint(CSSStyle *style, const char *indent)
{
    char *propertiesIndent = DFFormatString("%s    ",indent);

    const char **allSuffixes = CSSStyleCopySuffixes(style);
    DFSortStringsCaseInsensitive(allSuffixes);
    for (int suffixIndex = 0; allSuffixes[suffixIndex]; suffixIndex++) {
        const char *suffix = allSuffixes[suffixIndex];
        char *quotedSuffix = DFQuote(suffix);
        printf("%ssuffix %s\n",indent,quotedSuffix);
        free(quotedSuffix);
        CSSProperties *properties = CSSStyleRuleForSuffix(style,suffix);
        CSSPropertiesPrint(properties,propertiesIndent);
    }
    free(allSuffixes);

    free(propertiesIndent);
}
