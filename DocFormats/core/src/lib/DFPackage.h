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

#ifndef DocFormats_DFPackage_h
#define DocFormats_DFPackage_h

#include "DFError.h"
#include "DFTypes.h"

typedef struct DFPackage DFPackage;

DFPackage *DFPackageNewFilesystem(const char *rootPath);
DFPackage *DFPackageNewMemory(void);
DFPackage *DFPackageCreateZip(const char *filename, DFError **error);
DFPackage *DFPackageOpenZip(const char *filename, DFError **error);
DFPackage *DFPackageRetain(DFPackage *package);
void DFPackageRelease(DFPackage *package);
int DFPackageSave(DFPackage *package, DFError **error);

int DFPackageRead(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error);
int DFPackageWrite(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error);
int DFPackageExists(DFPackage *package, const char *path);
int DFPackageDelete(DFPackage *package, const char *path, DFError **error);
const char **DFPackageList(DFPackage *package, DFError **error);

#endif
