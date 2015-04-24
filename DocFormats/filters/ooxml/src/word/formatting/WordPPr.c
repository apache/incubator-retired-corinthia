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
#include "WordPPr.h"
#include "WordStyles.h"
#include "WordSection.h"
#include "CSS.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static Tag WordNumPr_Children[] = {
    WORD_ILVL,
    WORD_NUMID,
    WORD_NUMBERINGCHANGE,
    WORD_INS,
    0,
};

static Tag WordPPR_Children[] = {
    WORD_PSTYLE,
    WORD_KEEPNEXT,
    WORD_KEEPLINES,
    WORD_PAGEBREAKBEFORE,
    WORD_FRAMEPR,
    WORD_WIDOWCONTROL,
    WORD_NUMPR,
    WORD_SUPPRESSLINENUMBERS,
    WORD_PBDR,
    WORD_SHD,
    WORD_TABS,
    WORD_SUPPRESSAUTOHYPHENS,
    WORD_KINSOKU,
    WORD_WORDWRAP,
    WORD_OVERFLOWPUNCT,
    WORD_TOPLINEPUNCT,
    WORD_AUTOSPACEDE,
    WORD_AUTOSPACEDN,
    WORD_BIDI,
    WORD_ADJUSTRIGHTIND,
    WORD_SNAPTOGRID,
    WORD_SPACING,
    WORD_IND,
    WORD_CONTEXTUALSPACING,
    WORD_MIRRORINDENTS,
    WORD_SUPPRESSOVERLAP,
    WORD_JC,
    WORD_TEXTDIRECTION,
    WORD_TEXTALIGNMENT,
    WORD_TEXTBOXTIGHTWRAP,
    WORD_OUTLINELVL,
    WORD_DIVID,
    WORD_CNFSTYLE,
    WORD_RPR,
    WORD_SECTPR,
    WORD_PPRCHANGE,
    0,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                               Paragraph properties (pPr element)                               //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void WordGetPBdr(DFNode *concrete, CSSProperties *properties)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_TOP:
                WordGetBorder(child,"top",properties);
                break;
            case WORD_BOTTOM:
                WordGetBorder(child,"bottom",properties);
                break;
            case WORD_LEFT:
                WordGetBorder(child,"left",properties);
                break;
            case WORD_RIGHT:
                WordGetBorder(child,"right",properties);
                break;
            default:
                break;
        }
    }
}

static void WordPutPBdr(DFNode *concrete, CSSProperties *oldp, CSSProperties *newp)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    bzero(children,PREDEFINED_TAG_COUNT*sizeof(DFNode *));
    childrenToArray(concrete,children);

    DFDocument *doc = concrete->doc;

    WordPutBorder(doc,oldp,newp,&children[WORD_TOP],WORD_TOP,"top");
    WordPutBorder(doc,oldp,newp,&children[WORD_BOTTOM],WORD_BOTTOM,"bottom");
    WordPutBorder(doc,oldp,newp,&children[WORD_LEFT],WORD_LEFT,"left");
    WordPutBorder(doc,oldp,newp,&children[WORD_RIGHT],WORD_RIGHT,"right");

    Tag WordPBdr_children[] = {
        WORD_TOP,
        WORD_LEFT,
        WORD_BOTTOM,
        WORD_RIGHT,
        WORD_BETWEEN,
        WORD_BAR,
        0,
    };

    replaceChildrenFromArray(concrete,children,WordPBdr_children);
}

static void WordGetNumPr(DFNode *concrete, CSSProperties *properties)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_NUMID:
                CSSPut(properties,"-word-numId",DFGetAttribute(child,WORD_VAL));
                break;
            case WORD_ILVL:
                CSSPut(properties,"-word-ilvl",DFGetAttribute(child,WORD_VAL));
                break;
        }
    }
}

static void WordPutNumPr(DFNode *concrete, CSSProperties *newp)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    CSSProperties *oldp = CSSPropertiesNew();
    WordGetNumPr(concrete,oldp);

    if (!DFStringEquals(CSSGet(oldp,"-word-numId"),CSSGet(newp,"-word-numId")) ||
        !DFStringEquals(CSSGet(oldp,"-word-ilvl"),CSSGet(newp,"-word-ilvl"))) {
        if (CSSGet(newp,"-word-numId") != NULL) {
            children[WORD_NUMID] = DFCreateElement(concrete->doc,WORD_NUMID);
            DFSetAttribute(children[WORD_NUMID],WORD_VAL,CSSGet(newp,"-word-numId"));

            if (CSSGet(newp,"-word-ilvl") != NULL) {
                children[WORD_ILVL] = DFCreateElement(concrete->doc,WORD_ILVL);
                DFSetAttribute(children[WORD_ILVL],WORD_VAL,CSSGet(newp,"-word-ilvl"));
            }
            else {
                children[WORD_ILVL] = NULL;
            }
        }
        else {
            children[WORD_NUMID] = NULL;
            children[WORD_ILVL] = NULL;
        }
    }

    replaceChildrenFromArray(concrete,children,WordNumPr_Children);
    CSSPropertiesRelease(oldp);
}

void WordGetPPr(DFNode *pPr, CSSProperties *properties, const char **styleId, WordSection *section)
{
    assert(pPr->tag == WORD_PPR);
    if (styleId != NULL)
        *styleId = NULL;
    for (DFNode *child = pPr->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_PSTYLE:
                if (styleId != NULL)
                    *styleId = DFGetAttribute(child,WORD_VAL);
                break;
            case WORD_PBDR:
                WordGetPBdr(child,properties);
                break;
            case WORD_SHD:
                WordGetShd(child,properties);
                break;
            case WORD_JC: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if (val != NULL) {
                    if (!strcmp(val,"left") || !strcmp(val,"start"))
                        CSSPut(properties,"text-align","left");
                    else if (!strcmp(val,"right") || !strcmp(val,"end"))
                        CSSPut(properties,"text-align","right");
                    else if (!strcmp(val,"center"))
                        CSSPut(properties,"text-align","center");
                    else if (!strcmp(val,"both"))
                        CSSPut(properties,"text-align","justify");
                }
                break;
            }
            case WORD_IND: {
                if ((section == NULL) || (WordSectionContentWidth(section) <= 0))
                    break;

                // Units: 1/20th of a point

                const char *firstLine = DFGetAttribute(child,WORD_FIRSTLINE);
                const char *hanging = DFGetAttribute(child,WORD_HANGING);
                const char *left = DFGetAttribute(child,WORD_START);
                if (left == NULL)
                    left = DFGetAttribute(child,WORD_LEFT);;
                const char *right = DFGetAttribute(child,WORD_END);
                if (right == NULL)
                    right = DFGetAttribute(child,WORD_RIGHT);

                if (left != NULL) {
                    double leftPct = 100.0*atoi(left)/(double)WordSectionContentWidth(section);
                    char buf[100];
                    CSSPut(properties,"margin-left",DFFormatDoublePct(buf,100,leftPct));
                }

                if (right != NULL) {
                    double rightPct = 100.0*atoi(right)/(double)WordSectionContentWidth(section);
                    char buf[100];
                    CSSPut(properties,"margin-right",DFFormatDoublePct(buf,100,rightPct));
                }

                // hanging and firstLine attributes are mutually exclusive. If both are specified,
                // hanging takes precedence
                if (hanging != NULL) {
                    double indentPct = -100.0*atoi(hanging)/(double)WordSectionContentWidth(section);
                    char buf[100];
                    CSSPut(properties,"text-indent",DFFormatDoublePct(buf,100,indentPct));
                }
                else if (firstLine != NULL) {
                    double indentPct = 100.0*atoi(firstLine)/(double)WordSectionContentWidth(section);
                    char buf[100];
                    CSSPut(properties,"text-indent",DFFormatDoublePct(buf,100,indentPct));
                }

                break;
            }
            case WORD_SPACING: {
                const char *before = DFGetAttribute(child,WORD_BEFORE);
                if (before != NULL) { // units: 1/20th of a point
                    char buf[100];
                    CSSPut(properties,"margin-top",DFFormatDoublePt(buf,100,atoi(before)/20.0));
                }

                const char *after = DFGetAttribute(child,WORD_AFTER);
                if (after != NULL) { // units: 1/20th of a point
                    char buf[100];
                    CSSPut(properties,"margin-bottom",DFFormatDoublePt(buf,100,atoi(after)/20.0));
                }

                const char *line = DFGetAttribute(child,WORD_LINE);
                const char *lineRule = DFGetAttribute(child,WORD_LINERULE);
                if ((lineRule == NULL) || !strcmp(lineRule,"auto") || !strcmp(lineRule,"exact")) {
                    // Only other alternative for lineRule is "atLeast", which we ignore
                    if (line != NULL) { // units: 1/2.4th of a percent
                        char buf[100];
                        CSSPut(properties,"line-height",DFFormatDoublePct(buf,100,atoi(line)/2.4));
                    }
                }
                break;
            }
        }
    }
}

void WordPutPPr(DFNode *pPr, CSSProperties *properties, const char *styleId, WordSection *section, int outlineLvl)
{
    assert(pPr->tag == WORD_PPR);

    // The child elements of pPr have to be in a specific order. So we build up a structure based
    // on the existing elements (updated as appropriate from newProperties), and then rebuild
    // from that

    CSSProperties *oldp = CSSPropertiesNew();
    CSSProperties *newp = properties;
    const char *oldStyleId = NULL;
    const char *newStyleId = styleId;
    WordGetPPr(pPr,oldp,&oldStyleId,section);

    {
        DFNode *children[PREDEFINED_TAG_COUNT];
        childrenToArray(pPr,children);

        int existingOutlineLvl = -1;
        if (children[WORD_OUTLINELVL] != NULL) {
            const char *value = DFGetAttribute(children[WORD_OUTLINELVL],WORD_VAL);
            if (value != NULL)
                existingOutlineLvl = atoi(value);
        }

        // Style name
        if (!DFStringEquals(oldStyleId,newStyleId)) {
            if (newStyleId != NULL) {
                children[WORD_PSTYLE] = DFCreateElement(pPr->doc,WORD_PSTYLE);
                DFSetAttribute(children[WORD_PSTYLE],WORD_VAL,newStyleId);
            }
            else {
                children[WORD_PSTYLE] = NULL;
            }
        }

        // Paragraph border (pBdr)

        if (children[WORD_PBDR] == NULL)
            children[WORD_PBDR] = DFCreateElement(pPr->doc,WORD_PBDR);

        WordPutPBdr(children[WORD_PBDR],oldp,newp);

        if (children[WORD_PBDR]->first == NULL)
            children[WORD_PBDR] = NULL;

        // Numbering and outline level
        // Don't change these properties for styles with outline level >= 6, as these can't be
        // represented with the standard HTML heading elements, which only go from h1 - h6
        // (outline levels 0 - 5)
        if (existingOutlineLvl < 6) {
            if (children[WORD_NUMPR] == NULL)
                children[WORD_NUMPR] = DFCreateElement(pPr->doc,WORD_NUMPR);

            WordPutNumPr(children[WORD_NUMPR],newp);

            if (children[WORD_NUMPR]->first == NULL)
                children[WORD_NUMPR] = NULL;

            if ((outlineLvl >= 0) && (outlineLvl <= 5)) {
                if (children[WORD_OUTLINELVL] == NULL)
                    children[WORD_OUTLINELVL] = DFCreateElement(pPr->doc,WORD_OUTLINELVL);
                DFFormatAttribute(children[WORD_OUTLINELVL],WORD_VAL,"%d",outlineLvl);
            }
            else {
                children[WORD_OUTLINELVL] = NULL;
            }
        }

        // background-color

        char *oldBackgroundColor = CSSHexColor(CSSGet(oldp,"background-color"),0);
        char *newBackgroundColor = CSSHexColor(CSSGet(newp,"background-color"),0);
        if (!DFStringEquals(oldBackgroundColor,newBackgroundColor))
            WordPutShd(pPr->doc,&children[WORD_SHD],newBackgroundColor);
        free(oldBackgroundColor);
        free(newBackgroundColor);

        // text-align

        if (!DFStringEquals(CSSGet(oldp,"text-align"),CSSGet(newp,"text-align"))) {
            const char *newTextAlign = CSSGet(newp,"text-align");
            if (newTextAlign != NULL) {
                const char *val = NULL;
                if (!strcmp(newTextAlign,"left"))
                    val = "left";
                else if (!strcmp(newTextAlign,"right"))
                    val = "right";
                else if (!strcmp(newTextAlign,"center"))
                    val = "center";
                else if (!strcmp(newTextAlign,"justify"))
                    val = "both";
                if (val != NULL) {
                    children[WORD_JC] = DFCreateElement(pPr->doc,WORD_JC);
                    DFSetAttribute(children[WORD_JC],WORD_VAL,val);
                }
            }
            else {
                children[WORD_JC] = NULL;
            }
        }

        if ((section != NULL) && (WordSectionContentWidth(section) >= 0)) {
            const char *oldMarginLeft = CSSGet(oldp,"margin-left");
            const char *oldMarginRight = CSSGet(oldp,"margin-right");
            const char *oldTextIndent = CSSGet(oldp,"text-indent");
            const char *newMarginLeft = CSSGet(newp,"margin-left");
            const char *newMarginRight = CSSGet(newp,"margin-right");
            const char *newTextIndent = CSSGet(newp,"text-indent");

            // Special case of the List_Paragraph style, which is used by Word to ensure lists are indented. We
            // don't set this property for HTML, because it automatically indents lists. However we need to ensure
            // that it remains unchanged when updating the word document
            const char *newWordMarginLeft = CSSGet(newp,"-word-margin-left");
            if (newMarginLeft == NULL)
                newMarginLeft = newWordMarginLeft;

            if ((newMarginLeft == NULL) && (newMarginRight == NULL) && (newTextIndent == NULL)) {
                children[WORD_IND] = NULL;
            }
            else {
                if (children[WORD_IND] == NULL)
                    children[WORD_IND] = DFCreateElement(pPr->doc,WORD_IND);

                if (!DFStringEquals(oldMarginLeft,newMarginLeft)) {
                    if (newMarginLeft != NULL)
                        updateTwipsFromLength(children[WORD_IND],WORD_LEFT,newMarginLeft,WordSectionContentWidth(section));
                    else
                        DFRemoveAttribute(children[WORD_IND],WORD_LEFT);
                    DFRemoveAttribute(children[WORD_IND],WORD_START);
                }

                if (!DFStringEquals(oldMarginRight,newMarginRight)) {
                    if (newMarginRight != NULL)
                        updateTwipsFromLength(children[WORD_IND],WORD_RIGHT,newMarginRight,WordSectionContentWidth(section));
                    else
                        DFRemoveAttribute(children[WORD_IND],WORD_RIGHT);
                    DFRemoveAttribute(children[WORD_IND],WORD_END);
                }

                if (!DFStringEquals(oldTextIndent,newTextIndent)) {
                    if (newTextIndent != NULL) {
                        CSSLength length = CSSLengthFromString(newTextIndent);

                        if (CSSLengthIsValid(length)) {
                            double pts = CSSLengthToPts(length,WordSectionContentWidthPts(section));
                            int twips = (int)round(pts*20);

                            if (twips >= 0) {
                                DFFormatAttribute(children[WORD_IND],WORD_FIRSTLINE,"%d",twips);
                                DFRemoveAttribute(children[WORD_IND],WORD_HANGING);
                            }
                            else {
                                DFFormatAttribute(children[WORD_IND],WORD_HANGING,"%d",-twips);
                                DFRemoveAttribute(children[WORD_IND],WORD_FIRSTLINE);
                            }
                        }
                    }
                    else {
                        DFRemoveAttribute(children[WORD_IND],WORD_FIRSTLINE);
                        DFRemoveAttribute(children[WORD_IND],WORD_HANGING);
                    }
                }

            }
        }

        if (!DFStringEquals(CSSGet(oldp,"margin-top"),CSSGet(newp,"margin-top")) ||
            !DFStringEquals(CSSGet(oldp,"margin-bottom"),CSSGet(newp,"margin-bottom")) ||
            !DFStringEquals(CSSGet(oldp,"line-height"),CSSGet(newp,"line-height"))) {

            if ((CSSGet(newp,"margin-top") == NULL) &&
                (CSSGet(newp,"margin-bottom") == NULL) &&
                (CSSGet(newp,"line-height") == NULL)) {
                children[WORD_SPACING] = NULL;
            }
            else {
                children[WORD_SPACING] = DFCreateElement(pPr->doc,WORD_SPACING);

                if (DFStringEquals(CSSGet(newp,"margin-top"),"-word-auto")) {
                    DFSetAttribute(children[WORD_SPACING],WORD_BEFORE,"100");
                    DFSetAttribute(children[WORD_SPACING],WORD_BEFOREAUTOSPACING,"1");
                }
                else {
                    char *before = twipsFromCSS(CSSGet(newp,"margin-top"),WordSectionContentWidth(section));
                    DFSetAttribute(children[WORD_SPACING],WORD_BEFORE,before);
                    DFSetAttribute(children[WORD_SPACING],WORD_BEFOREAUTOSPACING,NULL);
                    free(before);
                }

                if (DFStringEquals(CSSGet(newp,"margin-bottom"),"-word-auto")) {
                    DFSetAttribute(children[WORD_SPACING],WORD_AFTER,"100");
                    DFSetAttribute(children[WORD_SPACING],WORD_AFTERAUTOSPACING,"1");
                }
                else {
                    char *after = twipsFromCSS(CSSGet(newp,"margin-bottom"),WordSectionContentWidth(section));
                    DFSetAttribute(children[WORD_SPACING],WORD_AFTER,after);
                    DFSetAttribute(children[WORD_SPACING],WORD_AFTERAUTOSPACING,NULL);
                    free(after);
                }

                CSSLength lineHeight = CSSLengthFromString(CSSGet(newp,"line-height"));
                if (CSSLengthIsValid(lineHeight) && (lineHeight.units == UnitsPct)) {
                    int value = (int)round(lineHeight.value*2.4);
                    DFFormatAttribute(children[WORD_SPACING],WORD_LINE,"%d",value);
                }
                
                if (children[WORD_SPACING]->attrsCount == 0)
                    children[WORD_SPACING] = NULL;
            }
        }
        
        replaceChildrenFromArray(pPr,children,WordPPR_Children);
    }
    CSSPropertiesRelease(oldp);
}
