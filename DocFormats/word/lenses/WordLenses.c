//
//  WordLenses.c
//  DocFormats
//
//  Created by Peter Kelly on 2/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "DFCommon.h"

DFNode *WordContainerGet(WordGetData *get, WordLens *childLens, DFNode *abstract, DFNode *concrete)
{
    return BDTContainerGet(get,(DFLens *)childLens,abstract,concrete);
}

void WordContainerPut(WordPutData *put, WordLens *childLens, DFNode *abstract, DFNode *concrete)
{
    BDTContainerPut(put,(DFLens *)childLens,abstract,concrete,
                    (DFLookupConcreteFunction)WordConverterGetConcrete);
}
