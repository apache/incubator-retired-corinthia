//
//  WordTblPr.h
//  DocFormats
//
//  Created by Peter Kelly on 21/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_WordTblPr_h
#define DocFormats_WordTblPr_h

#include "WordConverter.h"

void WordGetTblPr(DFNode *concrete, CSSProperties *table, CSSProperties *cell, struct WordSection *section, const char **styleId);
void WordPutTblPr(DFNode *concrete, CSSProperties *newTable, CSSProperties *newCell,
                  struct WordSection *section, const char *styleId);

void WordPutTrPr(DFNode *concrete, const char *oldJc, const char *newJc);

void WordGetTcPr(DFNode *concrete, CSSProperties *properties);
void WordPutTcPr2(DFNode *concrete, unsigned int gridSpan, const char *vMerge);
void WordPutTcPr1(DFNode *concrete, CSSProperties *newp);

#endif
