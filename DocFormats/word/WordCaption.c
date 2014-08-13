//
//  WordCaption.c
//  DocFormats
//
//  Created by Peter Kelly on 1/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordCaption.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordCaption                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordCaption *WordCaptionNew(DFNode *element)
{
    WordCaption *caption = (WordCaption *)calloc(1,sizeof(WordCaption));
    caption->retainCount = 1;
    caption->element = element;
    return caption;
}

WordCaption *WordCaptionRetain(WordCaption *caption)
{
    if (caption != NULL)
        caption->retainCount++;
    return caption;
}

void WordCaptionRelease(WordCaption *caption)
{
    if ((caption == NULL) || (--caption->retainCount > 0))
        return;
    free(caption);
}
