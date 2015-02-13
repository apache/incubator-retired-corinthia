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

#include "CSSClassNames.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSStyle.h"
#include "CSSSheet.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFNameMap.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFHTML.h"
#include "WordStyles.h"
#include "Word.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

static const char *classPrefixForElementName(const char *elementName);

static void ensureStylesReferencedFromNode(DFNode *node, CSSSheet *styleSheet)
{
    switch (node->tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_FIGURE: {
            char *selector = CSSMakeNodeSelector(node);
            CSSSheetLookupSelector(styleSheet,selector,1,0);
            free(selector);
            break;
        }
        case HTML_A: {
            if (HTML_nodeIsHyperlink(node)) {
                CSSStyle *style = CSSSheetLookupElement(styleSheet,"span","Hyperlink",0,0);
                if (style == NULL) {
                    style = CSSSheetLookupElement(styleSheet,"span","Hyperlink",1,0);
                    CSSPut(CSSStyleRule(style),"color","#0000FF");
                    CSSSetUnderline(CSSStyleRule(style),1);

                    CSSStyle *parent = CSSSheetDefaultStyleForFamily(styleSheet,StyleFamilyCharacter);
                    if (parent != NULL)
                        CSSStyleSetParent(style,parent->selector);
                }
            }
            break;
        }
        case HTML_TABLE: {
            const char *className = DFGetAttribute(node,HTML_CLASS);
            if (className == NULL) {
                CSSStyle *dflt = CSSSheetDefaultStyleForFamily(styleSheet,StyleFamilyTable);
                int changed = 0;
                CSSStyle *grid = WordSetupTableGridStyle(styleSheet,&changed);
                DFSetAttribute(node,HTML_CLASS,grid->className);

                // Copy over cell padding rules. These are not automatically inherited in Word, and must be
                // explicitly duplicated for each descendant of the TableNormal style.

                // Under normal circumstances, when the user has added a table in the editor, the TableGrid
                // style will already be defined, with the appropriate padding rules set in the document's
                // stylesheet, and StyleSheet.removeRedundantProperties will make sure that each table style
                // keeps the padding properties. However if we are creating a HTML file that was not
                // oringally created with the intention of being converted to a word document, the TableGrid
                // style will not exist, and must be created. In this case (i.e. where changed is true) we
                // have to explicitly copy these properties over.

                if (changed) {
                    CSSPut(CSSStyleCell(grid),"padding-left",CSSGet(CSSStyleCell(dflt),"padding-left"));
                    CSSPut(CSSStyleCell(grid),"padding-right",CSSGet(CSSStyleCell(dflt),"padding-right"));
                    CSSPut(CSSStyleCell(grid),"padding-top",CSSGet(CSSStyleCell(dflt),"padding-top"));
                    CSSPut(CSSStyleCell(grid),"padding-bottom",CSSGet(CSSStyleCell(dflt),"padding-bottom"));
                }
            }
            break;
        }
    }
    for (DFNode *child = node->first; child != NULL; child = child->next)
        ensureStylesReferencedFromNode(child,styleSheet);
}

void CSSEnsureReferencedStylesPresent(DFDocument *htmlDoc, CSSSheet *styleSheet)
{
    CSSSheetLookupSelector(styleSheet,"p",1,0);
    ensureStylesReferencedFromNode(htmlDoc->root,styleSheet);
}

static void addHTMLDefaults(CSSStyle *style, CSSStyle *parent)
{
    if (style->headingLevel > 0) {
        const char *fontWeight = NULL;
        const char *fontSize = NULL;

        switch (style->tag) {
            case HTML_H1:
                fontWeight = "bold";
                fontSize = "24pt";
                break;
            case HTML_H2:
                fontWeight = "bold";
                fontSize = "18pt";
                break;
            case HTML_H3:
                fontWeight = "bold";
                fontSize = "14pt";
                break;
            case HTML_H4:
                fontWeight = "bold";
                fontSize = "12pt";
                break;
            case HTML_H5:
                fontWeight = "bold";
                fontSize = "10pt";
                break;
            case HTML_H6:
                fontWeight = "bold";
                fontSize = "8pt";
                break;
        }

        CSSProperties *properties = CSSStyleRule(style);
        if ((CSSGet(properties,"font-weight") == NULL) && ((parent == NULL) || (CSSGet(CSSStyleRule(parent),"font-weight") == NULL)))
            CSSPut(properties,"font-weight",fontWeight);
        if ((CSSGet(properties,"font-size") == NULL) && ((parent == NULL) || (CSSGet(CSSStyleRule(parent),"font-size") == NULL)))
            CSSPut(properties,"font-size",fontSize);
        // FIXME: This likely differs between languages
        // FIXME: Not covered by tests
        char *styleNext = CSSStyleCopyNext(style);
        char *parentNext = (parent != NULL) ? CSSStyleCopyNext(parent) : NULL;
        if ((styleNext == NULL) && ((parent == NULL) || (parentNext == NULL)))
            CSSStyleSetNext(style,"p.Normal");
        free(styleNext);
        free(parentNext);
    }
}

void CSSSetHTMLDefaults(CSSSheet *styleSheet)
{
    // For any heading elements that have an unnamed style declaration, set the default HTML properties on that.
    // All named style declarations inherit from these.

    // For any heading elements that have only named style declarations, set the default HTML properties on each
    // of them individually.

    for (int i = 1; i <= 6; i++) {
        char *elementName = DFFormatString("h%d",i);
        CSSStyle *style = CSSSheetLookupElement(styleSheet,elementName,NULL,0,0);
        if (style != NULL)
            addHTMLDefaults(style,NULL);
        free(elementName);
    }

    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];

        if (CSSSelectorIsHeading(selector) && (CSSSelectorHasClassName(selector))) {
            char *elementName = CSSSelectorCopyElementName(selector);

            CSSStyle *namedStyle = CSSSheetLookupSelector(styleSheet,selector,0,0);
            CSSStyle *unnamedStyle = CSSSheetLookupSelector(styleSheet,elementName,0,0);
            CSSStyle *flattened = NULL;
            CSSStyle *parent = CSSSheetGetStyleParent(styleSheet,namedStyle);
            if (parent != NULL)
                flattened = CSSSheetFlattenedStyle(styleSheet,parent);

            if (unnamedStyle == NULL)
                addHTMLDefaults(namedStyle,flattened);

            CSSStyleRelease(flattened);
            free(elementName);
        }
    }
    free(allSelectors);
}

static void setMissingDisplayNames(CSSSheet *styleSheet)
{
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,allSelectors[i],0,0);
        char *className = NULL;
        char *displayName = CSSStyleCopyDisplayName(style);
        if ((style->headingLevel > 0) && (displayName == NULL)) {
            const char *defaultClass = classPrefixForElementName(style->elementName);

            if ((style->className != NULL) && (defaultClass != NULL) && DFStringHasPrefix(style->className,defaultClass))
                className = DFSubstring(style->className,strlen(defaultClass),strlen(style->className));
            else
                className = DFStrDup(style->className);

            if ((className != NULL) && strlen(className) > 0)
                displayName = DFFormatString("Heading %d (%s)",style->headingLevel,className);
            else
                displayName = DFFormatString("heading %d",style->headingLevel);
            CSSStyleSetDisplayName(style,displayName);
        }
        free(displayName);
        free(className);
    }
    free(allSelectors);
}

static void setParentForHeadings(CSSSheet *styleSheet, DFHashTable *repls)
{
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        if (!CSSSelectorIsHeading(selector))
            continue;

        CSSStyle *style = CSSSheetLookupSelector(styleSheet,selector,0,0);
        const char *defaultClass = classPrefixForElementName(style->elementName);
        if (defaultClass == NULL)
            continue;

        CSSStyle *parentStyle = CSSSheetLookupElement(styleSheet,style->elementName,defaultClass,0,0);
        if ((parentStyle != NULL) && (parentStyle != style))
            CSSStyleSetParent(style,parentStyle->selector);
    }
    free(allSelectors);
}

static const char *classPrefixForElementName(const char *elementName)
{
    if (!strcmp(elementName,"h1"))
        return "heading_1";
    else if (!strcmp(elementName,"h2"))
        return "heading_2";
    else if (!strcmp(elementName,"h3"))
        return "heading_3";
    else if (!strcmp(elementName,"h4"))
        return "heading_4";
    else if (!strcmp(elementName,"h5"))
        return "heading_5";
    else if (!strcmp(elementName,"h6"))
        return "heading_6";
    else
        return elementName;
}

static void determineReplacements(CSSSheet *styleSheet, DFHashTable *repls)
{
    // char * -> linked list of SelectorList nodes
    DFHashTable *selectorsByClassName = DFHashTableNew(NULL,NULL);

    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        char *className = CSSSelectorCopyClassName(selector);
        if (className == NULL)
            className = strdup("");;

        SelectorList *item = (SelectorList *)calloc(1,sizeof(SelectorList));
        item->selector = strdup(selector);
        item->next = DFHashTableLookup(selectorsByClassName,className);
        DFHashTableAdd(selectorsByClassName,className,item);
        free(className);
    }
    free(allSelectors);

    const char **allClasses = DFHashTableCopyKeys(selectorsByClassName);
    for (int i = 0; allClasses[i]; i++) {
        const char *className = allClasses[i];
        SelectorList *list = DFHashTableLookup(selectorsByClassName,className);
        SelectorList *start = list;
        SelectorList *next;
        for (; list != NULL; list = next) {
            next = list->next;
            const char *oldSelector = list->selector;
            char *elementName = CSSSelectorCopyElementName(oldSelector);
            int haveUnnamed = (CSSSheetLookupSelector(styleSheet,elementName,0,0) != NULL);
            if (((start != NULL) && (start->next != NULL)) || !strcmp(className,"") || haveUnnamed) {
                if (CSSSelectorIsHeading(oldSelector)) {
                    const char *prefix = classPrefixForElementName(elementName);
                    char *newSelector = DFFormatString("%s.%s%s",elementName,prefix,className);
                    DFHashTableAdd(repls,oldSelector,newSelector);
                    free(newSelector);
                }
            }
            free(elementName);
        }

        // Free selector list
        for (list = start; list != NULL; list = next) {
            next = list->next;
            free(list->selector);
            free(list);
        }
    }
    free(allClasses);
    DFHashTableRelease(selectorsByClassName);

    // Remove any "double replacements", where the new selector is itself a key in the hash table, e.g.:
    // h1 -> h1.Heading1 -> h1.Heading1Heading1
    //
    // This avoids a problem where a style is renamed twice, but the relevant nodes in the DOM tree are only updated
    // to refer to the first replacement, not the second, causing an assertion failure in WordParagraphPut, which
    // expects there to be a style for every selector it encounters during traversal.
    const char **allOldSelectors = DFHashTableCopyKeys(repls);
    for (int i = 0; allOldSelectors[i]; i++) {
        const char *newSelector = DFHashTableLookup(repls,allOldSelectors[i]);
        if (newSelector != NULL) // may have already been removed
            DFHashTableRemove(repls,newSelector);
    }
    free(allOldSelectors);
}

static void replaceSelectorsInNode(DFHashTable *repls, DFNode *node)
{
    switch (node->tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6: {
            const char *className = DFGetAttribute(node,HTML_CLASS);
            char *oldSelector = CSSMakeTagSelector(node->tag,className);
            const char *newSelector = DFHashTableLookup(repls,oldSelector);

            if (newSelector != NULL) {
                char *className = CSSSelectorCopyClassName(newSelector);
                DFSetAttribute(node,HTML_CLASS,className);
                free(className);
            }

            free(oldSelector);
            break;
        }
    }
    for (DFNode *child = node->first; child != NULL; child = child->next)
        replaceSelectorsInNode(repls,child);
}

static void replaceSelectorsInSheet(DFHashTable *repls, CSSSheet *styleSheet)
{
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *oldSelector = allSelectors[i];
        const char *newSelector = DFHashTableLookup(repls,oldSelector);
        if ((newSelector == NULL) || !strcmp(newSelector,oldSelector))
            continue;

        CSSStyle *style = CSSSheetLookupSelector(styleSheet,oldSelector,0,0);
        CSSStyle *existing = CSSSheetLookupSelector(styleSheet,newSelector,0,0);

        if (existing != NULL) {
            CSSSheetRemoveStyle(styleSheet,style);
        }
        else {
            // We must retain a reference to the style here, otherwise it will be deleted when removed from the sheet
            CSSStyleRetain(style);
            CSSSheetRemoveStyle(styleSheet,style);
            CSSStyleSetSelector(style,newSelector);
            CSSSheetAddStyle(styleSheet,style);
            CSSStyleRelease(style);
        }
    }
    free(allSelectors);

    // FIXME: Not covered by tests
    allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,allSelectors[i],0,0);

        char *parentSelector = CSSStyleCopyParent(style);
        if ((parentSelector != NULL) && (DFHashTableLookup(repls,parentSelector) != NULL))
            CSSStyleSetParent(style,DFHashTableLookup(repls,parentSelector));
        free(parentSelector);

        char *nextSelector = CSSStyleCopyNext(style);
        if ((nextSelector != NULL) && (DFHashTableLookup(repls,nextSelector) != NULL))
            CSSStyleSetNext(style,DFHashTableLookup(repls,nextSelector));
        free(nextSelector);
    }
    free(allSelectors);
}

void CSSEnsureUnique(CSSSheet *styleSheet, DFDocument *htmlDoc, int creating)
{
    DFHashTable *repls = DFHashTableNew((DFCopyFunction)strdup,free);
    determineReplacements(styleSheet,repls);
    replaceSelectorsInSheet(repls,styleSheet);
    replaceSelectorsInNode(repls,htmlDoc->root);
    setMissingDisplayNames(styleSheet);
    setParentForHeadings(styleSheet,repls);
    DFHashTableRelease(repls);
}
