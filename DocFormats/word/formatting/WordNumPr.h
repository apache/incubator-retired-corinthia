//
//  WordNumPr.h
//  DocFormats
//
//  Created by Peter Kelly on 21/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_WordNumPr_h
#define DocFormats_WordNumPr_h

#include "WordConverter.h"

void WordGetNumPrStyle(DFNode *numPr, CSSStyle *style, WordConverter *converter);
void updateNumbering(WordConverter *converter, CSSSheet *cssSheet);

#endif
