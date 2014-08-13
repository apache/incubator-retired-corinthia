//
//  DFXML.h
//  DocFormats
//
//  Created by Peter Kelly on 14/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFXML_h
#define DocFormats_DFXML_h

#include "DFXMLNamespaces.h"
#include "DFXMLNames.h"
#include "DFXMLForward.h"
#include "DFError.h"
#include "DFBuffer.h"

DFDocument *DFParseXMLString(const char *str, DFError **error);
DFDocument *DFParseXMLFile(const char *filename, DFError **error);

void DFSerializeXMLBuffer(DFDocument *doc, NamespaceID defaultNS, int indent, DFBuffer *buf);
char *DFSerializeXMLString(DFDocument *doc, NamespaceID defaultNS, int indent);
int DFSerializeXMLFile(DFDocument *doc, NamespaceID defaultNS, int indent, const char *filename, DFError **error);

#endif
