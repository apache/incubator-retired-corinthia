//
//  DFTidyWrapper.h
//  DocFormats
//
//  Created by Peter Kelly on 24/09/13.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFTidyWrapper_h
#define DocFormats_DFTidyWrapper_h

#include "DFBuffer.h"

int DFHTMLTidy(DFBuffer *input, DFBuffer *output, int xHTML, DFError **error);

#endif
