// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef DocFormats_DFXML_h
#define DocFormats_DFXML_h

#include "DFXMLNamespaces.h"
#include "DFXMLNames.h"
#include <DocFormats/DFXMLForward.h>
#include <DocFormats/DFError.h>
#include "DFBuffer.h"
#include <DocFormats/DFStorage.h>

DFDocument *DFParseXMLString(const char *str, DFError **error);
DFDocument *DFParseXMLFile(const char *filename, DFError **error);
DFDocument *DFParseXMLStorage(DFStorage *storage, const char *filename, DFError **error);

void DFSerializeXMLBuffer(DFDocument *doc, NamespaceID defaultNS, int indent, DFBuffer *buf);
char *DFSerializeXMLString(DFDocument *doc, NamespaceID defaultNS, int indent);
int DFSerializeXMLFile(DFDocument *doc, NamespaceID defaultNS, int indent, const char *filename, DFError **error);
int DFSerializeXMLStorage(DFDocument *doc, NamespaceID defaultNS, int indent,
                          DFStorage *storage, const char *filename,
                          DFError **error);

#endif
