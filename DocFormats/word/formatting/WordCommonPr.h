//
//  WordCommonPr.h
//  DocFormats
//
//  Created by Peter Kelly on 21/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_WordCommonPr_h
#define DocFormats_WordCommonPr_h

#include "WordConverter.h"

int Word_parseOnOff(const char *value);

char *twipsFromCSS(const char *str, int relativeTwips);
void updateTwipsFromLength(DFNode *element, Tag attr, const char *value, int relativeTwips);

void WordGetShd(DFNode *concrete, CSSProperties *properties);
void WordPutShd(DFDocument *doc, DFNode **shd, const char *hexColor);

void WordGetBorder(DFNode *concrete, const char *side, CSSProperties *properties);
void WordPutBorder(DFDocument *doc, CSSProperties *oldp, CSSProperties *newp, DFNode **childp,
                   Tag tag, const char *side);

#endif
