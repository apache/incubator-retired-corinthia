//
//  DFTable.h
//  DocFormats
//
//  Created by Peter Kelly on 16/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFTable_h
#define DocFormats_DFTable_h

#include "DFXMLForward.h"
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
