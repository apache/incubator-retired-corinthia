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
#include "WordLenses.h"
#include "WordSheet.h"
#include "WordStyles.h"
#include "WordNumbering.h"
#include "WordLists.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSLength.h"
#include "CSSSelector.h"
#include "CSSStyle.h"
#include "CSSSheet.h"
#include "DFHTML.h"
#include "DFNameMap.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordParagraphLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordParagraphCreateAbstractNode(WordGetData *get, DFNode *concrete)
{
    DFNode *pPr = DFChildWithTag(concrete,WORD_PPR);
    const char *styleId = NULL;
    CSSProperties *properties = CSSPropertiesNew();
    if (pPr != NULL)
        WordGetPPr(pPr,properties,&styleId,get->conv->mainSection);;

    const char *selector = WordSheetSelectorForStyleId(get->conv->styles,"paragraph",styleId);

    // FIXME: We should only be labelling paragraphs as figures when they contain what we recognise
    // as an actual valid figure. Currently this (incorrectly) identifies embedded objects such as
    // spreadsheets as figures, when really it should just put them in a normal paragraph
    // See nullupdate-embed01.test and nullupdate-embed02.test
    if (Word_isFigureParagraph(concrete))
        selector = "figure";;

    Tag elementName = (selector == NULL) ? HTML_P : CSSSelectorGetTag(selector);
    char *className = (selector == NULL) ? NULL : CSSSelectorCopyClassName(selector);
    DFNode *abstract = WordConverterCreateAbstract(get,elementName,concrete);

    DFHashTable *propertiesHash = CSSCollapseProperties(properties);
    char *styleValue = CSSSerializeProperties(propertiesHash);
    DFHashTableRelease(propertiesHash);
    if (strlen(styleValue) > 0)
        DFSetAttribute(abstract,HTML_STYLE,styleValue);
    free(styleValue);

    DFNode *numPr = DFChildWithTag(pPr,WORD_NUMPR);
    if (numPr != NULL) {
        const char *ilvl = NULL;
        const char *numId = NULL;
        for (DFNode *nchild = numPr->first; nchild != NULL; nchild = nchild->next) {
            switch (nchild->tag) {
                case WORD_ILVL:
                    ilvl = DFGetAttribute(nchild,WORD_VAL);
                    break;
                case WORD_NUMID:
                    numId = DFGetAttribute(nchild,WORD_VAL);
                    break;
            }
        }

        if ((ilvl != NULL) && (numId != NULL)) {
            // We have a list item. Now according to the rules of HTML, this must go inside
            // a UL or an OL element. However, since word uses a "flat" model of lists, to
            // the bidirectional transformation step simple, we represent LI elements as direct
            // children of the body (or other container). These are converted to the nested
            // model of HTML in a postprocessing stage (and vice versa on update)
            DFSetAttribute(abstract,WORD_ILVL,ilvl);
            DFSetAttribute(abstract,WORD_NUMID,numId);
        }
    }

    if (className != NULL) {
        DFSetAttribute(abstract,HTML_CLASS,className);
    }
    else if (elementName == HTML_P) {
        CSSStyle *defaultStyle = CSSSheetDefaultStyleForFamily(get->conv->styleSheet,StyleFamilyParagraph);
        if (defaultStyle != NULL)
            DFSetAttribute(abstract,HTML_CLASS,defaultStyle->className);
    }

    free(className);
    CSSPropertiesRelease(properties);
    return abstract;
}

static DFNode *WordParagraphGet(WordGetData *get, DFNode *concrete)
{
    DFNode *abstract = WordParagraphCreateAbstractNode(get,concrete);
    WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);

    // If the paragraph is devoid of text, add a BR element so it takes up a line of vertical space
    if (!HTML_nodeHasContent(abstract))
        DFAppendChild(abstract,WordConverterCreateAbstract(get,HTML_BR,NULL));

    return abstract;
}

static void WordParagraphPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    if (!HTML_isParagraphTag(abstract->tag) && (abstract->tag != HTML_FIGURE))
        return;

    if (concrete->tag != WORD_P)
        return;

    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);

    const char *htmlId = DFGetAttribute(abstract,CONV_LISTNUM);
    const char *ilvl = DFGetAttribute(abstract,CONV_ILVL);
    int isListItem = (DFGetAttribute(abstract,CONV_LISTITEM) != NULL);
    const char *numId = NULL;
    if (htmlId != NULL) {
        numId = DFHashTableLookup(put->numIdByHtmlId,htmlId);
        if (numId == NULL) {
            WordAbstractNum *abstractNum;
            const char *type = DFGetAttribute(abstract,CONV_LISTTYPE);
            abstractNum = WordNumberingCreateAbstractNumWithType(put->conv->numbering,type);
            WordConcreteNum *concrete = WordNumberingAddConcreteWithAbstract(put->conv->numbering,abstractNum);
            numId = concrete->numId;

            assert(DFHashTableLookup(put->numIdByHtmlId,htmlId) == NULL);
            assert(DFHashTableLookup(put->htmlIdByNumId,numId) == NULL);

            DFHashTableAdd(put->numIdByHtmlId,htmlId,numId);
            DFHashTableAdd(put->htmlIdByNumId,numId,htmlId);
        }

        // Get numbering level properties
        CSSProperties *levelProperties = CSSPropertiesNew();
        WordConcreteNum *concreteNum = WordNumberingConcreteWithId(put->conv->numbering,numId);
        WordNumLevel *level = WordConcreteNumGetLevel(concreteNum,atoi(ilvl));
        DFNode *pPr = DFChildWithTag(level->element,WORD_PPR);
        if (pPr != NULL) {
            const char *numLevelStyleId = NULL;
            WordGetPPr(pPr,levelProperties,&numLevelStyleId,put->conv->mainSection);
        }

        // Get paragraph properties
        const char *cssText = DFGetAttribute(abstract,HTML_STYLE);
        CSSProperties *paragraphProperties = CSSPropertiesNewWithString(cssText);

        // If both the level and paragraph have a margin-left property set, add the level's
        // margin-left to the paragraph's, since word treats the corresponding indentation property
        // as being absolute, not relative to the numbering level's indentation
        CSSLength levelMarginLeft = CSSLengthFromString(CSSGet(levelProperties,"margin-left"));
        CSSLength paragraphMarginLeft = CSSLengthFromString(CSSGet(paragraphProperties,"margin-left"));
        if (CSSLengthIsValid(levelMarginLeft) && (levelMarginLeft.units == UnitsPct) &&
            CSSLengthIsValid(paragraphMarginLeft) && (paragraphMarginLeft.units == UnitsPct)) {
            double newMarginLeft = levelMarginLeft.value + paragraphMarginLeft.value;
            char buf[100];
            CSSPut(paragraphProperties,"margin-left",DFFormatDoublePct(buf,100,newMarginLeft));
            char *propertiesText = CSSPropertiesCopyDescription(paragraphProperties);
            DFSetAttribute(abstract,HTML_STYLE,propertiesText);
            free(propertiesText);
        }

        CSSPropertiesRelease(levelProperties);
        CSSPropertiesRelease(paragraphProperties);
    }

    DFNode *pPr = DFChildWithTag(concrete,WORD_PPR);
    if (pPr == NULL)
        pPr = DFCreateElement(put->contentDoc,WORD_PPR);
    DFInsertBefore(concrete,pPr,concrete->first); // Ensure first, in case BDTContainerPut moved it

    char *selector = CSSMakeNodeSelector(abstract);
    const char *className = DFGetAttribute(abstract,HTML_CLASS);

    // FIXME: Won't work with Word documents created in non-English languages
    if ((selector != NULL) && (abstract->tag == HTML_P) && DFStringEquals(className,"Caption")) {
        free(selector);
        selector = strdup("caption");
    }

    if (selector != NULL) {
        CSSStyle *defaultStyle = CSSSheetDefaultStyleForFamily(put->conv->styleSheet,StyleFamilyParagraph);
        if ((defaultStyle != NULL) && DFStringEquals(defaultStyle->selector,selector)) {
            free(selector);
            selector = NULL;
        }
    }

    // Check that style exists in the CSS stylesheet
    // This should already be guaranteed by CSSEnsureReferencedStylesPresent
    if (selector != NULL) {
        CSSStyle *existing = CSSSheetLookupSelector(put->conv->styleSheet,selector,0,0);
        assert(existing != NULL);
    }

    const char *inlineCSSText = DFGetAttribute(abstract,HTML_STYLE);
    CSSProperties *properties = CSSPropertiesNewWithString(inlineCSSText);

    if ((numId != NULL) && (ilvl != NULL)) {
        if (isListItem) {
            CSSPut(properties,"-word-numId",numId);
            CSSPut(properties,"-word-ilvl",ilvl);
        }
        else {
            double indent = listDesiredIndent(put->conv,numId,ilvl);
            if (indent > 0) {
                char buf[100];
                CSSPut(properties,"margin-left",DFFormatDoublePct(buf,100,indent));
            }
        }
    }

    const char *styleId = NULL;
    if ((selector != NULL) && !DFStringEquals(selector,"p"))
        styleId = WordSheetStyleIdForSelector(put->conv->styles,selector);

    WordPutPPr(pPr,properties,styleId,put->conv->mainSection,-1);

    if (pPr->first == NULL)
        DFRemoveNode(pPr);

    free(selector);
    CSSPropertiesRelease(properties);
}

static DFNode *WordParagraphCreate(WordPutData *put, DFNode *abstract)
{
    DFNode *concrete = DFCreateElement(put->contentDoc,WORD_P);
    WordParagraphPut(put,abstract,concrete);
    return concrete;
}

static int WordParagraphIsVisible(WordPutData *put, DFNode *concrete)
{
    return 1;
}

static void WordParagraphRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordParagraphLens = {
    .isVisible = WordParagraphIsVisible,
    .get = WordParagraphGet,
    .put = WordParagraphPut,
    .create = WordParagraphCreate,
    .remove = WordParagraphRemove,
};
