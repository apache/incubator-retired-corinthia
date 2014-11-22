// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
