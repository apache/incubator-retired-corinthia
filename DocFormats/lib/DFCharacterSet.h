//
//  DFCharacterSet.h
//  DocFormats
//
//  Created by Peter Kelly on 18/07/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFCharacterSet_h
#define DocFormats_DFCharacterSet_h

#include "DFTypes.h"

int DFCharIsHex(uint32_t ch);
int DFCharIsWhitespace(uint32_t ch);
int DFCharIsNewline(uint32_t ch);
int DFCharIsWhitespaceOrNewline(uint32_t ch);
int DFCharIsPunctuation(uint32_t ch);

#endif
