//
//  WordSection.h
//  DocFormats
//
//  Created by Peter Kelly on 13/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordSection_h
#define DocFormats_WordSection_h

#include "CSSProperties.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordSection                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordSection WordSection;

struct WordSection {
    // All measurements are in twips (1/20th of a point)
    int pageWidth;
    int pageHeight;
    int leftMargin;
    int rightMargin;
    int topMargin;
    int bottomMargin;
};

WordSection *WordSectionNew(void);
void WordSectionFree(WordSection *section);

int WordSectionContentWidth(WordSection *section);
int WordSectionContentHeight(WordSection *section);
double WordSectionContentWidthPts(WordSection *section);
double WordSectionContentHeightPts(WordSection *section);

void WordSectionUpdateFromCSSPage(WordSection *section, CSSProperties *page, CSSProperties *body);

#endif
