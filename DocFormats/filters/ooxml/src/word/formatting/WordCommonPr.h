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

#ifndef DocFormats_WordCommonPr_h
#define DocFormats_WordCommonPr_h

#include "WordConverter.h"

int Word_parseOnOff(const char *value);

char *twipsFromCSS(const char *str, int relativeTwips);
void updateTwipsFromLength(DFNode *element, Tag attr, const char *value, int relativeTwips);

void WordGetShd(DFNode *concrete, CSSProperties *properties);
void WordPutShd(DFDocument *doc, DFNode **shd, const char *hexColor);

void WordGetBorder(DFNode *concrete, const char *side, CSSProperties *properties);
void WordPutBorder(DFDocument *doc, CSSProperties *oldp, CSSProperties *newp, DFNode **childp,
                   Tag tag, const char *side);

#endif
