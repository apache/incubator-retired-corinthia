//
//  CSSParser.h
//  DocFormats
//
//  Created by Peter Kelly on 5/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_CSSParser_h
#define DocFormats_CSSParser_h

#include "DFHashTable.h"
#include "DFArray.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSParser                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CSSParser CSSParser;

CSSParser *CSSParserNew(const char *input);
void CSSParserFree(CSSParser *parser);

DFHashTable *CSSParserRules(CSSParser *p);
DFArray *CSSParserSelectors(CSSParser *p);
DFHashTable *CSSParserProperties(CSSParser *p);
DFArray *CSSParserContent(CSSParser *p);

#endif
