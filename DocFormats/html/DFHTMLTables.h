//
//  DFHTMLTables.h
//  DocFormats
//
//  Created by Peter Kelly on 17/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFHTMLTables_h
#define DocFormats_DFHTMLTables_h

#include "DFXMLForward.h"
#include "DFTable.h"

DFTable *HTML_tableStructure(DFNode *table);
DFNode *HTML_createColgroup(DFDocument *doc, DFTable *structure);

#endif
