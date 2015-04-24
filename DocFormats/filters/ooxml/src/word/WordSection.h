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

#ifndef DocFormats_WordSection_h
#define DocFormats_WordSection_h

#include "CSSProperties.h"
#include "OOXMLTypedefs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordSection                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

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
