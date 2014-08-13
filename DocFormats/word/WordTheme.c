//
//  WordTheme.c
//  DocFormats
//
//  Created by Peter Kelly on 7/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordTheme.h"
#include "WordConverter.h"
#include "DFDOM.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordTheme                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordTheme *WordThemeNew(WordPackage *package)
{
    WordTheme *theme = (WordTheme *)calloc(1,sizeof(WordTheme));

    DFDocument *doc = package->theme;
    if (doc == NULL)
        return theme;

    assert(doc->root != NULL);
    if (doc->root->tag != DML_MAIN_THEME)
        return theme;

    DFNode *themeElementsElem = DFChildWithTag(doc->root,DML_MAIN_THEMEELEMENTS);
    DFNode *fontSchemeElem = DFChildWithTag(themeElementsElem,DML_MAIN_FONTSCHEME);
    DFNode *majorFontElem = DFChildWithTag(fontSchemeElem,DML_MAIN_MAJORFONT);
    DFNode *minorFontElem = DFChildWithTag(fontSchemeElem,DML_MAIN_MINORFONT);

    const char *majorFont = DFGetChildAttribute(majorFontElem,DML_MAIN_LATIN,NULL_TYPEFACE);
    const char *minorFont = DFGetChildAttribute(minorFontElem,DML_MAIN_LATIN,NULL_TYPEFACE);
    theme->majorFont = (majorFont != NULL) ? strdup(majorFont) : NULL;
    theme->minorFont = (minorFont != NULL) ? strdup(minorFont) : NULL;

    return theme;
}

void WordThemeFree(WordTheme *theme)
{
    if (theme != NULL) {
        free(theme->majorFont);
        free(theme->minorFont);
        free(theme);
    }
}
