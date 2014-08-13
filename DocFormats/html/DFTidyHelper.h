//
//  DFTidyHelper.h
//  DocFormats
//
//  Created by Peter Kelly on 6/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFTidyHelper_h
#define DocFormats_DFTidyHelper_h

#include "DFDOM.h"
#include "tidy.h"

char *copyTidyNodeValue(TidyNode tnode, TidyDoc tdoc);
DFNode *fromTidyNode(DFDocument *htmlDoc, TidyDoc tdoc, TidyNode tnode);

#endif
