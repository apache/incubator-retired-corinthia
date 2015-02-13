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

#ifndef DocFormats_Word_h
#define DocFormats_Word_h

#include <DocFormats/DFError.h>
#include <DocFormats/DFStorage.h>
#include "CSSStyle.h"
#include "CSSSheet.h"

#define WordStyleNameFootnoteReference "footnote reference"
#define WordStyleNameFootnoteText "footnote text"
#define WordStyleNameEndnoteReference "endnote reference"
#define WordStyleNameEndnoteText "endnote text"

CSSStyle *WordSetupTableGridStyle(CSSSheet *styleSheet, int *changed);

DFDocument *WordGet(DFStorage *concreteStorage, DFStorage *abstractStorage, DFError **error);
int WordPut(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, DFError **error);
int WordCreate(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, DFError **error);
int WordCollapseBookmarks(DFStorage *concreteStorage, DFError **error);
int WordExpandBookmarks(DFStorage *concreteStorage, DFError **error);

#endif
