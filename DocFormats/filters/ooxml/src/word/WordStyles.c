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
#include "WordStyles.h"
#include "DFDOM.h"
#include "DFHTML.h"
#include "DFXMLNames.h"
#include "CSS.h"
#include "CSSLength.h"
#include "CSSProperties.h"
#include "WordNumbering.h"
#include "WordTheme.h"
#include "WordSection.h"
#include "WordSheet.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static Tag WordSectPr_Children[] = {
    WORD_FOOTNOTEPR,
    WORD_ENDNOTEPR,
    WORD_TYPE,
    WORD_PGSZ,
    WORD_PGMAR,
    WORD_PAPERSRC,
    WORD_PGBORDERS,
    WORD_LNNUMTYPE,
    WORD_PGNUMTYPE,
    WORD_COLS,
    WORD_FORMPROT,
    WORD_VALIGN,
    WORD_NOENDNOTE,
    WORD_TITLEPG,
    WORD_TEXTDIRECTION,
    WORD_BIDI,
    WORD_RTLGUTTER,
    WORD_DOCGRID,
    WORD_PRINTERSETTINGS,
    WORD_SECTPRCHANGE,
    0,
};

static Tag WordStyle_Children[] = {
    WORD_NAME,
    WORD_ALIASES,
    WORD_BASEDON,
    WORD_NEXT,
    WORD_LINK,
    WORD_AUTOREDEFINE,
    WORD_HIDDEN,
    WORD_UIPRIORITY,
    WORD_SEMIHIDDEN,
    WORD_UNHIDEWHENUSED,
    WORD_QFORMAT,
    WORD_LOCKED,
    WORD_PERSONAL,
    WORD_PERSONALCOMPOSE,
    WORD_PERSONALREPLY,
    WORD_RSID,
    WORD_PPR,
    WORD_RPR,
    WORD_TBLPR,
    WORD_TRPR,
    WORD_TCPR,
    WORD_TBLSTYLEPR,
    0,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                           Style definitions (elements in styles.xml)                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void WordGetTblStylePr(DFNode *concrete, CSSStyle *style, WordSection *section, WordTheme *theme)
{
    const char *tableComponent = DFGetAttribute(concrete,WORD_TYPE);
    if (tableComponent == NULL)
        return;;
    CSSProperties *rule = CSSStyleRuleForTableComponent(style,tableComponent);
    if (rule == NULL)
        return;
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        const char *styleId = NULL;
        switch (child->tag) {
            case WORD_RPR:
                WordGetRPr(child,rule,&styleId,theme);
                break;
            case WORD_PPR:
                WordGetPPr(child,rule,&styleId,section);
                break;
            case WORD_TBLPR:
                WordGetTblPr(child,rule,NULL,section,&styleId);
                break;
            case WORD_TRPR:
                // FIXME
                break;
            case WORD_TCPR:
                WordGetTcPr(child,rule);
                break;
        }
    }
}

static void WordGetStyle(DFNode *concrete, CSSStyle *style, WordConverter *converter)
{
    WordSection *section = converter->mainSection;
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        const char *styleId = NULL;
        switch (child->tag) {
            case WORD_NAME:
                CSSStyleSetDisplayName(style,DFGetAttribute(child,WORD_VAL));
                break;
            case WORD_NEXT: {
                // FIXME: do he need to handle style types other than paragraph here?
                const char *nextName = DFGetAttribute(child,WORD_VAL);
                if (nextName == NULL)
                    continue;
                WordStyle *nextStyle = WordSheetStyleForTypeId(converter->styles,"paragraph",nextName);
                if (nextStyle == NULL)
                    continue;
                CSSStyleSetNext(style,nextStyle->selector);
                break;
            }
            case WORD_RPR:
                WordGetRPr(child,CSSStyleRule(style),&styleId,converter->theme);
                break;
            case WORD_PPR: {
                WordGetPPr(child,CSSStyleRule(style),&styleId,section);
                if (style->headingLevel > 0) {
                    DFNode *numPr = DFChildWithTag(child,WORD_NUMPR);
                    if (numPr != NULL)
                        WordGetNumPrStyle(numPr,style,converter);
                }
                break;
            }
            case WORD_TBLPR:
                WordGetTblPr(child,CSSStyleRule(style),CSSStyleCell(style),section,&styleId);
                break;
            case WORD_TRPR:
                // FIXME
                break;
            case WORD_TCPR:
                WordGetTcPr(child,CSSStyleRule(style));
                break;
            case WORD_TBLSTYLEPR:
                WordGetTblStylePr(child,style,section,converter->theme);
                break;
        }
    }

    // Special case: The ListParagraph style that word automatically adds specifies an indentation
    // of 36pt. We don't actually want this, because HTML automatically indents lists anyway. If
    // we see this, clear it, but keep the old value around for when we update the word document.
    StyleFamily family = WordStyleFamilyForSelector(style->selector);
    const char *name = WordSheetStyleIdForSelector(converter->styles,style->selector);
    if ((family == StyleFamilyParagraph) && DFStringEquals(name,"ListParagraph")) {
        CSSProperties *properties = CSSStyleRule(style);
        const char *wordMarginLeft = CSSGet(properties,"margin-left");
        CSSPut(properties,"-word-margin-left",wordMarginLeft);
        CSSPut(properties,"margin-left",NULL);
    }

    DFNode *pPr = DFChildWithTag(concrete,WORD_PPR);
    DFNode *numPr = DFChildWithTag(pPr,WORD_NUMPR);
    const char *numId = DFGetChildAttribute(numPr,WORD_NUMID,WORD_VAL);
    if ((numId != NULL) && (atoi(numId) == 0)) {
        switch (style->tag) {
            case HTML_H1:
            case HTML_H2:
            case HTML_H3:
            case HTML_H4:
            case HTML_H5:
            case HTML_H6:
            case HTML_FIGURE:
            case HTML_TABLE: {
                char *counterIncrement = DFFormatString("%s 0",style->elementName);
                CSSPut(CSSStyleRule(style),"counter-reset","null");
                CSSPut(CSSStyleRule(style),"counter-increment",counterIncrement);
                CSSPut(CSSStyleBefore(style),"content","none");
                free(counterIncrement);
            }
        }
    }
}

static void WordPutStyle(DFNode *concrete, CSSStyle *style, WordConverter *converter)
{
    // If we find a style with a counter-increment value of "<elementname> 0", this means that it's
    // an explicitly unnumbered paragraph. So we set the numbering id to 0, so word doesn't increment
    // the corresponding counter when encountering this style
    if (CSSGet(CSSStyleRule(style),"counter-increment") != NULL) {
        char *zeroIncrement = DFFormatString("%s 0",style->elementName);
        if (DFStringEquals(CSSGet(CSSStyleRule(style),"counter-increment"),zeroIncrement))
            CSSPut(CSSStyleRule(style),"-word-numId","0");
        free(zeroIncrement);
    }

    int outlineLvl = -1;
    if ((style->headingLevel >= 1) && (style->headingLevel <= 6)) {
        outlineLvl = style->headingLevel-1;
        char *nextSelector = CSSStyleCopyNext(style);
        if (nextSelector == NULL) {
            CSSStyle *def = CSSSheetDefaultStyleForFamily(converter->styleSheet,StyleFamilyParagraph);
            if (def != NULL)
                CSSStyleSetNext(style,def->selector);
        }
        free(nextSelector);
    }

    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    char *parentSelector = CSSStyleCopyParent(style);
    if (parentSelector == NULL) {
        if (style->className != NULL) {
            if ((style->tag != HTML_P) && (style->tag != HTML_SPAN) && (style->tag != HTML_TABLE)) {
                CSSStyle *parentStyle = CSSSheetLookupElement(converter->styleSheet,style->elementName,NULL,0,0);
                if ((parentStyle != NULL) && !parentStyle->latent)
                    parentSelector = strdup(style->elementName);
            }
        }
    }

    // Based on
    const char *basedOn = WordSheetStyleIdForSelector(converter->styles,parentSelector);
    if (basedOn == NULL) {
        children[WORD_BASEDON] = NULL;
    }
    else {
        children[WORD_BASEDON] = DFCreateElement(concrete->doc,WORD_BASEDON);
        DFSetAttribute(children[WORD_BASEDON],WORD_VAL,basedOn);
    }

    // Style for next paragraph
    children[WORD_NEXT] = NULL;
    char *nextSelector = CSSStyleCopyNext(style);
    if (nextSelector != NULL) {
        StyleFamily thisFamily = WordStyleFamilyForSelector(style->selector);
        StyleFamily nextFamily = WordStyleFamilyForSelector(nextSelector);
        if (nextFamily == thisFamily) {
            children[WORD_NEXT] = DFCreateElement(concrete->doc,WORD_NEXT);
            const char *nextStyleId = WordSheetStyleIdForSelector(converter->styles,nextSelector);
            DFSetAttribute(children[WORD_NEXT],WORD_VAL,nextStyleId);
        }
    }

    if (WordStyleFamilyForSelector(style->selector) == StyleFamilyTable) {
        // Table properties
        if (children[WORD_TBLPR] == NULL)
            children[WORD_TBLPR] = DFCreateElement(concrete->doc,WORD_TBLPR);;
        const char *oldJc = DFGetChildAttribute(children[WORD_TBLPR],WORD_JC,WORD_VAL);
        CSSProperties *wholeTable = CSSStyleRuleForSuffix(style,DFTableSuffixWholeTable);
        WordPutTblPr(children[WORD_TBLPR],wholeTable,CSSStyleCell(style),converter->mainSection,NULL);
        const char *newJc = DFGetChildAttribute(children[WORD_TBLPR],WORD_JC,WORD_VAL);
        if (children[WORD_TBLPR]->first == NULL)
            children[WORD_TBLPR] = NULL;

        if (children[WORD_TRPR] == NULL)
            children[WORD_TRPR] = DFCreateElement(concrete->doc,WORD_TRPR);
        WordPutTrPr(children[WORD_TRPR],oldJc,newJc);
        if (children[WORD_TRPR]->first == NULL)
            children[WORD_TRPR] = NULL;
    }
    else {
        // Paragraph properties
        if (children[WORD_PPR] == NULL)
            children[WORD_PPR] = DFCreateElement(concrete->doc,WORD_PPR);
        WordPutPPr(children[WORD_PPR],CSSStyleRule(style),NULL,converter->mainSection,outlineLvl);
        if (children[WORD_PPR]->first == NULL)
            children[WORD_PPR] = NULL;

        // Run properties
        if (children[WORD_RPR] == NULL)
            children[WORD_RPR] = DFCreateElement(concrete->doc,WORD_RPR);
        WordPutRPr(children[WORD_RPR],CSSStyleRule(style),NULL,converter->theme);
        if (children[WORD_RPR]->first == NULL)
            children[WORD_RPR] = NULL;
    }

    replaceChildrenFromArray(concrete,children,WordStyle_Children);
    free(parentSelector);
    free(nextSelector);
}

static void WordGetSectPr(DFNode *concrete, CSSProperties *body, CSSProperties *page, WordSection *section)
{
    DFNode *pgSz = NULL;
    DFNode *pgMar = NULL;
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_PGSZ:
                pgSz = child;
                break;
            case WORD_PGMAR:
                pgMar = child;
                break;
        }
    }

    if ((pgSz != NULL) && (pgMar != NULL)) {
        const char *widthStr = DFGetAttribute(pgSz,WORD_W);
        const char *heightStr = DFGetAttribute(pgSz,WORD_H);

        if (widthStr != NULL)
            section->pageWidth = atoi(widthStr);
        if (heightStr != NULL)
            section->pageHeight = atoi(heightStr);

        if ((widthStr != NULL) && (heightStr != NULL)) {
            const char *leftStr = DFGetAttribute(pgMar,WORD_LEFT);
            const char *rightStr = DFGetAttribute(pgMar,WORD_RIGHT);
            const char *topStr = DFGetAttribute(pgMar,WORD_TOP);
            const char *bottomStr = DFGetAttribute(pgMar,WORD_BOTTOM);

            // In CSS, margins (both horizontal and vertical) are measured relative to the width of
            // the containing block, when expressed as percentages

            double width = atof(widthStr);

            if (leftStr != NULL) {
                double leftPct = 100.0*atof(leftStr)/width;
                char buf[100];
                CSSPut(body,"margin-left",DFFormatDoublePct(buf,100,leftPct));
                section->leftMargin = atoi(leftStr);
            }

            if (rightStr != NULL) {
                double rightPct = 100.0*atof(rightStr)/width;
                char buf[100];
                CSSPut(body,"margin-right",DFFormatDoublePct(buf,100,rightPct));
                section->rightMargin = atoi(rightStr);
            }

            if (topStr != NULL) {
                double topPct = 100.0*atof(topStr)/width;
                char buf[100];
                CSSPut(body,"margin-top",DFFormatDoublePct(buf,100,topPct));
                section->topMargin = atoi(topStr);
            }

            if (bottomStr != NULL) {
                double bottomPct = 100.0*atof(bottomStr)/width;
                char buf[100];
                CSSPut(body,"margin-bottom",DFFormatDoublePct(buf,100,bottomPct));
                section->bottomMargin = atoi(bottomStr);
            }

            // A "twip" is a twentieth of a point
            int widthTwips = atoi(widthStr);
            int heightTwips = atoi(heightStr);

            if ((widthTwips == A4_WIDTH_TWIPS) && (heightTwips == A4_HEIGHT_TWIPS))
                CSSPut(page,"size","A4 portrait");
            else if ((widthTwips == A4_HEIGHT_TWIPS) && (heightTwips == A4_WIDTH_TWIPS))
                CSSPut(page,"size","A4 landscape");
            else if ((widthTwips == LETTER_WIDTH_TWIPS) && (heightTwips == LETTER_HEIGHT_TWIPS))
                CSSPut(page,"size","letter portrait");
            else if ((widthTwips == LETTER_HEIGHT_TWIPS) && (heightTwips == LETTER_WIDTH_TWIPS))
                CSSPut(page,"size","letter landscape");
        }
    }
}

static void WordPutSectPr(DFNode *concrete, CSSSheet *styleSheet, WordSection *section)
{
    // Note: The sectPr element can potentially contain one or more headerReference or
    // footerReference elements at the start, before the elements in WordSectPr_Children.
    // So the straight childrenToArray/replaceChildrenFromArray method used elsewhere doesn't
    // work here - we need to make sure these other elements are maintained

    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);



    CSSProperties *oldBody = CSSPropertiesNew();
    CSSProperties *oldPage = CSSPropertiesNew();
    WordGetSectPr(concrete,oldBody,oldPage,section);

    CSSProperties *newBody = CSSSheetBodyProperties(styleSheet);
    CSSProperties *newPage = CSSSheetPageProperties(styleSheet);


    // Page size
    if (children[WORD_PGSZ] == NULL)
        children[WORD_PGSZ] = DFCreateElement(concrete->doc,WORD_PGSZ);

    int updatePageSize = 0;
    const char *widthStr = DFGetAttribute(children[WORD_PGSZ],WORD_W);
    const char *heightStr = DFGetAttribute(children[WORD_PGSZ],WORD_H);
    int widthTwips;
    int heightTwips;
    if ((widthStr == NULL) || (atoi(widthStr) <= 0) ||
        (heightStr == NULL) || (atoi(heightStr) <= 0)) {
        // Invalid or missing page size: Set to A4 portrait
        widthTwips = A4_WIDTH_TWIPS;
        heightTwips = A4_HEIGHT_TWIPS;
        updatePageSize = 1;
    }
    else {
        widthTwips = atoi(widthStr);
        heightTwips = atoi(heightStr);
    }

    if (!DFStringEquals(CSSGet(oldPage,"size"),CSSGet(newPage,"size"))) {
        const char *newSize = CSSGet(newPage,"size");
        if (DFStringEqualsCI(newSize,"A4 portrait")) {
            widthTwips = A4_WIDTH_TWIPS;
            heightTwips = A4_HEIGHT_TWIPS;
        }
        else if (DFStringEqualsCI(newSize,"A4 landscape")) {
            widthTwips = A4_HEIGHT_TWIPS;
            heightTwips = A4_WIDTH_TWIPS;
        }
        else if (DFStringEqualsCI(newSize,"letter portrait")) {
            widthTwips = LETTER_WIDTH_TWIPS;
            heightTwips = LETTER_HEIGHT_TWIPS;
        }
        else if (DFStringEqualsCI(newSize,"letter landscape")) {
            widthTwips = LETTER_HEIGHT_TWIPS;
            heightTwips = LETTER_WIDTH_TWIPS;
        }
        else {
            widthTwips = A4_WIDTH_TWIPS;
            heightTwips = A4_HEIGHT_TWIPS;
        }
        updatePageSize = 1;
    }

    if (updatePageSize) {
        DFFormatAttribute(children[WORD_PGSZ],WORD_W,"%d",widthTwips);
        DFFormatAttribute(children[WORD_PGSZ],WORD_H,"%d",heightTwips);

        if (widthTwips > heightTwips)
            DFSetAttribute(children[WORD_PGSZ],WORD_ORIENT,"landscape");
        else
            DFRemoveAttribute(children[WORD_PGSZ],WORD_ORIENT);
    }

    if (children[WORD_PGMAR] == NULL)
        children[WORD_PGMAR] = DFCreateElement(concrete->doc,WORD_PGMAR);

    // Page margins
    if (!DFStringEquals(CSSGet(oldBody,"margin-left"),CSSGet(newBody,"margin-left")) || updatePageSize)
        updateTwipsFromLength(children[WORD_PGMAR],WORD_LEFT,CSSGet(newBody,"margin-left"),widthTwips);

    if (!DFStringEquals(CSSGet(oldBody,"margin-right"),CSSGet(newBody,"margin-right")) || updatePageSize)
        updateTwipsFromLength(children[WORD_PGMAR],WORD_RIGHT,CSSGet(newBody,"margin-right"),widthTwips);

    if (!DFStringEquals(CSSGet(oldBody,"margin-top"),CSSGet(newBody,"margin-top")) || updatePageSize)
        updateTwipsFromLength(children[WORD_PGMAR],WORD_TOP,CSSGet(newBody,"margin-top"),widthTwips);

    if (!DFStringEquals(CSSGet(oldBody,"margin-bottom"),CSSGet(newBody,"margin-bottom")) || updatePageSize)
        updateTwipsFromLength(children[WORD_PGMAR],WORD_BOTTOM,CSSGet(newBody,"margin-bottom"),widthTwips);

    if (children[WORD_PGMAR]->attrsCount == 0)
        children[WORD_PGMAR] = NULL;;

    DFArray *extra = DFArrayNew(NULL,NULL);
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_HEADERREFERENCE:
            case WORD_FOOTERREFERENCE:
                DFArrayAppend(extra,child);
                break;
        }
    }
    replaceChildrenFromArray(concrete,children,WordSectPr_Children);
    for (long i = DFArrayCount(extra)-1; i >= 0; i--) {
        DFNode *child = DFArrayItemAt(extra,i);
        DFInsertBefore(concrete,child,concrete->first);
    }

    DFArrayRelease(extra);
    CSSPropertiesRelease(oldBody);
    CSSPropertiesRelease(oldPage);
}

static DFNode *findSectPr(WordConverter *converter, int add)
{
    DFNode *root = converter->package->document->root;
    if (root->tag != WORD_DOCUMENT)
        return NULL;;
    DFNode *body = DFChildWithTag(root,WORD_BODY);
    if (body == NULL)
        return NULL;;
    DFNode *sectPr = DFChildWithTag(body,WORD_SECTPR);
    if ((sectPr == NULL) & add) {
        sectPr = DFCreateElement(converter->package->document,WORD_SECTPR);
        DFAppendChild(body,sectPr);
    }
    return sectPr;
}

static void parseBody(WordConverter *converter, CSSSheet *styleSheet)
{
    DFNode *sectPr = findSectPr(converter,0);
    if (sectPr == NULL)
        return;
    WordGetSectPr(sectPr,CSSSheetBodyProperties(styleSheet),CSSSheetPageProperties(styleSheet),converter->mainSection);
}

static void updateBody(WordConverter *converter, CSSSheet *styleSheet)
{
    DFNode *sectPr = findSectPr(converter,1);
    if (sectPr == NULL)
        return;
    WordPutSectPr(sectPr,styleSheet,converter->mainSection);
}

static void styleParse(WordStyle *wordStyle, WordConverter *converter, CSSStyle *style)
{
    DFNode *concrete = wordStyle->element;
    WordGetStyle(concrete,style,converter);

    if (wordStyle->basedOn != NULL) {
        WordStyle *parent = WordSheetStyleForTypeId(converter->styles,wordStyle->type,wordStyle->basedOn);
        if (parent != NULL) {
            CSSStyleSetParent(style,parent->selector);
        }
    }
}

static void fixParagraphSpacing(CSSStyle *style, DFNode *element)
{
    DFNode *pPr = DFChildWithTag(element,WORD_PPR);
    DFNode *spacing = DFChildWithTag(pPr,WORD_SPACING);
    const char *beforeAuto = DFGetAttribute(spacing,WORD_BEFOREAUTOSPACING);
    const char *afterAuto = DFGetAttribute(spacing,WORD_AFTERAUTOSPACING);
    CSSProperties *rule = CSSStyleRule(style);

    if (beforeAuto != NULL) {
        if (Word_parseOnOff(beforeAuto))
            CSSPut(rule,"margin-top",NULL);
        else
            CSSPut(rule,"margin-top","0");
    }
    else {
        if (CSSGet(rule,"margin-top") == NULL)
            CSSPut(rule,"margin-top","0");
    }

    int isBeforeAuto = ((beforeAuto != NULL) && Word_parseOnOff(beforeAuto));
    int isAfterAuto = ((afterAuto != NULL) && Word_parseOnOff(afterAuto));

    if (isBeforeAuto)
        CSSPut(rule,"margin-top",NULL);
    else if (CSSGet(rule,"margin-top") == NULL)
        CSSPut(rule,"margin-top","0");

    if (isAfterAuto)
        CSSPut(rule,"margin-bottom",NULL);
    else if (CSSGet(rule,"margin-bottom") == NULL)
        CSSPut(rule,"margin-bottom","0");
}

CSSSheet *WordParseStyles(WordConverter *converter)
{
    CSSSheet *styleSheet = CSSSheetNew();
    CSSStyle *bodyStyle = CSSSheetLookupElement(styleSheet,"body",NULL,1,0);
    CSSPut(CSSStyleRule(bodyStyle),"counter-reset","h1 h2 h3 h4 h5 h6 figure table");
    parseBody(converter,styleSheet);
    if (converter->package->styles == NULL)
        return styleSheet;;
    DFNode *root = converter->package->styles->root;
    if (root == NULL)
        return styleSheet;
    if (root->tag != WORD_STYLES)
        return styleSheet;;

    WordSheet *sheet = converter->styles;

    const char **allIdents = WordSheetCopyIdents(sheet);
    for (int i = 0; allIdents[i]; i++) {
        WordStyle *wordStyle = WordSheetStyleForIdent(sheet,allIdents[i]);
        if ((wordStyle->selector == NULL) || WordStyleIsProtected(wordStyle))
            continue;
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,wordStyle->selector,1,0);
        styleParse(wordStyle,converter,style);
        const char *defaultVal = DFGetAttribute(wordStyle->element,WORD_DEFAULT);
        if ((defaultVal != NULL) && Word_parseOnOff(defaultVal)) {
            StyleFamily family = WordStyleFamilyForSelector(style->selector);
            CSSSheetSetDefaultStyle(styleSheet,style,family);
            CSSSetDefault(CSSStyleRule(style),1);

            if (family == StyleFamilyParagraph)
                fixParagraphSpacing(style,wordStyle->element);
        }
    }
    free(allIdents);

    DFNode *docDefaults = DFChildWithTag(root,WORD_DOCDEFAULTS);
    DFNode *pPrDefault = DFChildWithTag(docDefaults,WORD_PPRDEFAULT);
    DFNode *pPr = DFChildWithTag(pPrDefault,WORD_PPR);
    if (pPr != NULL) {
        CSSStyle *body = CSSSheetLookupElement(styleSheet,"body",NULL,1,0);
        const char *styleId = NULL;
        WordGetPPr(pPr,CSSStyleRule(body),&styleId,converter->mainSection);
    }

    // Special case for figure style: set left and right margin to auto, if not already set
    CSSStyle *figure = CSSSheetLookupElement(styleSheet,"figure",NULL,0,0);
    if (figure != NULL) {
        if (CSSGet(CSSStyleRule(figure),"margin-left") == NULL)
            CSSPut(CSSStyleRule(figure),"margin-left","auto");
        if (CSSGet(CSSStyleRule(figure),"margin-right") == NULL)
            CSSPut(CSSStyleRule(figure),"margin-right","auto");
    }

    return styleSheet;
}

static void updateDefault(CSSStyle *style, DFNode *element, CSSSheet *styleSheet, WordConverter *converter)
{
    StyleFamily family = WordStyleFamilyForSelector(style->selector);
    if (CSSSheetDefaultStyleForFamily(styleSheet,family) == style)
        DFSetAttribute(element,WORD_DEFAULT,"1");
    else
        DFRemoveAttribute(element,WORD_DEFAULT);
}

static void updateDefaults(WordConverter *converter, CSSSheet *styleSheet)
{
    DFNode *root = converter->package->styles->root;
    CSSStyle *bodyStyle = CSSSheetLookupElement(converter->styleSheet,"body",NULL,0,0);
    if (bodyStyle != NULL) {

        // Remove margin properties
        DFHashTable *collapsed = CSSCollapseProperties(CSSStyleRule(bodyStyle));
        CSSProperties *copy = CSSPropertiesNewWithRaw(collapsed);
        DFHashTableRelease(collapsed);
        CSSPut(copy,"margin-top",NULL);
        CSSPut(copy,"margin-bottom",NULL);
        CSSPut(copy,"margin-left",NULL);
        CSSPut(copy,"margin-right",NULL);

        DFNode *docDefaults = DFChildWithTag(root,WORD_DOCDEFAULTS);
        DFNode *rPrDefault = DFChildWithTag(docDefaults,WORD_RPRDEFAULT);
        DFNode *pPrDefault = DFChildWithTag(docDefaults,WORD_PPRDEFAULT);
        DFNode *rPr = DFChildWithTag(rPrDefault,WORD_RPR);
        DFNode *pPr = DFChildWithTag(pPrDefault,WORD_PPR);

        int hadEmptyRPrDefault = ((rPrDefault != NULL) && (rPrDefault->first == NULL));
        int hadEmptyPPrDefault = ((pPrDefault != NULL) && (pPrDefault->first == NULL));

        if (docDefaults == NULL)
            docDefaults = DFCreateElement(converter->package->styles,WORD_DOCDEFAULTS);
        if (rPrDefault == NULL)
            rPrDefault = DFCreateElement(converter->package->styles,WORD_RPRDEFAULT);
        if (pPrDefault == NULL)
            pPrDefault = DFCreateElement(converter->package->styles,WORD_PPRDEFAULT);
        if (rPr == NULL)
            rPr = DFCreateChildElement(rPrDefault,WORD_RPR);
        if (pPr == NULL)
            pPr = DFCreateChildElement(pPrDefault,WORD_PPR);

        DFAppendChild(docDefaults,rPrDefault);
        DFAppendChild(docDefaults,pPrDefault);
        DFInsertBefore(root,docDefaults,root->first);

        WordPutPPr(pPr,copy,NULL,converter->mainSection,-1);

        if (rPr->first == NULL)
            DFRemoveNode(rPr);
        if (pPr->first == NULL)
            DFRemoveNode(pPr);

        if ((rPrDefault->first == NULL) && !hadEmptyRPrDefault)
            DFRemoveNode(rPrDefault);
        if ((pPrDefault->first == NULL) && !hadEmptyPPrDefault)
            DFRemoveNode(pPrDefault);

        if (docDefaults->first == NULL)
            DFRemoveNode(docDefaults);

        CSSPropertiesRelease(copy);
    }
}

static DFHashTable *WordSheetFindUsedConcreteNumIds(WordSheet *sheet)
{
    DFHashTable *concreteNumIds = DFHashTableNew(NULL,NULL); // Used as a set
    const char **allIdents = WordSheetCopyIdents(sheet);
    for (int i = 0; allIdents[i]; i++) {
        const char *ident = allIdents[i];
        WordStyle *style = WordSheetStyleForIdent(sheet,ident);
        DFNode *pPr = DFChildWithTag(style->element,WORD_PPR);
        DFNode *numPr = DFChildWithTag(pPr,WORD_NUMPR);
        const char *numId = DFGetChildAttribute(numPr,WORD_NUMID,WORD_VAL);
        if (numId != NULL)
            DFHashTableAdd(concreteNumIds,numId,"");
    }
    free(allIdents);
    return concreteNumIds;
}

static char *WordStyleNameForStyle(CSSStyle *style)
{
    char *displayName = CSSStyleCopyDisplayName(style);
    if (displayName != NULL)
        return displayName;

    if (style->className != NULL)
        return WordStyleNameFromClassName(style->className);

    switch (style->tag) {
        case HTML_H1: return strdup("heading 1");
        case HTML_H2: return strdup("heading 2");
        case HTML_H3: return strdup("heading 3");
        case HTML_H4: return strdup("heading 4");
        case HTML_H5: return strdup("heading 5");
        case HTML_H6: return strdup("heading 6");
        case HTML_FIGURE: return strdup("Figure");
    }

    return NULL;
}

void WordUpdateStyles(WordConverter *converter, CSSSheet *styleSheet)
{
    CSSStyle *paraDefault = CSSSheetDefaultStyleForFamily(styleSheet,StyleFamilyParagraph);
    if (CSSGet(CSSStyleRule(paraDefault),"margin-top") == NULL)
        CSSPut(CSSStyleRule(paraDefault),"margin-top","-word-auto");

    if (CSSGet(CSSStyleRule(paraDefault),"margin-bottom") == NULL)
        CSSPut(CSSStyleRule(paraDefault),"margin-bottom","-word-auto");

    if (converter->package->styles == NULL) // FIXME: create this document
        return;;
    DFNode *root = converter->package->styles->root;
    if ((root == NULL) || (root->tag != WORD_STYLES))
        return;;

    DFHashTable *remainingSelectors = DFHashTableNew(NULL,NULL); // Used as a set
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        DFHashTableAdd(remainingSelectors,selector,"");
    }
    free(allSelectors);

    WordSheet *sheet = converter->styles;
    DFHashTable *oldConcreteNumIds = WordSheetFindUsedConcreteNumIds(sheet);
    updateNumbering(converter,styleSheet);

    // Update or remove existing styles
    const char **allIdents = WordSheetCopyIdents(sheet);
    for (int i = 0; allIdents[i]; i++) {
        WordStyle *wordStyle = WordSheetStyleForIdent(sheet,allIdents[i]);
        DFNode *element = wordStyle->element;

        if (WordStyleIsProtected(wordStyle)) {
            DFHashTableRemove(remainingSelectors,wordStyle->selector);
            continue;
        }

        if (!DFStringEquals(wordStyle->type,"paragraph") &&
            !DFStringEquals(wordStyle->type,"character") &&
            !DFStringEquals(wordStyle->type,"table"))
            continue;

        CSSStyle *cssStyle = CSSSheetLookupSelector(styleSheet,wordStyle->selector,0,0);
        if (cssStyle == NULL) {
            // Remove style
            WordSheetRemoveStyle(sheet,wordStyle);
            continue;
        }

        // Update style
        WordPutStyle(element,cssStyle,converter);
        updateDefault(cssStyle,element,styleSheet,converter);
        DFHashTableRemove(remainingSelectors,wordStyle->selector);
    }
    free(allIdents);

    // Sort the list of new styles, so that test output is deterministic
    const char **sortedSelectors = DFHashTableCopyKeys(remainingSelectors);
    DFSortStringsCaseInsensitive(sortedSelectors);

    // Add new styles. We do this in two stages - first creating the styles, and then setting their properties.
    // This is because the second stage depends on referenced styles (e.g. based on and next) to be already
    // present.
    for (int selIndex = 0; sortedSelectors[selIndex]; selIndex++) {
        const char *selector = sortedSelectors[selIndex];
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,selector,0,0);
        const char *familyStr = NULL;

        StyleFamily family = WordStyleFamilyForSelector(selector);
        if (family == StyleFamilyParagraph)
            familyStr = "paragraph";
        else if (family == StyleFamilyCharacter)
            familyStr = "character";
        else if (family == StyleFamilyTable)
            familyStr = "table";
        else
            continue;

        char *styleId = WordStyleIdForStyle(style);
        char *name = WordStyleNameForStyle(style);
        if (name == NULL)
            name = strdup(styleId);;
        WordStyle *wordStyle = WordSheetAddStyle(sheet,familyStr,styleId,name,selector);
        DFCreateChildElement(wordStyle->element,WORD_QFORMAT);
        free(styleId);
        free(name);
    }

    for (int selIndex = 0; sortedSelectors[selIndex]; selIndex++) {
        const char *selector = sortedSelectors[selIndex];
        StyleFamily family = WordStyleFamilyForSelector(selector);
        if ((family != StyleFamilyParagraph) &&
            (family != StyleFamilyCharacter) &&
            (family != StyleFamilyTable))
            continue;

        CSSStyle *style = CSSSheetLookupSelector(styleSheet,selector,0,0);
        WordStyle *wordStyle = WordSheetStyleForSelector(converter->styles,selector);
        assert(wordStyle != NULL);

        CSSStyleAddDefaultHTMLProperties(style);
        // FIXME: language
        // FIXME: not covered by tests
        if ((style->headingLevel >= 1) && (style->headingLevel <= 6))
            CSSStyleSetNext(style,"p.Normal");

        WordPutStyle(wordStyle->element,style,converter);
        updateDefault(style,wordStyle->element,styleSheet,converter);
    }
    free(sortedSelectors);

    // Update body style (document defaults)
    updateDefaults(converter,styleSheet);

    updateBody(converter,styleSheet);

    DFHashTable *newConcreteNumIds = WordSheetFindUsedConcreteNumIds(sheet);
    const char **oldKeys = DFHashTableCopyKeys(oldConcreteNumIds);
    for (int oldIndex = 0; oldKeys[oldIndex]; oldIndex++) {
        const char *numId = oldKeys[oldIndex];
        if (DFHashTableLookup(newConcreteNumIds,numId) == NULL) {
            WordConcreteNum *concreteNum = WordNumberingConcreteWithId(converter->numbering,numId);
            if (concreteNum != NULL)
                WordNumberingRemoveConcrete(converter->numbering,concreteNum);
        }
    }
    free(oldKeys);
    DFHashTableRelease(remainingSelectors);
    DFHashTableRelease(oldConcreteNumIds);
    DFHashTableRelease(newConcreteNumIds);
}

CSSStyle *WordSetupTableGridStyle(CSSSheet *styleSheet, int *changed)
{
    CSSStyle *style = CSSSheetLookupElement(styleSheet,"table","Table_Grid",0,0);
    if (style == NULL) {
        style = CSSSheetLookupElement(styleSheet,"table","Table_Grid",1,0);

        CSSProperties *border = CSSPropertiesNewWithString("border: 1px solid black");

        DFHashTable *collapsed = CSSCollapseProperties(border);
        CSSPropertiesUpdateFromRaw(CSSStyleRule(style),collapsed);
        CSSPropertiesUpdateFromRaw(CSSStyleCell(style),collapsed);
        DFHashTableRelease(collapsed);
        CSSPut(CSSStyleRule(style),"margin-left","auto");
        CSSPut(CSSStyleRule(style),"margin-right","auto");

        // These must be set last, as updateRaw clears them when modifying rule
        CSSStyleSetParent(style,"table.Normal_Table");
        CSSStyleSetDisplayName(style,"Table Grid");

        if (changed != NULL)
            *changed = 1;

        CSSPropertiesRelease(border);
    }

    return style;
}
