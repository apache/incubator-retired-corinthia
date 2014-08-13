//
//  WordEquation.c
//  DocFormats
//
//  Created by Peter Kelly on 2/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordLenses.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordEquationLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int WordEquationIsVisible(WordPutData *put, DFNode *concrete)
{
    return 0;
}

WordLens WordEquationLens = {
    .isVisible = WordEquationIsVisible,
    .get = NULL,  // LENS FIXME
    .put = NULL, // LENS FIXME
    .create = NULL,  // LENS FIXME
    .remove = NULL, // LENS FIXME
};
