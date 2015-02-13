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

#include "CSSSheet.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSParser.h"
#include "CSSSelector.h"
#include "CSSLength.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFBuffer.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSSheet                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CSSSheet {
    size_t retainCount;
    DFHashTable *_styles; // const char * -> CSSStyle
    DFHashTable *_defaultStyles; // int(StyleFamily) -> CSSStyle
};

CSSSheet *CSSSheetNew(void)
{
    CSSSheet *sheet = (CSSSheet *)calloc(1,sizeof(CSSSheet));
    sheet->retainCount = 1;
    sheet->_styles = DFHashTableNew((DFCopyFunction)CSSStyleRetain,(DFFreeFunction)CSSStyleRelease);
    sheet->_defaultStyles = DFHashTableNew((DFCopyFunction)CSSStyleRetain,(DFFreeFunction)CSSStyleRelease);
    return sheet;
}

CSSSheet *CSSSheetRetain(CSSSheet *sheet)
{
    if (sheet != NULL)
        sheet->retainCount++;
    return sheet;
}

void CSSSheetRelease(CSSSheet *sheet)
{
    if ((sheet == NULL) || (--sheet->retainCount > 0))
        return;

    DFHashTableRelease(sheet->_styles);
    DFHashTableRelease(sheet->_defaultStyles);
    free(sheet);
}

const char **CSSSheetCopySelectors(CSSSheet *sheet)
{
    return DFHashTableCopyKeys(sheet->_styles);
}

void CSSSheetAddStyle(CSSSheet *sheet, CSSStyle *style)
{
    assert(style->selector != NULL);
    DFHashTableAdd(sheet->_styles,style->selector,style);
}

void CSSSheetRemoveStyle(CSSSheet *sheet, CSSStyle *style)
{
    DFHashTableRemove(sheet->_styles,style->selector);
}

CSSStyle *CSSSheetLookupSelector(CSSSheet *sheet, const char *selector, int add, int latent)
{
    CSSStyle *style = DFHashTableLookup(sheet->_styles,selector);
    if ((style == NULL) && add) {
        style = CSSStyleNew(selector);
        style->latent = latent;
        CSSSheetAddStyle(sheet,style);
        CSSStyleRelease(style); // The hash table keeps a reference, so this is safe
    }
    if ((style != NULL) && add && style->latent && !latent)
        style->latent = 0;
    return style;
}

CSSStyle *CSSSheetLookupElement(CSSSheet *sheet, const char *elementName, const char *className, int add, int latent)
{
    char *selector = CSSMakeSelector(elementName,className);
    CSSStyle *result = CSSSheetLookupSelector(sheet,selector,add,latent);
    free(selector);
    return result;
}

CSSStyle *CSSSheetFlattenedStyle(CSSSheet *sheet, CSSStyle *orig)
{
    // FIXME: Need tests for parent cycles
    CSSStyle *ancestor = orig;
    CSSStyle *result = CSSStyleNew("temp");
    DFHashTable *visited = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    const char **allSuffixes = NULL;
    while (1) {
        free(allSuffixes);
        allSuffixes = CSSStyleCopySuffixes(ancestor);
        for (int suffixIndex = 0; allSuffixes[suffixIndex]; suffixIndex++) {
            const char *suffix = allSuffixes[suffixIndex];
            CSSProperties *origProperties = CSSStyleRuleForSuffix(ancestor,suffix);
            CSSProperties *collapsedProperties = CSSStyleRuleForSuffix(result,suffix);

            const char **allNames = CSSPropertiesCopyNames(origProperties);
            for (int nameIndex = 0; allNames[nameIndex]; nameIndex++) {
                const char *name = allNames[nameIndex];
                if (!strcmp(name,"-uxwrite-default") && (ancestor != orig))
                    continue;
                if (CSSGet(collapsedProperties,name) == NULL)
                    CSSPut(collapsedProperties,name,CSSGet(origProperties,name));
            }
            free(allNames);
        }
        DFHashTableAdd(visited,ancestor->selector,"");
        ancestor = CSSSheetGetStyleParent(sheet,ancestor);
        if ((ancestor == NULL) || (DFHashTableLookup(visited,ancestor->selector) != NULL))
            break;
    }
    free(allSuffixes);
    DFHashTableRelease(visited);
    return result;
}

DFHashTable *CSSSheetRules(CSSSheet *sheet)
{
    DFHashTable *result = DFHashTableNew((DFCopyFunction)DFHashTableRetain,(DFFreeFunction)DFHashTableRelease);
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    for (int selIndex = 0; allSelectors[selIndex]; selIndex++) {
        const char *selector = allSelectors[selIndex];
        CSSStyle *origStyle = CSSSheetLookupSelector(sheet,selector,0,0);
        if ((origStyle == NULL) || origStyle->latent)
            continue;

        char *elementName = CSSSelectorCopyElementName(selector);
        char *className = CSSSelectorCopyClassName(selector);

        char *baseSelector;
        if (className != NULL) {
            char *escapedClassName = CSSEscapeIdent(className);
            baseSelector = DFFormatString("%s.%s",elementName,escapedClassName);
            free(escapedClassName);
        }
        else {
            baseSelector = strdup(elementName);
        }

        CSSStyle *flattenedStyle = CSSSheetFlattenedStyle(sheet,origStyle);
        const char **allSuffixes = CSSStyleCopySuffixes(flattenedStyle);
        for (int suffixIndex = 0; allSuffixes[suffixIndex]; suffixIndex++) {
            const char *suffix = allSuffixes[suffixIndex];
            char *fullSelector = DFFormatString("%s%s",baseSelector,suffix);
            CSSProperties *properties = CSSStyleRuleForSuffix(flattenedStyle,suffix);
            DFHashTable *collapsed = CSSCollapseProperties(properties);
            if (!((DFHashTableCount(collapsed) == 0) && ((strlen(suffix) > 0) || CSSIsBuiltinSelector(selector))))
                DFHashTableAdd(result,fullSelector,collapsed);
            free(fullSelector);
            DFHashTableRelease(collapsed);
        }
        free(allSuffixes);

        CSSStyleRelease(flattenedStyle);
        free(elementName);
        free(className);
        free(baseSelector);
    }
    free(allSelectors);
    return result;
}

char *CSSSheetCopyCSSText(CSSSheet *sheet)
{
    DFHashTable *hashRules = CSSSheetRules(sheet);
    char *cssText = CSSCopyStylesheetTextFromRules(hashRules);
    DFHashTableRelease(hashRules);
    return cssText;
}

char *CSSSheetCopyText(CSSSheet *sheet)
{
    DFBuffer *result = DFBufferNew();
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    DFSortStringsCaseInsensitive(allSelectors);
    for (int selIndex = 0; allSelectors[selIndex]; selIndex++) {
        CSSStyle *style = CSSSheetLookupSelector(sheet,allSelectors[selIndex],0,0);
        DFBufferFormat(result,"%s\n",style->selector);

        const char **sortedSuffixes = CSSStyleCopySuffixes(style);
        DFSortStringsCaseInsensitive(sortedSuffixes);
        for (int suffixIndex = 0; sortedSuffixes[suffixIndex]; suffixIndex++) {
            const char *suffix = sortedSuffixes[suffixIndex];
            char *quotedSuffix = DFQuote(suffix);
            DFBufferFormat(result,"    %s\n",quotedSuffix);
            free(quotedSuffix);
            CSSProperties *properties = CSSStyleRuleForSuffix(style,suffix);

            const char **sortedNames = CSSPropertiesCopyNames(properties);
            DFSortStringsCaseInsensitive(sortedNames);
            for (int nameIndex = 0; sortedNames[nameIndex]; nameIndex++) {
                const char *name = sortedNames[nameIndex];
                const char *value = CSSGet(properties,name);
                DFBufferFormat(result,"        %s = %s\n",name,value);
            }
            free(sortedNames);
        }
        free(sortedSuffixes);
    }
    free(allSelectors);
    char *str = strdup(result->data);
    DFBufferRelease(result);
    return str;
}

static void breakCycles(CSSSheet *sheet)
{
    // FIXME: Not covered by tests
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        CSSStyle *style = CSSSheetLookupSelector(sheet,selector,0,0);
        DFHashTable *visited = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
        int depth = 0;

        while (style != NULL) {
            if (DFHashTableLookup(visited,style->selector) != NULL) {
                CSSStyleSetParent(style,NULL);
                break;
            }
            DFHashTableAdd(visited,style->selector,"");
            style = CSSSheetGetStyleParent(sheet,style);
            depth++;
        }
        DFHashTableRelease(visited);
    }
    free(allSelectors);
}

static const char **reverseTopologicalSortedSelectors(CSSSheet *sheet)
{
    // FIXME: Not covered by tests
    DFArray *selectorsByDepth = DFArrayNew((DFCopyFunction)DFArrayRetain,(DFFreeFunction)DFArrayRelease);
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        CSSStyle *style = CSSSheetLookupSelector(sheet,selector,0,0);
        unsigned int depth = 0;

        while ((style = CSSSheetGetStyleParent(sheet,style)) != NULL)
            depth++;

        while (DFArrayCount(selectorsByDepth) < depth+1) {
            DFArray *array = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
            DFArrayAppend(selectorsByDepth,array);
            DFArrayRelease(array);
        }

        DFArray *selectorsAtCurrentDepth = DFArrayItemAt(selectorsByDepth,depth);
        DFArrayAppend(selectorsAtCurrentDepth,(char*)selector);
    }
    free(allSelectors);

    DFArray *sortedSelectors = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (size_t i = DFArrayCount(selectorsByDepth); i > 0; i--) {
        DFArray *atDepth = DFArrayItemAt(selectorsByDepth,i-1);
        for (size_t j = 0; j < DFArrayCount(atDepth); j++)
            DFArrayAppend(sortedSelectors,DFArrayItemAt(atDepth,j));
    }
    const char **result = DFStringArrayFlatten(sortedSelectors);
    DFArrayRelease(selectorsByDepth);
    DFArrayRelease(sortedSelectors);
    return result;
}

static void removeRedundantProperties(CSSSheet *sheet)
{
    // Remove any properties set on a style that have the same value as the corresponding property
    // on the parent style. This is necessary because CSS doesn't support style inheritance (in
    // the sense of Word & ODF's styles), so when we save out a HTML file, every style has all
    // properties of its ancestors. After reading in a HTML file for the purposes of updating the
    // original Word or ODF style, we don't want these extra property settings to remain, so that
    // we can avoid adding spurious extra redundant property settings to the original file.

    breakCycles(sheet);
    const char **sortedSelectors = reverseTopologicalSortedSelectors(sheet);

    for (size_t selIndex = 0; sortedSelectors[selIndex]; selIndex++) {
        const char *selector = sortedSelectors[selIndex];
        CSSStyle *child = CSSSheetLookupSelector(sheet,selector,0,0);
        CSSStyle *parent = CSSSheetGetStyleParent(sheet,child);
        if (parent == NULL)
            continue;
        const char **allSuffixes = CSSStyleCopySuffixes(child);
        for (int suffixIndex = 0; allSuffixes[suffixIndex]; suffixIndex++) {
            const char *suffix = allSuffixes[suffixIndex];
            int isCell = !strcmp(suffix," > * > tr > td");
            CSSProperties *childProperties = CSSStyleRuleForSuffix(child,suffix);
            CSSProperties *parentProperties = CSSStyleRuleForSuffix(parent,suffix);

            const char **allNames = CSSPropertiesCopyNames(childProperties);
            for (int nameIndex = 0; allNames[nameIndex]; nameIndex++) {
                const char *name = allNames[nameIndex];

                // In docx's styles.xml, the tblCellMar values in table styles are not inherited
                // (this seems like a bug in word, as isn't inconsistent with all other properties)
                // So keep these ones.
                if (isCell && DFStringHasPrefix(name,"padding-"))
                    continue;

                const char *childVal = CSSGet(childProperties,name);
                const char *parentVal = CSSGet(parentProperties,name);
                if ((childVal != NULL) && (parentVal != NULL) && DFStringEquals(childVal,parentVal))
                    CSSPut(childProperties,name,NULL);
            }
            free(allNames);
        }
        free(allSuffixes);
    }
    free(sortedSelectors);
}

static void updateFromRawCSSRules(CSSSheet *sheet, DFHashTable *rules)
{
    // FIXME: Handle class names containing escape sequences
    DFHashTableRelease(sheet->_styles);
    sheet->_styles = DFHashTableNew((DFCopyFunction)CSSStyleRetain,(DFFreeFunction)CSSStyleRelease);

    const char **sortedSelectors = DFHashTableCopyKeys(rules);
    DFSortStringsCaseInsensitive(sortedSelectors);
    for (int selIndex = 0; sortedSelectors[selIndex]; selIndex++) {
        const char *constSelector = sortedSelectors[selIndex];

        // Treat any selectors specifying the class name only as paragraph styles
        char *selector;
        if (!strncmp(constSelector,".",1))
            selector = DFFormatString("p%s",constSelector); // FIXME: Not covered by tests
        else
            selector = strdup(constSelector);

        DFHashTable *raw = DFHashTableLookup(rules,constSelector);
        char *baseId = NULL;
        char *suffix = NULL;
        CSSParseSelector(selector,&baseId,&suffix);

        CSSStyle *style = CSSSheetLookupSelector(sheet,baseId,0,0);
        if (style == NULL) {
            style = CSSStyleNew(baseId);
            CSSSheetAddStyle(sheet,style);
            CSSStyleRelease(style);
        }

        CSSProperties *properties = CSSStyleRuleForSuffix(style,suffix);
        CSSProperties *expanded = CSSPropertiesNewWithRaw(raw);

        const char **allNames = CSSPropertiesCopyNames(expanded);
        for (int nameIndex = 0; allNames[nameIndex]; nameIndex++) {
            const char *name = allNames[nameIndex];
            CSSPut(properties,name,CSSGet(expanded,name));
        }
        free(allNames);

        if (!strcmp(suffix,"")) {
            const char *defaultVal = CSSGet(properties,"-uxwrite-default");
            if ((defaultVal != NULL) && DFStringEqualsCI(defaultVal,"true"))
                CSSSheetSetDefaultStyle(sheet,style,StyleFamilyFromHTMLTag(style->tag));
        }
        CSSPropertiesRelease(expanded);
        free(baseId);
        free(suffix);
        free(selector);
    }
    free(sortedSelectors);
    removeRedundantProperties(sheet);
}

void CSSSheetUpdateFromCSSText(CSSSheet *sheet, const char *cssText)
{
    DFHashTable *rules = DFHashTableNew((DFCopyFunction)DFHashTableRetain,(DFFreeFunction)DFHashTableRelease);

    CSSParser *parser = CSSParserNew(cssText);
    DFHashTable *top = CSSParserRules(parser);
    CSSParserFree(parser);

    const char **allSelectorsText = DFHashTableCopyKeys(top);
    for (int i = 0; allSelectorsText[i]; i++) {
        const char *selectorsText = allSelectorsText[i];
        const char *propertiesText = DFHashTableLookup(top,selectorsText);

        parser = CSSParserNew(selectorsText);
        DFArray *selectors = CSSParserSelectors(parser);
        CSSParserFree(parser);
        if (selectors == NULL)
            continue;

        parser = CSSParserNew(propertiesText);
        DFHashTable *properties = CSSParserProperties(parser);
        CSSParserFree(parser);
        if (properties == NULL) {
            DFArrayRelease(selectors);
            continue;
        }

        for (size_t selIndex = 0; selIndex < DFArrayCount(selectors); selIndex++) {
            const char *selector = DFArrayItemAt(selectors,selIndex);
            DFHashTableAdd(rules,selector,properties);
        }

        DFHashTableRelease(properties);
        DFArrayRelease(selectors);
    }

    updateFromRawCSSRules(sheet,rules);
    free(allSelectorsText);
    DFHashTableRelease(top);
    DFHashTableRelease(rules);
}

CSSProperties *CSSSheetPageProperties(CSSSheet *sheet)
{
    return CSSStyleRule(CSSSheetLookupElement(sheet,"@page",NULL,1,0));
}

CSSProperties *CSSSheetBodyProperties(CSSSheet *sheet)
{
    return CSSStyleRule(CSSSheetLookupElement(sheet,"body",NULL,1,0));
}

CSSStyle *CSSSheetDefaultStyleForFamily(CSSSheet *sheet, StyleFamily family)
{
    return DFHashTableLookupInt(sheet->_defaultStyles,(int)family);
}

void CSSSheetSetDefaultStyle(CSSSheet *sheet, CSSStyle *style, StyleFamily family)
{
    if (style == NULL)
        DFHashTableRemoveInt(sheet->_defaultStyles,(int)family);
    else
        DFHashTableAddInt(sheet->_defaultStyles,(int)family,style);
}

int CSSSheetHeadingNumbering(CSSSheet *sheet)
{
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    for (int i = 0; allSelectors[i]; i++) {
        CSSStyle *style = CSSSheetLookupSelector(sheet,allSelectors[i],0,0);
        if (style->headingLevel == 0)
            continue;
        if (CSSGet(CSSStyleBefore(style),"content") == NULL)
            continue;
        DFArray *contentParts = CSSParseContent(CSSGet(CSSStyleBefore(style),"content"));
        for (size_t partIndex = 0; partIndex < DFArrayCount(contentParts); partIndex++) {
            ContentPart *part = DFArrayItemAt(contentParts,partIndex);
            if (part->type == ContentPartCounter) {
                free(allSelectors);
                DFArrayRelease(contentParts);
                return 1;
            }
        }
        DFArrayRelease(contentParts);
    }
    free(allSelectors);
    return 0;
}

typedef struct StyleList StyleList;

struct StyleList {
    CSSStyle *style;
    StyleList *next;
};

static DFHashTable *getStylesByHeadingLevel(CSSSheet *sheet)
{
    DFHashTable *stylesByHeadingLevel = DFHashTableNew(NULL,NULL);
    const char **allSelectors = CSSSheetCopySelectors(sheet);
    for (int i = 0; allSelectors[i]; i++) {
        CSSStyle *style = CSSSheetLookupSelector(sheet,allSelectors[i],0,0);
        if (style->headingLevel > 0) {
            int headingLevel = style->headingLevel;
            StyleList *item = (StyleList *)calloc(1,sizeof(StyleList));
            item->style = CSSStyleRetain(style);
            item->next = DFHashTableLookupInt(stylesByHeadingLevel,headingLevel);
            DFHashTableAddInt(stylesByHeadingLevel,headingLevel,item);
        }
    }
    free(allSelectors);
    return stylesByHeadingLevel;
}

static void enableNumberingForStyle(CSSStyle *style)
{
    int level = style->headingLevel;

    DFBuffer *reset = DFBufferNew();
    for (int after = level+1; after <= 6; after++) {
        if (reset->len == 0)
            DFBufferFormat(reset,"h%d",after);
        else
            DFBufferFormat(reset," h%d",after);
    }

    DFBuffer *content = DFBufferNew();
    for (int upto = 1; upto <= level; upto++) {
        if (content->len == 0)
            DFBufferFormat(content,"counter(h%d)",upto);
        else
            DFBufferFormat(content," \".\" counter(h%d)",upto);
    }
    DFBufferFormat(content," \" \"");

    CSSProperties *rule = CSSStyleRule(style);
    CSSPut(rule,"counter-increment",style->elementName);
    if (reset->len > 0)
        CSSPut(rule,"counter-reset",reset->data);
    CSSPut(CSSStyleBefore(style),"content",content->data);
    style->latent = 0;

    DFBufferRelease(reset);
    DFBufferRelease(content);
}

static void disableNumberingForStyle(CSSStyle *style, int explicitly)
{
    CSSProperties *rule = CSSStyleRule(style);
    CSSProperties *before = CSSStyleBefore(style);
    if (explicitly) {
        // FIXME: Not covered by tests
        char *increment = DFFormatString("h%d 0",style->headingLevel);
        CSSPut(rule,"counter-increment",increment);
        CSSPut(rule,"counter-reset","null");
        CSSPut(before,"content","\"\"");
        style->latent = 0;
        free(increment);
    }
    else {
        CSSPut(rule,"counter-increment",NULL);
        CSSPut(rule,"counter-reset",NULL);
        CSSPut(before,"content",NULL);
    }
}

// FIXME: This won't work now for Word documents, where styles always have class names
// FIXME: Not covered by tests
void CSSSheetSetHeadingNumbering(CSSSheet *sheet, int enabled)
{
    CSSStyle *defaultStyle = CSSSheetDefaultStyleForFamily(sheet,StyleFamilyParagraph);

    if (enabled) {
        DFHashTable *stylesByHeadingLevel = getStylesByHeadingLevel(sheet);
        for (int level = 1; level <= 6; level++) {

            StyleList *styles = DFHashTableLookupInt(stylesByHeadingLevel,level);
            int haveClassless = 0;

            for (StyleList *item = styles; item != NULL; item = item->next) {
                if (item->style->className == NULL)
                    haveClassless = 1;
            }

            // If there exists a style at level n which has no class name, then that is the parent of all other
            // heading styles. Set the numbering on that. Alternatively, if there no styles exist at level n, then
            // create a new one with no class name, and set the numbering information on that.
            if ((styles == NULL) || haveClassless) {
                char *elementName = DFFormatString("h%d",level);
                CSSStyle *style = CSSSheetLookupElement(sheet,elementName,NULL,1,0);
                enableNumberingForStyle(style);
                free(elementName);
            }
            else {
                // There are multiple heading styles at level n, all of which have a class name associated with them.
                // Set the numbering information on those which either have no parentId, or whose parentId is the
                // default paragraph style. This caters for situations like the "HeadingXUnnumbered" styles we used
                // to have, which were based on the corresponding "HeadingX" style.
                for (StyleList *item = styles; item != NULL; item = item->next) {
                    char *parentSelector = CSSStyleCopyParent(item->style);
                    if ((parentSelector == NULL) || !strcmp(parentSelector,defaultStyle->selector)) {
                        enableNumberingForStyle(item->style);
                    }
                    else {
                        disableNumberingForStyle(item->style,1);
                    }
                    free(parentSelector);
                }
            }

            StyleList *next;
            for (; styles != NULL; styles = next) {
                next = styles->next;
                CSSStyleRelease(styles->style);
                free(styles);
            }
        }
        DFHashTableRelease(stylesByHeadingLevel);
    }
    else {
        // Clear all numbering information on all heading styles
        const char **allSelectors = CSSSheetCopySelectors(sheet);
        for (int i = 0; allSelectors[i]; i++) {
            CSSStyle *style = CSSSheetLookupSelector(sheet,allSelectors[i],0,0);
            if (style->headingLevel > 0) {
                CSSPut(CSSStyleRule(style),"counter-increment",NULL);
                CSSPut(CSSStyleRule(style),"counter-reset",NULL);
                CSSPut(CSSStyleBefore(style),"content",NULL);
            }
        }
        free(allSelectors);
    }
}

// FIXME: This won't work now for Word documents, where styles always have class names
// FIXME: Not covered by tests
int CSSSheetIsNumberingUsed(CSSSheet *sheet)
{
    CSSStyle *style = CSSSheetLookupElement(sheet,"body",NULL,0,0);
    return (CSSGet(CSSStyleRule(style),"counter-reset") != NULL);
}

int CSSStyleIsNumbered(CSSStyle *style)
{
    const char *beforeContent = CSSGet(CSSStyleBefore(style),"content");
    return ((beforeContent != NULL) && !DFStringEquals(beforeContent,"\"\""));
}

CSSStyle *CSSSheetGetStyleParent(CSSSheet *sheet, CSSStyle *style)
{
    char *parentSelector = CSSStyleCopyParent(style);
    if (parentSelector == NULL)
        return NULL;;

    CSSStyle *parent = CSSSheetLookupSelector(sheet,parentSelector,0,0);
    free(parentSelector);
    return parent;
}

CSSStyle *CSSSheetParentOfStyle(CSSSheet *sheet, CSSStyle *style)
{
    // FIXME: Not covered by tests
    if (style == NULL)
        return NULL;;

    CSSStyle *parent = CSSSheetGetStyleParent(sheet,style);
    if (parent != NULL)
        return parent;

    if (CSSSelectorHasClassName(style->selector)) {
        char *elementName = CSSSelectorCopyElementName(style->selector);
        parent = CSSSheetLookupSelector(sheet,elementName,0,0);
        free(elementName);
        if (parent != NULL)
            return parent;
    }

    return NULL;
}

static int CSSIsBuiltinSelector2(Tag tag, const char *elementName, const char *className)
{
    switch (tag) {
        case HTML_P:
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_BLOCKQUOTE:
        case HTML_PRE:
        case HTML_BODY:
        case HTML_FIGURE:
        case HTML_FIGCAPTION:
        case HTML_TABLE:
        case HTML_CAPTION:
            return (className == NULL);
        case HTML_NAV:
            if ((className != NULL) &&
                (!strcmp(className,"tableofcontents") ||
                 !strcmp(className,"listoffigures") ||
                 !strcmp(className,"listoftables")))
                return 1;
            break;
        default:
            break;
    }

    if ((elementName != NULL) && !strcmp(elementName,"@page"))
        return 1;

    return 0;
}

int CSSIsBuiltinSelector(const char *selector)
{
    Tag tag = CSSSelectorGetTag(selector);
    char *elementName = CSSSelectorCopyElementName(selector);
    char *className = CSSSelectorCopyClassName(selector);
    int result = CSSIsBuiltinSelector2(tag,elementName,className);
    free(elementName);
    free(className);
    return result;
}
