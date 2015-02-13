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

#include "DFTable.h"
#include "DFDOM.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

DFTableDimensions DFTableDimensionsMake(unsigned int rows, unsigned int cols)
{
    DFTableDimensions dimensions;
    dimensions.rows = rows;
    dimensions.cols = cols;
    return dimensions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFCell                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFCell *DFCellNew(DFNode *element, unsigned int row, unsigned int col)
{
    DFCell *cell = (DFCell *)calloc(1,sizeof(DFCell));
    cell->retainCount = 1;
    cell->element = element;
    cell->row = row;
    cell->col = col;
    cell->rowSpan = 1;
    cell->colSpan = 1;
    return cell;
}

DFCell *DFCellRetain(DFCell *cell)
{
    if (cell != NULL)
        cell->retainCount++;
    return cell;
}

void DFCellRelease(DFCell *cell)
{
    if ((cell == NULL) || (--cell->retainCount > 0))
        return;
    free(cell);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFTable                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFTable *DFTableNew(unsigned int rows, unsigned int cols)
{
    DFTable *table = (DFTable *)calloc(1,sizeof(DFTable));
    table->retainCount = 1;
    table->rows = rows;
    table->cols = cols;
    table->rowElements = (DFNode **)calloc(table->rows,sizeof(DFNode *));
    table->colWidths = (double *)calloc(table->cols,sizeof(double));

    table->cells = (DFCell ***)calloc(rows,sizeof(DFCell **));
    for (unsigned int r = 0; r < table->rows; r++)
        table->cells[r] = (DFCell **)calloc(cols,sizeof(DFCell *));

    return table;
}

DFTable *DFTableRetain(DFTable *table)
{
    if (table != NULL)
        table->retainCount++;
    return table;
}

void DFTableRelease(DFTable *table)
{
    if ((table == NULL) || (--table->retainCount > 0))
        return;

    for (unsigned int r = 0; r < table->rows; r++) {
        for (unsigned int c = 0; c < table->cols; c++) {
            DFCellRelease(table->cells[r][c]);
        }
        free(table->cells[r]);
    }
    free(table->cells);
    free(table->rowElements);
    free(table->colWidths);
    free(table);
}

DFCell *DFTableGetCell(DFTable *table, unsigned int row, unsigned int col)
{
    assert((row >= 0) && (row < table->rows));
    assert((col >= 0) && (col < table->cols));
    return table->cells[row][col];
}

void DFTableSetCell(DFTable *table, unsigned int row, unsigned int col, DFCell *cell)
{
    assert((row >= 0) && (row < table->rows));
    assert((col >= 0) && (col < table->cols));
    if (table->cells[row][col] != cell) {
        DFCellRelease(table->cells[row][col]);
        table->cells[row][col] = DFCellRetain(cell);
    }
}

DFNode *DFTableGetRowElement(DFTable *table, unsigned int row)
{
    assert(row < table->rows);
    return table->rowElements[row];
}

void DFTableSetRowElement(DFTable *table, DFNode *element, unsigned int row)
{
    assert(row < table->rows);
    table->rowElements[row] = element;
}

void DFTablePrint(DFTable *table)
{
    for (unsigned int row = 0; row < table->rows; row++) {
        for (unsigned int col = 0; col < table->cols; col++) {
            DFCell *cell = table->cells[row][col];
            if (cell == NULL)
                printf("     -    ");
            else
                printf(" %5d/%d/%d",cell->element->seqNo,cell->colSpan,cell->rowSpan);
        }
        printf("\n");
    }
}

double DFTableWidthForCol(DFTable *table, unsigned int col)
{
    assert(col < table->cols);
    if (col < table->cols)
        return table->colWidths[col];
    else
        return 0; // For when assertions are disabled
}

double DFTablePctWidthForCol(DFTable *table, unsigned int col)
{
    assert(col < table->cols);
    if (col >= table->cols)
        return 0; // For when assertions are disabled
    if (table->totalColWidths <= 0)
        return 0;
    else
        return 100.0*table->colWidths[col]/table->totalColWidths;
}

void DFTableSetColWidth(DFTable *table, unsigned int col, double width)
{
    assert(col < table->cols);
    if (col < table->cols) { // For when assertions are disabled
        if (width < 0)
            width = 0;
        table->totalColWidths -= table->colWidths[col];
        table->colWidths[col] = width;
        table->totalColWidths += table->colWidths[col];
    }
}

void DFTableFixZeroWidthCols(DFTable *table)
{
    unsigned int nonZeroCols = 0;
    for (unsigned int i = 0; i < table->cols; i++) {
        if (table->colWidths[i] != 0)
            nonZeroCols++;
    }

    double averageWidth = (nonZeroCols == 0) ? 1 : (table->totalColWidths/nonZeroCols);

    for (unsigned int i = 0; i < table->cols; i++) {
        if (table->colWidths[i] == 0) {
            table->colWidths[i] = averageWidth;
            table->totalColWidths += averageWidth;
        }
    }
}
