//
//  WordStyles.h
//  DocFormats
//
//  Created by Peter Kelly on 26/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordStyles_h
#define DocFormats_WordStyles_h

#include "WordCommonPr.h"
#include "WordNumPr.h"
#include "WordPPr.h"
#include "WordRPr.h"
#include "WordTblPr.h"

CSSSheet *WordParseStyles(WordConverter *converter);
void WordUpdateStyles(WordConverter *converter, CSSSheet *styleSheet);

#endif
