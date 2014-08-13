//
//  CSSSelector.h
//  DocFormats
//
//  Created by Peter Kelly on 11/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_CSSSelector_h
#define DocFormats_CSSSelector_h

#include "DFXMLNames.h"
#include "DFXMLForward.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           StyleFamily                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    StyleFamilyUnknown,
    StyleFamilyParagraph,
    StyleFamilyCharacter,
    StyleFamilyTable,
    StyleFamilySpecial,
} StyleFamily;

StyleFamily StyleFamilyFromHTMLTag(Tag tag);

typedef struct SelectorList SelectorList;

struct SelectorList {
    char *selector;
    SelectorList *next;
};

char *CSSMakeSelector(const char *elementName, const char *className);
char *CSSMakeTagSelector(Tag tag, const char *className);
char *CSSMakeNodeSelector(DFNode *node);

Tag CSSSelectorGetTag(const char *selector);
int CSSSelectorHasClassName(const char *selector);
char *CSSSelectorCopyElementName(const char *selector);
char *CSSSelectorCopyClassName(const char *selector);

int CSSSelectorHeadingLevel(const char *selector); // 0 for all non-heading styles, 1-6 otherwise
int CSSSelectorIsHeading(const char *selector);
StyleFamily CSSSelectorFamily(const char *selector);

#endif
