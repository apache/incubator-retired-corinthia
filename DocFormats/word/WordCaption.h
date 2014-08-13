//
//  WordCaption.h
//  DocFormats
//
//  Created by Peter Kelly on 1/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordCaption_h
#define DocFormats_WordCaption_h

#include "DFXMLForward.h"
#include "DFTypes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordCaption                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct WordCaption WordCaption;

struct WordCaption {
    size_t retainCount;
    DFNode *element;
    DFNode *number;
    DFNode *contentStart;
    struct WordBookmark *captionTextBookmark;
    struct WordBookmark *labelNumBookmark;
    struct WordBookmark *textBookmark;
};

WordCaption *WordCaptionNew(DFNode *element);
WordCaption *WordCaptionRetain(WordCaption *caption);
void WordCaptionRelease(WordCaption *caption);

#endif
