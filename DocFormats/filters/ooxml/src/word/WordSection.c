// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "WordSection.h"
#include "WordStyles.h"
#include "CSSProperties.h"
#include "CSSLength.h"
#include "DFString.h"
#include "DFCommon.h"
#include <math.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordSection                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordSection *WordSectionNew(void)
{
    WordSection *section = (WordSection *)calloc(1,sizeof(WordSection));
    section->pageWidth = A4_WIDTH_TWIPS;
    section->pageHeight = A4_HEIGHT_TWIPS;
    section->leftMargin = 0;
    section->rightMargin = 0;
    section->topMargin = 0;
    section->bottomMargin = 0;
    return section;
}

void WordSectionFree(WordSection *section)
{
    free(section);
}

int WordSectionContentWidth(WordSection *section)
{
    return section->pageWidth - section->leftMargin - section->rightMargin;
}

int WordSectionContentHeight(WordSection *section)
{
    return section->pageHeight - section->topMargin - section->bottomMargin;
}

double WordSectionContentWidthPts(WordSection *section)
{
    return WordSectionContentWidth(section)/20.0;
}

double WordSectionContentHeightPts(WordSection *section)
{
    return WordSectionContentHeight(section)/20.0;
}

static int twipsFromMarginValue(WordSection *section, const char *value)
{
    // Margin percentages (including top and bottom) are always relative to the page width
    CSSLength length = CSSLengthFromString(value);
    if (CSSLengthIsValid(length) && (length.units == UnitsPct))
        return (int)round((length.value/100)*section->pageWidth);
    else if (CSSLengthIsValid(length) && (length.units == UnitsPt))
        return (int)round(length.value*20);
    else
        return 0;
}

void WordSectionUpdateFromCSSPage(WordSection *section, CSSProperties *page, CSSProperties *body)
{
    // FIXME: not covered by tests
    const char *size = CSSGet(page,"size");
    if (DFStringEqualsCI(size,"A4 portrait")) {
        section->pageWidth = A4_WIDTH_TWIPS;
        section->pageHeight = A4_HEIGHT_TWIPS;
    }
    else if (DFStringEqualsCI(size,"A4 landscape")) {
        section->pageWidth = A4_HEIGHT_TWIPS;
        section->pageHeight = A4_WIDTH_TWIPS;
    }
    else if (DFStringEqualsCI(size,"letter portrait")) {
        section->pageWidth = LETTER_WIDTH_TWIPS;
        section->pageHeight = LETTER_HEIGHT_TWIPS;
    }
    else if (DFStringEqualsCI(size,"letter landscape")) {
        section->pageWidth = LETTER_HEIGHT_TWIPS;
        section->pageHeight = LETTER_WIDTH_TWIPS;
    }
    else {
        section->pageWidth = A4_WIDTH_TWIPS;
        section->pageHeight = A4_HEIGHT_TWIPS;
    }

    section->leftMargin = twipsFromMarginValue(section,CSSGet(body,"margin-left"));
    section->rightMargin = twipsFromMarginValue(section,CSSGet(body,"margin-right"));
    section->topMargin = twipsFromMarginValue(section,CSSGet(body,"margin-top"));
    section->bottomMargin = twipsFromMarginValue(section,CSSGet(body,"margin-bottom"));
}
