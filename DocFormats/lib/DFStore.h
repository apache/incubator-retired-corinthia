// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DocFormats_DFStore_h
#define DocFormats_DFStore_h

#include "DFError.h"
#include "DFTypes.h"

#define UnknownPath ""

typedef struct DFStore DFStore;

DFStore *DFStoreNewFilesystem(const char *rootPath);
DFStore *DFStoreRetain(DFStore *store);
void DFStoreRelease(DFStore *store);
void DFStoreSave(DFStore *store);

int DFStoreReadFile(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error);
int DFStoreWriteFile(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error);

int DFStoreFileExists(DFStore *store, const char *path);
int DFStoreIsDirectory(DFStore *store, const char *path);
int DFStoreCreateDirectory(DFStore *store, const char *path, int intermediates, DFError **error);
int DFStoreDeleteFile(DFStore *store, const char *path, DFError **error);
const char **DFStoreContentsOfDirectory(DFStore *store, const char *path, int recursive, DFError **error);

#endif
