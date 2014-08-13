//
//  Word.h
//  DocFormats
//
//  Created by Peter Kelly on 25/09/13.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_Word_h
#define DocFormats_Word_h

#include "DFError.h"
#include "CSSStyle.h"
#include "CSSSheet.h"

#define WordStyleNameFootnoteReference "footnote reference"
#define WordStyleNameFootnoteText "footnote text"
#define WordStyleNameEndnoteReference "endnote reference"
#define WordStyleNameEndnoteText "endnote text"

CSSStyle *WordSetupTableGridStyle(CSSSheet *styleSheet, int *changed);

int DFHTMLToWord(const char *sourcePath, const char *destPath, const char *tempPath, DFError **error);

#endif
