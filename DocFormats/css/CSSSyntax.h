//
//  CSSSyntax.h
//  DocFormats
//
//  Created by Peter Kelly on 3/11/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_CSSSyntax_h
#define DocFormats_CSSSyntax_h

int CSSValueIsNumber(const char *str);
int CSSValueIsLength(const char *str);
int CSSValueIsColor(const char *str);
int CSSValueIsBorderStyle(const char *str);
int CSSValueIsBorderWidth(const char *str);
int CSSValueIsBorderColor(const char *str);

#endif
