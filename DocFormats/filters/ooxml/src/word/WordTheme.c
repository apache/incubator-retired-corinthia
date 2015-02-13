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

#include "DFPlatform.h"
#include "WordTheme.h"
#include "WordConverter.h"
#include "DFDOM.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
        return theme;;

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
