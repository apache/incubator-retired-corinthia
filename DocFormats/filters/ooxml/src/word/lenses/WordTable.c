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
#include "WordStyles.h"
#include "WordSheet.h"
#include "Word.h"
#include "DFTable.h"
#include "DFHTMLTables.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSLength.h"
#include "WordSection.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    CSSProperties *tableProperties;
    CSSProperties *cellProperties;
    WordStyle *style;
    DFTable *structure;
    double totalWidthPts;
} ConcreteInfo;

static ConcreteInfo *ConcreteInfoNew(void)
{
    ConcreteInfo *info = (ConcreteInfo *)calloc(1,sizeof(ConcreteInfo));
    info->tableProperties = CSSPropertiesNew();
    info->cellProperties = CSSPropertiesNew();
    return info;
}

static void ConcreteInfoFree(ConcreteInfo *info)
{
    DFTableRelease(info->structure);
    CSSPropertiesRelease(info->tableProperties);
    CSSPropertiesRelease(info->cellProperties);
    free(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordTcLens                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordTcCreateAbstractNode(WordGetData *get, DFNode *concrete)
{
    DFNode *td = WordConverterCreateAbstract(get,HTML_TD,concrete);

    CSSProperties *properties = CSSPropertiesNew();
    DFNode *tcPr = DFChildWithTag(concrete,WORD_TCPR);
    if (tcPr != NULL)
        WordGetTcPr(tcPr,properties);;
    DFHashTable *collapsed = CSSCollapseProperties(properties);
    char *styleValue = CSSSerializeProperties(collapsed);
    DFHashTableRelease(collapsed);
    if (strlen(styleValue) > 0)
        DFSetAttribute(td,HTML_STYLE,styleValue);
    free(styleValue);

    CSSPropertiesRelease(properties);
    return td;
}

static DFNode *WordTcGet(WordGetData *get, DFNode *concrete)
{
    DFNode *abstract = WordTcCreateAbstractNode(get,concrete);
    WordContainerGet(get,&WordBlockLevelLens,abstract,concrete);
    return abstract;
}

static void WordTcPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordBlockLevelLens,abstract,concrete);
}

static void WordTcRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordBlockLevelLens.remove(put,child);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         TableStructure                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *Word_getChildVal(DFNode *parent, Tag childName)
{
    DFNode *child = DFChildWithTag(parent,childName);
    if (child == NULL)
        return NULL;
    else
        return DFGetAttribute(child,WORD_VAL);
}

static const char *Word_getPropertyVal(DFNode *parent, Tag propertiesName, Tag childName)
{
    DFNode *properties = DFChildWithTag(parent,propertiesName);
    if (properties == NULL)
        return NULL;
    else
        return Word_getChildVal(properties,childName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordTblLens                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void populateTableStructure(DFTable *structure, DFNode *concrete)
{
    // Populate table
    int row = 0;
    for (DFNode *tblChild = concrete->first; tblChild != NULL; tblChild = tblChild->next) {
        if (tblChild->tag != WORD_TR)
            continue;
        DFTableSetRowElement(structure,tblChild,row);
        int col = 0;
        for (DFNode *trChild = tblChild->first; trChild != NULL; trChild = trChild->next) {
            if (trChild->tag != WORD_TC)
                continue;

            const char *gridSpan = NULL;
            const char *vMerge = NULL;

            DFNode *tcPr = DFChildWithTag(trChild,WORD_TCPR);
            if (tcPr != NULL) {
                DFNode *gridSpanNode = DFChildWithTag(tcPr,WORD_GRIDSPAN);
                DFNode *vMergeNode = DFChildWithTag(tcPr,WORD_VMERGE);

                if (gridSpanNode != NULL)
                    gridSpan = DFGetAttribute(gridSpanNode,WORD_VAL);
                if (vMergeNode != NULL) {
                    vMerge = DFGetAttribute(vMergeNode,WORD_VAL);
                    if (vMerge == NULL)
                        vMerge = "continue";
                }
            }
            int colSpan = (gridSpan != NULL) ? atoi(gridSpan) : 1;

            DFCell *cell;
            if ((vMerge != NULL) && !DFStringEquals(vMerge,"restart") && (row > 0)) {
                cell = DFCellRetain(DFTableGetCell(structure,row-1,col));
                if (cell->rowSpan < row + 1 - cell->row)
                    cell->rowSpan = row + 1 - cell->row;
            }
            else {
                cell = DFCellNew(trChild,row,col);
                cell->colSpan = colSpan;
            }

            for (int i = 0; i < colSpan; i++)
                DFTableSetCell(structure,row,col+i,cell);
            col += colSpan;
            DFCellRelease(cell);
        }
        row++;
    }
}

static DFTableDimensions tableDimensionsForConcrete(DFNode *concrete)
{
    int rows = 0;
    int cols = 0;

    for (DFNode *tblChild = concrete->first; tblChild != NULL; tblChild = tblChild->next) {
        if (tblChild->tag == WORD_TR) {
            rows++;

            int thisRowCols = 0;
            for (DFNode *trChild = tblChild->first; trChild != NULL; trChild = trChild->next) {
                if (trChild->tag == WORD_TC) {
                    const char *gridSpan = Word_getPropertyVal(trChild,WORD_TCPR,WORD_GRIDSPAN);
                    int colSpan = (gridSpan != NULL) ? atoi(gridSpan) : 1;
                    thisRowCols += colSpan;
                }
            }
            if (cols < thisRowCols)
                cols = thisRowCols;

        }
    }
    return DFTableDimensionsMake(rows,cols);
}

static DFTable *tableStructureForConcrete(DFNode *concrete)
{
    DFTableDimensions dim = tableDimensionsForConcrete(concrete);
    DFTable *structure = DFTableNew(dim.rows,dim.cols);
    populateTableStructure(structure,concrete);

    // Compute column widths
    unsigned int col = 0;
    DFNode *tblGrid = DFChildWithTag(concrete,WORD_TBLGRID);
    if (tblGrid != NULL) {
        for (DFNode *child = tblGrid->first; child != NULL; child = child->next) {

            if ((child->tag != WORD_GRIDCOL) || (col >= structure->cols))
                continue;

            const char *widthStr = DFGetAttribute(child,WORD_W);
            if (widthStr == NULL)
                continue;

            // Column widths are measured in twips (twentieths of a point)
            // FIXME: What if the width is specified as a percentage?
            double width = atoi(widthStr)/20.0;
            DFTableSetColWidth(structure,col,width);
            col++;
        }
    }
    DFTableFixZeroWidthCols(structure);

    return structure;
}

// Finds the type of measurement used for cell widths in a table. If the table is empty, or
// the cells use different types of widths, this returns NULL.
static const char *cellWidthTypeForTable(DFNode *tbl)
{
    const char *commonType = NULL;

    for (DFNode *tr = tbl->first; tr != NULL; tr = tr->next) {
        if (tr->tag != WORD_TR)
            continue;
        for (DFNode *tc = tr->first; tc != NULL; tc = tc->next) {
            if (tc->tag != WORD_TC)
                continue;
            DFNode *tcPr = DFChildWithTag(tc,WORD_TCPR);
            DFNode *tcW = DFChildWithTag(tcPr,WORD_TCW);

            if (tcW != NULL) {
                const char *type = DFGetAttribute(tcW,WORD_TYPE);
                if (type == NULL)
                    type = "dxa";

                if (commonType == NULL) // First cell we've encountered
                    commonType = type;
                else if (!DFStringEquals(commonType,type))
                    return NULL;
            }
        }
    }

    return commonType;
}

typedef struct {
    double leftPts;
    double rightPts;
} CellPadding;

static CellPadding getPadding(CSSSheet *styleSheet, WordStyle *wordStyle, CSSProperties *cellProperties)
{
    const char *paddingLeftStr = NULL;
    const char *paddingRightStr = NULL;

    CSSStyle *style;
    if ((wordStyle != NULL) && (wordStyle->selector != NULL)) {
        style = CSSSheetLookupSelector(styleSheet,wordStyle->selector,0,0);
    }
    else {
        style = CSSSheetDefaultStyleForFamily(styleSheet,StyleFamilyTable);
    }

    if (style != NULL) {
        paddingLeftStr = CSSGet(CSSStyleCell(style),"padding-left");
        paddingRightStr = CSSGet(CSSStyleCell(style),"padding-right");
    }

    if (CSSGet(cellProperties,"padding-left") != NULL)
        paddingLeftStr = CSSGet(cellProperties,"padding-left");
    if (CSSGet(cellProperties,"padding-right") != NULL)
        paddingRightStr = CSSGet(cellProperties,"padding-right");;

    CellPadding padding;
    padding.leftPts = 0;
    padding.rightPts = 0;

    CSSLength paddingLeftLength = CSSLengthFromString(paddingLeftStr);
    if (CSSLengthIsValid(paddingLeftLength) && (paddingLeftLength.units == UnitsPt))
        padding.leftPts = paddingLeftLength.value;;

    CSSLength paddingRightLength = CSSLengthFromString(paddingRightStr);
    if (CSSLengthIsValid(paddingRightLength) && (paddingRightLength.units == UnitsPt))
        padding.rightPts = paddingRightLength.value;;

    return padding;
}

static void calcTotals(WordGetData *get, ConcreteInfo *cinfo)
{
    CellPadding padding = getPadding(get->conv->styleSheet,cinfo->style,cinfo->cellProperties);
    cinfo->totalWidthPts = cinfo->structure->totalColWidths - padding.leftPts - padding.rightPts;
}

static ConcreteInfo *getConcreteInfo(WordConverter *converter, DFNode *concrete)
{
    ConcreteInfo *cinfo = ConcreteInfoNew();

    DFNode *tblPr = DFChildWithTag(concrete,WORD_TBLPR);
    if (tblPr != NULL) {
        const char *styleId = NULL;
        WordGetTblPr(tblPr,cinfo->tableProperties,cinfo->cellProperties,converter->mainSection,&styleId);
        cinfo->style = WordSheetStyleForTypeId(converter->styles,"table",styleId);
    }
    cinfo->structure = tableStructureForConcrete(concrete);

    return cinfo;
}

static DFNode *WordTblGet(WordGetData *get, DFNode *concrete)
{
    if (concrete->tag != WORD_TBL)
        return NULL;;

    DFNode *table = WordConverterCreateAbstract(get,HTML_TABLE,concrete);
    ConcreteInfo *cinfo = getConcreteInfo(get->conv,concrete);
    calcTotals(get,cinfo);
    const char *cellWidthType = cellWidthTypeForTable(concrete);
    int autoWidth = DFStringEquals(cellWidthType,"auto");

    if ((CSSGet(cinfo->tableProperties,"width") == NULL) && autoWidth) {
        CSSPut(cinfo->tableProperties,"width",NULL);
    }
    else {
        // Determine column widths and table width

        if (cinfo->totalWidthPts > 0) {
            DFNode *colgroup = HTML_createColgroup(get->conv->html,cinfo->structure);
            DFAppendChild(table,colgroup);

            double tableWidthPct = 100.0;
            if (WordSectionContentWidth(get->conv->mainSection) > 0) {
                double contentWidthPts = WordSectionContentWidth(get->conv->mainSection)/20.0;
                tableWidthPct = 100.0*cinfo->totalWidthPts/contentWidthPts;
                if (CSSGet(cinfo->tableProperties,"width") == NULL) {
                    char buf[100];
                    CSSPut(cinfo->tableProperties,"width",DFFormatDoublePct(buf,100,tableWidthPct));
                }
            }
        }

        if (CSSGet(cinfo->tableProperties,"width") == NULL)
            CSSPut(cinfo->tableProperties,"width","100%");
    }

    DFHashTable *collapsed = CSSCollapseProperties(cinfo->tableProperties);
    char *styleValue = CSSSerializeProperties(collapsed);
    DFHashTableRelease(collapsed);
    if (strlen(styleValue) > 0)
        DFSetAttribute(table,HTML_STYLE,styleValue);
    free(styleValue);
    if ((cinfo->style != NULL) && (cinfo->style->selector != NULL)) {
        char *className = CSSSelectorCopyClassName(cinfo->style->selector);
        DFSetAttribute(table,HTML_CLASS,className);
        free(className);
    }
    else {
        CSSStyle *defaultStyle = CSSSheetDefaultStyleForFamily(get->conv->styleSheet,StyleFamilyTable);
        if (defaultStyle != NULL)
            DFSetAttribute(table,HTML_CLASS,defaultStyle->className);
    }

    // Create rows and cells
    int row = 0;
    for (DFNode *tblChild = concrete->first; tblChild != NULL; tblChild = tblChild->next) {
        if (tblChild->tag != WORD_TR)
            continue;
        DFNode *tr = WordConverterCreateAbstract(get,HTML_TR,tblChild);
        DFAppendChild(table,tr);
        unsigned int col = 0;
        while (col < cinfo->structure->cols) {
            DFCell *cell = DFTableGetCell(cinfo->structure,row,col);
            if (cell == NULL) {
                DFNode *td = DFCreateElement(get->conv->html,HTML_TD);
                DFAppendChild(tr,td);
                col++;
                continue;
            }

            if (row == cell->row) {
                DFNode *td = WordTcGet(get,cell->element);
                DFAppendChild(tr,td);
                if (cell->colSpan != 1)
                    DFFormatAttribute(td,HTML_COLSPAN,"%d",cell->colSpan);
                if (cell->rowSpan != 1)
                    DFFormatAttribute(td,HTML_ROWSPAN,"%d",cell->rowSpan);
            }
            col += cell->colSpan;
        }
        row++;
    }

    ConcreteInfoFree(cinfo);
    return table;
}

static DFNode *concreteRowForAbstractRow(WordPutData *put, DFNode *htmlTr)
{
    DFNode *wordTr = WordConverterGetConcrete(put,htmlTr);
    if (wordTr == NULL) {
        wordTr = DFCreateElement(put->contentDoc,WORD_TR);
    }
    else {
        // We're reusing an existing row element, but we first need to clear all the cells in
        // it, because we'll be rebuilding the cell list
        DFNode *next;
        for (DFNode *child = wordTr->first; child != NULL; child = next) {
            next = child->next;
            if (child->tag == WORD_TC)
                DFRemoveNode(child);
        }
    }
    return wordTr;
}

static void updateTrJc(DFNode *wordTr, const char *oldJc, const char *newJc)
{
    DFNode *tblPrEx = DFChildWithTag(wordTr,WORD_TBLPREX);
    DFNode *trPr = DFChildWithTag(wordTr,WORD_TRPR);

    if (tblPrEx != NULL)
        DFRemoveNode(tblPrEx);
    if (trPr != NULL)
        DFRemoveNode(trPr);

    if (trPr == NULL)
        trPr = DFCreateElement(wordTr->doc,WORD_TRPR);

    WordPutTrPr(trPr,oldJc,newJc);

    if (trPr->first == NULL)
        trPr = NULL;;

    DFNode *first = wordTr->first;
    if (tblPrEx != NULL)
        DFInsertBefore(wordTr,tblPrEx,first);
    if (trPr != NULL)
        DFInsertBefore(wordTr,trPr,first);
}

static void WordTblPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    if ((abstract->tag != HTML_TABLE) || (concrete->tag != WORD_TBL))
        return;;

    DFTable *abstractStructure = HTML_tableStructure(abstract);
    const char *inlineCSSText = DFGetAttribute(abstract,HTML_STYLE);
    CSSProperties *tableProperties = CSSPropertiesNewWithString(inlineCSSText);
    CSSProperties *cellProperties = CSSPropertiesNew();
    const char *className = DFGetAttribute(abstract,HTML_CLASS);
    char *selector = CSSMakeSelector("table",className);
    WordStyle *style = WordSheetStyleForSelector(put->conv->styles,selector);
    CellPadding padding = getPadding(put->conv->styleSheet,style,cellProperties);

    DFNode *tblPr = DFChildWithTag(concrete,WORD_TBLPR);
    if (tblPr == NULL)
        tblPr = DFCreateElement(concrete->doc,WORD_TBLPR);;

    DFNode *tblGrid = DFChildWithTag(concrete,WORD_TBLGRID);
    if (tblGrid == NULL)
        tblGrid = DFCreateElement(concrete->doc,WORD_TBLGRID);

    while (concrete->first != NULL)
        DFRemoveNode(concrete->first);

    const char *oldJc = DFGetChildAttribute(tblPr,WORD_JC,WORD_VAL);
    WordPutTblPr(tblPr,tableProperties,NULL,put->conv->mainSection,style != NULL ? style->styleId : NULL);
    const char *newJc = DFGetChildAttribute(tblPr,WORD_JC,WORD_VAL);

    double tableWidthPct = 100;
    if (CSSGet(tableProperties,"width") != NULL) {
        CSSLength length = CSSLengthFromString(CSSGet(tableProperties,"width"));
        if (CSSLengthIsValid(length) && (length.units == UnitsPct))
            tableWidthPct = length.value;
    }

    double contentWidthPts = WordSectionContentWidthPts(put->conv->mainSection);
    double totalWidthPts = (contentWidthPts+padding.leftPts+padding.rightPts)*(tableWidthPct/100.0);

    while (tblGrid->first != NULL)
        DFRemoveNode(tblGrid->first);
    for (unsigned int i = 0; i < abstractStructure->cols; i++) {
        DFNode *gridCol = DFCreateChildElement(tblGrid,WORD_GRIDCOL);

        double colWidthPct = DFTablePctWidthForCol(abstractStructure,i);
        double colWidthPts = totalWidthPts*colWidthPct/100.0;
        int colWidthTwips = (int)round(colWidthPts*20);

        DFFormatAttribute(gridCol,WORD_W,"%d",colWidthTwips);
    }

    DFAppendChild(concrete,tblPr);
    DFAppendChild(concrete,tblGrid);
    for (unsigned int row = 0; row < abstractStructure->rows; row++) {
        DFNode *htmlTr = DFTableGetRowElement(abstractStructure,row);
        DFNode *wordTr = concreteRowForAbstractRow(put,htmlTr);
        updateTrJc(wordTr,oldJc,newJc);
        DFAppendChild(concrete,wordTr);
        unsigned int col = 0;
        while (col < abstractStructure->cols) {

            DFCell *cell = DFTableGetCell(abstractStructure,row,col);
            assert(cell != NULL);

            DFNode *tc = WordConverterGetConcrete(put,cell->element);
            if ((tc == NULL) || (row != cell->row))
                tc = DFCreateElement(concrete->doc,WORD_TC);
            DFAppendChild(wordTr,tc);

            if (cell->row == row)
                WordTcPut(put,cell->element,tc);;

            const char *vMerge = NULL;
            if (cell->rowSpan > 1) {
                if (row == cell->row)
                    vMerge = "restart";
                else
                    vMerge = "continue";
            }

            DFNode *tcPr = DFChildWithTag(tc,WORD_TCPR);
            if (tcPr == NULL)
                tcPr = DFCreateElement(concrete->doc,WORD_TCPR);
            // Make sure tcPr comes first
            DFInsertBefore(tc,tcPr,tc->first);

            WordPutTcPr2(tcPr,cell->colSpan,vMerge);

            const char *inlineCSSText = DFGetAttribute(cell->element,HTML_STYLE);
            CSSProperties *innerCellProperties = CSSPropertiesNewWithString(inlineCSSText);

            if ((row == cell->row) && (totalWidthPts > 0)) {
                double spannedWidthPct = 0;
                for (unsigned int c = col; c < col + cell->colSpan; c++)
                    spannedWidthPct += DFTablePctWidthForCol(abstractStructure,c);
                char buf[100];
                CSSPut(innerCellProperties,"width",DFFormatDoublePct(buf,100,spannedWidthPct));
            }

            WordPutTcPr1(tcPr,innerCellProperties);

            int haveBlockLevelElement = 0;
            for (DFNode *tcChild = tc->first; tcChild != NULL; tcChild = tcChild->next) {
                if (WordBlockLevelLens.isVisible(put,tcChild))
                    haveBlockLevelElement = 1;
            }

            // Every cell must contain at least one block-level element
            if (!haveBlockLevelElement) {
                DFNode *p = DFCreateElement(concrete->doc,WORD_P);
                DFAppendChild(tc,p);
            }

            col += cell->colSpan;
            CSSPropertiesRelease(innerCellProperties);
        }
    }

    free(selector);
    DFTableRelease(abstractStructure);
    CSSPropertiesRelease(tableProperties);
    CSSPropertiesRelease(cellProperties);
}

static int WordTblIsVisible(WordPutData *put, DFNode *concrete)
{
    return (concrete->tag == WORD_TBL);
}

static void WordTblRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *tr = concrete->first; tr != NULL; tr = tr->next) {
        if (tr->tag == WORD_TR) {
            for (DFNode *tc = tr->first; tc != NULL; tc = tc->next) {
                if (tc->tag == WORD_TC)
                    WordTcRemove(put,tc);
            }
        }
    }
}

WordLens WordTableLens = {
    .isVisible = WordTblIsVisible,
    .get = WordTblGet,
    .put = WordTblPut,
    .create = NULL, // LENS FIXME
    .remove = WordTblRemove,
};
