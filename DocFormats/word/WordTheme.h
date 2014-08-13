//
//  WordTheme.h
//  DocFormats
//
//  Created by Peter Kelly on 7/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordTheme_h
#define DocFormats_WordTheme_h

#include "WordPackage.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordTheme                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordTheme WordTheme;

struct WordTheme {
    char *majorFont;
    char *minorFont;
};

WordTheme *WordThemeNew(WordPackage *package);
void WordThemeFree(WordTheme *theme);

#endif
