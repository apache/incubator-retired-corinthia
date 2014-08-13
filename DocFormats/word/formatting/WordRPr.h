//
//  WordRPr.h
//  DocFormats
//
//  Created by Peter Kelly on 21/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_WordRPr_h
#define DocFormats_WordRPr_h

#include "WordConverter.h"

void WordGetRPr(DFNode *concrete, CSSProperties *properties, const char **styleId, struct WordTheme *theme);
void WordPutRPr(DFNode *concrete, CSSProperties *newp, const char *newStyleId, struct WordTheme *theme);

#endif
