//
//  CSSClassNames.h
//  DocFormats
//
//  Created by Peter Kelly on 20/08/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_CSSClassNames_h
#define DocFormats_CSSClassNames_h

#include "DFXMLForward.h"
#include "CSSSheet.h"

void CSSEnsureReferencedStylesPresent(DFDocument *htmlDoc, CSSSheet *styleSheet);
void CSSSetHTMLDefaults(CSSSheet *styleSheet);
void CSSEnsureUnique(CSSSheet *styleSheet, DFDocument *htmlDoc, int creating);

#endif
