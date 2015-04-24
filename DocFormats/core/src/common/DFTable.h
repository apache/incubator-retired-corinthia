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

#ifndef DocFormats_DFTable_h
#define DocFormats_DFTable_h

#include <DocFormats/DFXMLForward.h>
#include "DFTypes.h"

typedef struct {
    unsigned int rows;
    unsigned int cols;
} DFTableDimensions;

DFTableDimensions DFTableDimensionsMake(unsigned int rows, unsigned int cols);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFCell                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFCell DFCell;

struct DFCell {
    size_t retainCount;
    DFNode *element;
    unsigned int row;
    unsigned int col;
    unsigned int colSpan;
    unsigned int rowSpan;
};

DFCell *DFCellNew(DFNode *element, unsigned int row, unsigned int col);
DFCell *DFCellRetain(DFCell *cell);
void DFCellRelease(DFCell *cell);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFTable                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFTable DFTable;

struct DFTable {
    size_t retainCount;
    unsigned int rows;
    unsigned int cols;
    double totalColWidths;

    DFCell ***cells;
    DFNode **rowElements;
    double *colWidths;
};

DFTable *DFTableNew(unsigned int rows, unsigned int cols);
DFTable *DFTableRetain(DFTable *table);
void DFTableRelease(DFTable *table);

DFCell *DFTableGetCell(DFTable *table, unsigned int row, unsigned int col);
void DFTableSetCell(DFTable *table, unsigned int row, unsigned int col, DFCell *cell);
DFNode *DFTableGetRowElement(DFTable *table, unsigned int row);
void DFTableSetRowElement(DFTable *table, DFNode *element, unsigned int row);
void DFTablePrint(DFTable *table);
double DFTableWidthForCol(DFTable *table, unsigned int col);
double DFTablePctWidthForCol(DFTable *table, unsigned int col);
void DFTableSetColWidth(DFTable *table, unsigned int col, double width);
void DFTableFixZeroWidthCols(DFTable *table);

#endif
