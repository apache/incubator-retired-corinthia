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

#ifndef DocFormats_DFUtil_h
#define DocFormats_DFUtil_h

#include <DocFormats/DFError.h>
#include "DFHashTable.h"

int DFFileExists(const char *path);
int DFIsDirectory(const char *path);
int DFCreateDirectory(const char *path, int intermediates, DFError **error);
int DFEmptyDirectory(const char *path, DFError **error);
int DFCopyFile(const char *srcPath, const char *destPath, DFError **error);
int DFDeleteFile(const char *path, DFError **error);
const char **DFContentsOfDirectory(const char *path, int recursive, DFError **error);
int DFPathContentsEqual(const char *path1, const char *path2);

char *DFPathDirName(const char *path);
char *DFPathBaseName(const char *path);
char *DFPathExtension(const char *path);
char *DFPathWithoutExtension(const char *path);
char *DFPathResolveAbsolute(const char *base, const char *relative);
char *DFPathNormalize(const char *path);
char *DFRemovePercentEncoding(const char *encoded);
#endif
