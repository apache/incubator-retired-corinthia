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

#include "DFHTMLTables.h"
#include "DFTable.h"
#include "DFDOM.h"
#include "DFString.h"
#include "CSSLength.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>

typedef struct RowList RowList;

struct RowList {
    DFNode *rowNode;
    RowList *next;
};

static unsigned int HTML_countCols(DFNode *parent)
{
    unsigned int cols = 0;
    for (DFNode *child = parent->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case HTML_TD:
            case HTML_TR: {
                unsigned int colSpan = 1;
                const char *colSpanStr = DFGetAttribute(child,HTML_COLSPAN);
                if ((colSpanStr != NULL) && (atoi(colSpanStr) >= 1))
                    colSpan = atoi(colSpanStr);
                cols += colSpan;
                break;
            }
        }
    }
    return cols;
}

/* Goes through all of the TR children of a table element, and builds a linked list of RowList objects, with one
   entry for each row. The listPtr parameter must point to a variable in the caller which, upon completion of this
   function, will point to the first item in the list. */
static void getRowList(DFNode *table, RowList **listPtr)
{
    for (DFNode *tableChild = table->first; tableChild != NULL; tableChild = tableChild->next) {
        switch (tableChild->tag) {
            case HTML_TR: {
                RowList *item = (RowList *)calloc(1,sizeof(RowList));
                item->rowNode = tableChild;
                *listPtr = item;
                listPtr = &item->next;
                break;
            }
            case HTML_THEAD:
            case HTML_TBODY:
            case HTML_TFOOT: {
                for (DFNode *partChild = tableChild->first; partChild != NULL; partChild = partChild->next) {
                    if (partChild->tag == HTML_TR) {
                        RowList *item = (RowList *)calloc(1,sizeof(RowList));
                        item->rowNode = partChild;
                        *listPtr = item;
                        listPtr = &item->next;
                    }
                }
                break;
            }
        }
    }
}

static int countRows(RowList *rowList)
{
    int numRows = 0;
    for (RowList *item = rowList; item != NULL; item = item->next) {
        numRows++;
    }
    return numRows;
}

/* For each row in the list, count the number of columns it contains. Return the maximum column count out of all
   of the rows. */
static int findMaxCols(RowList *rows)
{
    int maxCols = 0;
    for (RowList *rowItem = rows; rowItem != NULL; rowItem = rowItem->next) {
        int rowCols = HTML_countCols(rowItem->rowNode);
        if (maxCols < rowCols)
            maxCols = rowCols;
    }
    return maxCols;
}

static void HTML_populateStructure(DFTable *structure, RowList *rowList)
{
    int row = 0;
    for (RowList *rowItem = rowList; rowItem != NULL; rowItem = rowItem->next) {
        DFNode *tr = rowItem->rowNode;
        DFTableSetRowElement(structure,tr,row);
        int col = 0;

        /* Look through all of the children of the current TR element. Each time we find either a TH or TD element,
           add a new cell to the DFTable structure. If the cell spans only a single row and column (the typical case),
           then we will only set a single pointer in the array. However, if the cell spans multiple rows or columns,
           then we will set multiple pointers in the array to refer to the cell object we have created to represent
           this HTML element. */
        for (DFNode *trChild = tr->first; trChild != NULL; trChild = trChild->next) {

            // Is it a TD or TH element?
            if ((trChild->tag != HTML_TD) && (trChild->tag != HTML_TH))
                continue;

            // If there is already a cell set at the current location, this means that the row above the current
            // one contains a cell which spans multiple rows. According to the rules of HTML, this means that we must
            // skip past this, and add the new cell *after* the multiple row-spanning cell above us.
            while (DFTableGetCell(structure,row,col) != NULL)
                col++;

            // Determine the number of rows and columns spanned based on the rowspan and colspan attributes of the
            // TD or TH element
            unsigned int colSpan = 1;
            unsigned int rowSpan = 1;
            const char *colSpanStr = DFGetAttribute(trChild,HTML_COLSPAN);
            const char *rowSpanStr = DFGetAttribute(trChild,HTML_ROWSPAN);
            if ((colSpanStr != NULL) && (atoi(colSpanStr) >= 1))
                colSpan = atoi(colSpanStr);
            if ((rowSpanStr != NULL) && (atoi(rowSpanStr) >= 1))
                rowSpan = atoi(rowSpanStr);;

            // Create the cell object, initialising it with the row and column on which it starts, and setting the
            // rowSpan and colSpan fields to the values derived from the attributes of the TD or TH element, if any
            DFCell *cell = DFCellNew(trChild,row,col);
            cell->rowSpan = rowSpan;
            cell->colSpan = colSpan;

            // Loop through all the locations in the table grid covered by the cell, based on the row and column
            // span. For each location (and there will be at least one), set the cell pointer at that location to
            // refer to the cell object we just created.
            for (unsigned int r = 0; (r < rowSpan) && (row + r < structure->rows); r++) {
                for (unsigned int c = 0; (c < colSpan) && (col + c < structure->cols); c++) {
                    DFTableSetCell(structure,row + r,col + c,cell);
                }
            }
            DFCellRelease(cell);

            // Advance the current column variable by the number of columns spanned by the current cell
            col += colSpan;
        }
        row++;
    }
}

static void HTML_getColWidths(DFTable *structure, DFNode *node, unsigned int *col)
{
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case HTML_COLGROUP:
                HTML_getColWidths(structure,child,col);
                break;
            case HTML_COL: {
                if (*col >= structure->cols)
                    return;;
                const char *width = DFGetAttribute(child,HTML_WIDTH);
                CSSLength length = CSSLengthFromString(width);
                double pct = 0;
                if (CSSLengthIsValid(length) && (length.units == UnitsPct))
                    pct = length.value;
                DFTableSetColWidth(structure,*col,pct);
                (*col)++;
                break;
            }
        }
    }
}

static void freeRowList(RowList *rowList)
{
    while (rowList != NULL) {
        RowList *next = rowList->next;
        free(rowList);
        rowList = next;
    }
}

DFTable *HTML_tableStructure(DFNode *table)
{
    assert(table->tag == HTML_TABLE);

    RowList *rowList = NULL;
    getRowList(table,&rowList);
    int numRows = countRows(rowList);
    int numCols = findMaxCols(rowList);
    DFTable *structure = DFTableNew(numRows,numCols);
    HTML_populateStructure(structure,rowList);
    unsigned int col = 0;
    HTML_getColWidths(structure,table,&col);
    DFTableFixZeroWidthCols(structure);
    freeRowList(rowList);
    return structure;
}

DFNode *HTML_createColgroup(DFDocument *doc, DFTable *structure)
{
    DFNode *colgroup = DFCreateElement(doc,HTML_COLGROUP);
    for (unsigned int i = 0; i < structure->cols; i++) {
        DFNode *col = DFCreateChildElement(colgroup,HTML_COL);
        double width = DFTablePctWidthForCol(structure,i);
        char buf[100];
        DFSetAttribute(col,HTML_WIDTH,DFFormatDoublePct(buf,100,width));
    }
    return colgroup;
}
