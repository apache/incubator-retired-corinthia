//
//  DFHTMLNormalization.h
//  DocFormats
//
//  Created by Peter Kelly on 26/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFHTMLNormalization_h
#define DocFormats_DFHTMLNormalization_h

#include "DFXMLNames.h"
#include "DFXMLForward.h"

void HTML_normalizeDocument(DFDocument *doc);

void HTML_pushDownInlineProperties(DFNode *node);

#endif
