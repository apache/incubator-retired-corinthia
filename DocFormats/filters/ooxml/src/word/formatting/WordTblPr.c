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
#include "WordTblPr.h"
#include "WordStyles.h"
#include "WordSection.h"
#include "CSS.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static Tag WordTblPr_children[] = {
    WORD_TBLSTYLE,
    WORD_TBLPPR,
    WORD_TBLOVERLAP,
    WORD_BIDIVISUAL,
    WORD_TBLSTYLEROWBANDSIZE,
    WORD_TBLSTYLECOLBANDSIZE,
    WORD_TBLW,
    WORD_JC,
    WORD_TBLCELLSPACING,
    WORD_TBLIND,
    WORD_TBLBORDERS,
    WORD_SHD,
    WORD_TBLLAYOUT,
    WORD_TBLCELLMAR,
    WORD_TBLLOOK,
    WORD_TBLCAPTION,
    WORD_TBLDESCRIPTION,
    WORD_TBLPRCHANGE,
    0,
};

static Tag WordTblBorders_children[] = {
    WORD_TOP,
    WORD_START,
    WORD_LEFT,
    WORD_BOTTOM,
    WORD_END,
    WORD_RIGHT,
    WORD_INSIDEH,
    WORD_INSIDEV,
    0,
};

static Tag WordTblCellMar_children[] = {
    WORD_TOP,
    WORD_START,
    WORD_LEFT,
    WORD_BOTTOM,
    WORD_END,
    WORD_RIGHT,
    0,
};

static Tag Word_TcPr_children[] = {
    WORD_CNFSTYLE,
    WORD_TCW,
    WORD_GRIDSPAN,
    WORD_HMERGE,
    WORD_VMERGE,
    WORD_TCBORDERS,
    WORD_SHD,
    WORD_NOWRAP,
    WORD_TCMAR,
    WORD_TEXTDIRECTION,
    WORD_TCFITTEXT,
    WORD_VALIGN,
    WORD_HIDEMARK,
    WORD_HEADERS,
    WORD_CELLINS,
    WORD_CELLDEL,
    WORD_CELLMERGE,
    WORD_TCPRCHANGE,
    0,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                          Table properties (tblPr and related elements)                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static char *cssPctWidth(const char *type, const char *width, WordSection *section)
{
    if ((type == NULL) || (width == NULL))
        return NULL;
    if (!strcmp(type,"pct")) {
        // Units: 1/50ths of a percent
        double pct = atoi(width)/50.0;
        char buf[100];
        return strdup(DFFormatDoublePct(buf,100,pct));
    }
    else if (!strcmp(type,"dxa") && (WordSectionContentWidth(section) > 0)) {
        // Units: 1/20ths of a point
        double pct = 100.0*atoi(width)/(double)WordSectionContentWidth(section);
        char buf[100];
        return strdup(DFFormatDoublePct(buf,100,pct));
    }
    return NULL;
}

static void WordGetTblBorders(DFNode *concrete, CSSProperties *table, CSSProperties *cell)
{
    int haveInsideH = 0;
    int haveInsideV = 0;
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_TOP:
                WordGetBorder(child,"top",table);
                break;
            case WORD_BOTTOM:
                WordGetBorder(child,"bottom",table);
                break;
            case WORD_LEFT:
                WordGetBorder(child,"left",table);
                break;
            case WORD_RIGHT:
                WordGetBorder(child,"right",table);
                break;
            case WORD_INSIDEH:
                WordGetBorder(child,"top",cell);
                WordGetBorder(child,"bottom",cell);
                haveInsideH = 1;
                break;
            case WORD_INSIDEV:
                WordGetBorder(child,"left",cell);
                WordGetBorder(child,"right",cell);
                haveInsideV = 1;
                break;
        }
    }

    if (haveInsideH) {
        if (CSSGet(table,"border-left-style") == NULL) {
            CSSPut(table,"border-left-style","hidden");
            CSSPut(table,"border-left-width",NULL);
            CSSPut(table,"border-left-color",NULL);
        }
        if (CSSGet(table,"border-right-style") == NULL) {
            CSSPut(table,"border-right-style","hidden");
            CSSPut(table,"border-right-width",NULL);
            CSSPut(table,"border-right-color",NULL);
        }
    }
    if (haveInsideV) {
        if (CSSGet(table,"border-top-style") == NULL) {
            CSSPut(table,"border-top-style","hidden");
            CSSPut(table,"border-top-width",NULL);
            CSSPut(table,"border-top-color",NULL);
        }
        if (CSSGet(table,"border-bottom-style") == NULL) {
            CSSPut(table,"border-bottom-style","hidden");
            CSSPut(table,"border-bottom-width",NULL);
            CSSPut(table,"border-bottom-color",NULL);
        }
    }
}

static void WordPutTblBorders(DFNode *concrete,
                              CSSProperties *oldTable,
                              CSSProperties *oldCell,
                              CSSProperties *newTable,
                              CSSProperties *newCell)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    DFDocument *doc = concrete->doc;

    WordPutBorder(doc,oldTable,newTable,&children[WORD_TOP],WORD_TOP,"top");
    WordPutBorder(doc,oldTable,newTable,&children[WORD_BOTTOM],WORD_BOTTOM,"bottom");
    WordPutBorder(doc,oldTable,newTable,&children[WORD_LEFT],WORD_LEFT,"left");
    WordPutBorder(doc,oldTable,newTable,&children[WORD_RIGHT],WORD_RIGHT,"right");

    // Avoid updating inside borders for HTML tables with direct formatting
    if (newCell != NULL) {
        WordPutBorder(doc,oldCell,newCell,&children[WORD_INSIDEH],WORD_INSIDEH,"bottom");
        WordPutBorder(doc,oldCell,newCell,&children[WORD_INSIDEV],WORD_INSIDEV,"right");
    }

    replaceChildrenFromArray(concrete,children,WordTblBorders_children);
}

static void WordGetTblCellMarSide(DFNode *concrete, CSSProperties *properties, const char *side)
{
    const char *type = DFGetAttribute(concrete,WORD_TYPE);
    const char *w = DFGetAttribute(concrete,WORD_W);
    if ((type != NULL) && (w != NULL) && !strcmp(type,"dxa")) {
        double pts = atoi(w)/20.0;
        char *name = DFFormatString("padding-%s",side);
        char buf[100];
        char *value = strdup(DFFormatDoublePt(buf,100,pts));
        CSSPut(properties,name,value);
        free(name);
        free(value);
    }
}

static void WordPutTblCellMarSide(DFDocument *doc, CSSProperties *oldp, CSSProperties *newp,
                                  DFNode **childp, Tag tag, const char *side)
{
    // Avoid updating cell margins for HTML tables with direct formatting
    if (newp == NULL)
        return;

    char *name = DFFormatString("padding-%s",side);
    const char *oldValue = CSSGet(oldp,name);
    const char *newValue = CSSGet(newp,name);
    if (!DFStringEquals(oldValue,newValue)) {
        *childp = NULL;

        if (newValue != NULL) {
            CSSLength length = CSSLengthFromString(newValue);
            if (CSSLengthIsValid(length) && (length.units == UnitsPt)) {
                *childp = DFCreateElement(doc,tag);
                int twips = (int)round(length.value*20.0);
                DFFormatAttribute(*childp,WORD_W,"%d",twips);
                DFSetAttribute(*childp,WORD_TYPE,"dxa");
            }
        }
    }
    free(name);
}

static void WordPutTblCellMar(DFNode *concrete, CSSProperties *oldp, CSSProperties *newp)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    DFDocument *doc = concrete->doc;
    WordPutTblCellMarSide(doc,oldp,newp,&children[WORD_LEFT],WORD_LEFT,"left");
    WordPutTblCellMarSide(doc,oldp,newp,&children[WORD_RIGHT],WORD_RIGHT,"right");
    WordPutTblCellMarSide(doc,oldp,newp,&children[WORD_TOP],WORD_TOP,"top");
    WordPutTblCellMarSide(doc,oldp,newp,&children[WORD_BOTTOM],WORD_BOTTOM,"bottom");

    replaceChildrenFromArray(concrete,children,WordTblCellMar_children);
}

static void WordGetTblCellMar(DFNode *concrete, CSSProperties *properties)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_LEFT:
                WordGetTblCellMarSide(child,properties,"left");
                break;
            case WORD_RIGHT:
                WordGetTblCellMarSide(child,properties,"right");
                break;
            case WORD_TOP:
                WordGetTblCellMarSide(child,properties,"top");
                break;
            case WORD_BOTTOM:
                WordGetTblCellMarSide(child,properties,"bottom");
                break;
        }
    }
}

void WordGetTblPr(DFNode *concrete, CSSProperties *table, CSSProperties *cell, WordSection *section, const char **styleId)
{
    if (styleId != NULL)
        *styleId = NULL;

    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_TBLW: {
                const char *type = DFGetAttribute(child,WORD_TYPE);
                const char *width = DFGetAttribute(child,WORD_W);
                if (type == NULL)
                    type = "dxa";
                if (width == NULL)
                    break;
                char *widthValue = cssPctWidth(type,width,section);
                CSSPut(table,"width",widthValue);
                free(widthValue);
                break;
            }
            case WORD_TBLSTYLE:
                if (styleId != NULL)
                    *styleId = DFGetAttribute(child,WORD_VAL);
                break;
            case WORD_TBLBORDERS:
                if (cell != NULL)
                    WordGetTblBorders(child,table,cell);
                break;
            case WORD_TBLCELLMAR:
                WordGetTblCellMar(child,cell);
                break;
            case WORD_JC: {
                const char *value = DFGetAttribute(child,WORD_VAL);
                if ((value != NULL) && !strcmp(value,"center")) {
                    CSSPut(table,"margin-left","auto");
                    CSSPut(table,"margin-right","auto");
                }
                else if ((value != NULL) && (!strcmp(value,"right") || !strcmp(value,"end"))) {
                    CSSPut(table,"margin-left","auto");
                    CSSPut(table,"margin-right","0");
                }
                else if ((value != NULL) && (!strcmp(value,"left") || !strcmp(value,"start"))) {
                    CSSPut(table,"margin-left","0");
                    CSSPut(table,"margin-right","auto");
                }
                break;
            }
        }
    }

    // We have to process tblInd *after* jc, because a jc value of anything other than "left" or
    // "start" causes the tblInd to be ignored
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_TBLIND:
                if ((CSSGet(table,"margin-left") == NULL) || !strcmp(CSSGet(table,"margin-left"),"0")) {
                    const char *type = DFGetAttribute(child,WORD_TYPE);
                    const char *w = DFGetAttribute(child,WORD_W);
                    char *newMarginLeft = cssPctWidth(type,w,section);
                    if ((newMarginLeft != NULL) && !DFStringEquals(newMarginLeft,"0%"))
                        CSSPut(table,"margin-left",newMarginLeft);
                    free(newMarginLeft);
                }
                break;
        }
    }
}

void WordPutTblPr(DFNode *concrete, CSSProperties *newTable, CSSProperties *newCell,
                  WordSection *section, const char *styleId)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    CSSProperties *oldTable = CSSPropertiesNew();
    CSSProperties *oldCell = CSSPropertiesNew();
    const char *oldStyleId = NULL;
    WordGetTblPr(concrete,oldTable,oldCell,section,&oldStyleId);

    if (!DFStringEquals(oldStyleId,styleId)) {
        if (styleId != NULL) {
            children[WORD_TBLSTYLE] = DFCreateElement(concrete->doc,WORD_TBLSTYLE);
            DFSetAttribute(children[WORD_TBLSTYLE],WORD_VAL,styleId);
        }
        else {
            children[WORD_TBLSTYLE] = NULL;
        }
    }

    if (!DFStringEquals(CSSGet(oldTable,"width"),CSSGet(newTable,"width"))) {
        CSSLength length = CSSLengthNull;
        if (CSSGet(newTable,"width") != NULL) {
            length = CSSLengthFromString(CSSGet(newTable,"width"));
            if (!CSSLengthIsValid(length) || (length.units != UnitsPct))
                length = CSSLengthNull;
        }

        if (CSSLengthIsValid(length) && (length.units == UnitsPct)) {
            int pctVal = (int)(round(length.value*50));
            children[WORD_TBLW] = DFCreateElement(concrete->doc,WORD_TBLW);
            DFFormatAttribute(children[WORD_TBLW],WORD_W,"%d",pctVal);
            DFSetAttribute(children[WORD_TBLW],WORD_TYPE,"pct");
        }
        else {
            children[WORD_TBLW] = NULL;
        }
    }

    if (!DFStringEquals(CSSGet(oldTable,"margin-left"),CSSGet(newTable,"margin-left")) ||
        !DFStringEquals(CSSGet(oldTable,"margin-right"),CSSGet(newTable,"margin-right"))) {

        const char *jc = NULL;

        const char *newMarginLeft = CSSGet(newTable,"margin-left");
        const char *newMarginRight = CSSGet(newTable,"margin-right");

        if (DFStringEquals(newMarginLeft,"0") && DFStringEquals(newMarginRight,"auto")) {
            jc = "left";
        }
        else if (DFStringEquals(newMarginLeft,"auto") && DFStringEquals(newMarginRight,"0")) {
            jc = "right";
        }
        else if (DFStringEquals(newMarginLeft,"auto") && DFStringEquals(newMarginRight,"auto")) {
            jc = "center";
        }

        if (jc != NULL) {
            children[WORD_JC] = DFCreateElement(concrete->doc,WORD_JC);
            DFSetAttribute(children[WORD_JC],WORD_VAL,jc);
        }
        else {
            children[WORD_JC] = NULL;
        }

        // Only set tblInd if table is left justified (or unspecified)
        if ((jc == NULL) || DFStringEquals(jc,"left")) {
            if (!DFStringEquals(CSSGet(oldTable,"margin-left"),CSSGet(newTable,"margin-left"))) {


                int haveTwips = 0;
                int twips = 0;
                CSSLength length = CSSLengthFromString(CSSGet(newTable,"margin-left"));
                if (CSSLengthIsValid(length) && (length.units == UnitsPct)) {
                    twips = (int)round((length.value/100.0)*WordSectionContentWidth(section));
                    haveTwips = 1;
                }
                else if (CSSLengthIsValid(length) && (length.units == UnitsPt)) {
                    twips = (int)round(length.value*20);
                    haveTwips = 1;
                }

                if (haveTwips) {
                    children[WORD_TBLIND] = DFCreateElement(concrete->doc,WORD_TBLIND);
                    DFSetAttribute(children[WORD_TBLIND],WORD_TYPE,"dxa");
                    DFFormatAttribute(children[WORD_TBLIND],WORD_W,"%d",twips);
                }
                else {
                    children[WORD_TBLIND] = NULL;
                }
            }
        }
    }

    if (children[WORD_TBLBORDERS] == NULL)
        children[WORD_TBLBORDERS] = DFCreateElement(concrete->doc,WORD_TBLBORDERS);
    WordPutTblBorders(children[WORD_TBLBORDERS],oldTable,oldCell,newTable,newCell);
    if (children[WORD_TBLBORDERS]->first == NULL)
        children[WORD_TBLBORDERS] = NULL;

    if (children[WORD_TBLCELLMAR] == NULL)
        children[WORD_TBLCELLMAR] = DFCreateElement(concrete->doc,WORD_TBLCELLMAR);
    WordPutTblCellMar(children[WORD_TBLCELLMAR],oldCell,newCell);
    if (children[WORD_TBLCELLMAR]->first == NULL)
        children[WORD_TBLCELLMAR] = NULL;

    replaceChildrenFromArray(concrete,children,WordTblPr_children);

    CSSPropertiesRelease(oldTable);
    CSSPropertiesRelease(oldCell);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                               Table row properties (trPr element)                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void WordPutTrPr(DFNode *concrete, const char *oldJc, const char *newJc)
{
    if (!DFStringEquals(oldJc,newJc)) {
        DFNode *jcElem = DFChildWithTag(concrete,WORD_JC);
        if (newJc != NULL) {
            if (jcElem == NULL) {
                jcElem = DFCreateElement(concrete->doc,WORD_JC);
                DFInsertBefore(concrete,jcElem,concrete->first);
            }
            DFSetAttribute(jcElem,WORD_VAL,newJc);
        }
        else {
            if (jcElem != NULL)
                DFRemoveNode(jcElem);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                              Table cell properties (tcPr element)                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void WordGetTcPr(DFNode *concrete, CSSProperties *properties)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_SHD:
                WordGetShd(child,properties);
                break;
        }
    }
}

void WordPutTcPr1(DFNode *concrete, CSSProperties *newp)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    CSSProperties *oldp = CSSPropertiesNew();
    WordGetTcPr(concrete,oldp);

    if (!DFStringEquals(CSSGet(oldp,"width"),CSSGet(newp,"width"))) {
        if (CSSGet(newp,"width") != NULL) {
            CSSLength length = CSSLengthFromString(CSSGet(newp,"width"));
            if (length.units == UnitsPct) {
                int pct50 = (int)round(length.value*50);
                children[WORD_TCW] = DFCreateElement(concrete->doc,WORD_TCW);
                DFSetAttribute(children[WORD_TCW],WORD_TYPE,"pct");
                DFFormatAttribute(children[WORD_TCW],WORD_W,"%d",pct50);
            }
            else {
                children[WORD_TCW] = NULL;
            }
        }
        else {
            children[WORD_TCW] = NULL;
        }
    }

    char *oldBackgroundColor = CSSHexColor(CSSGet(oldp,"background-color"),0);
    char *newBackgroundColor = CSSHexColor(CSSGet(newp,"background-color"),0);
    if (!DFStringEquals(oldBackgroundColor,newBackgroundColor))
        WordPutShd(concrete->doc,&children[WORD_SHD],newBackgroundColor);
    free(oldBackgroundColor);
    free(newBackgroundColor);

    replaceChildrenFromArray(concrete,children,Word_TcPr_children);
    CSSPropertiesRelease(oldp);
}

void WordPutTcPr2(DFNode *concrete, unsigned int gridSpan, const char *vMerge)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    if (gridSpan > 1) {
        children[WORD_GRIDSPAN] = DFCreateElement(concrete->doc,WORD_GRIDSPAN);
        DFFormatAttribute(children[WORD_GRIDSPAN],WORD_VAL,"%u",gridSpan);
    }
    else {
        children[WORD_GRIDSPAN] = NULL;
    }

    if (vMerge != NULL) {
        children[WORD_VMERGE] = DFCreateElement(concrete->doc,WORD_VMERGE);
        if ((vMerge != NULL) && !strcmp(vMerge,"restart"))
            DFSetAttribute(children[WORD_VMERGE],WORD_VAL,vMerge);
    }
    else {
        children[WORD_VMERGE] = NULL;
    }

    replaceChildrenFromArray(concrete,children,Word_TcPr_children);
}
