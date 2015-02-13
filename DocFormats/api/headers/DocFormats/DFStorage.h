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

#ifndef DocFormats_DFStorage_h
#define DocFormats_DFStorage_h

#include <DocFormats/DFError.h>
#include <DocFormats/Formats.h>
#include <stddef.h>

typedef struct DFStorage DFStorage;

DFStorage *DFStorageNewFilesystem(const char *rootPath, DFFileFormat format);
DFStorage *DFStorageNewMemory(DFFileFormat format);
DFStorage *DFStorageCreateZip(const char *filename, DFError **error);
DFStorage *DFStorageOpenZip(const char *filename, DFError **error);
DFStorage *DFStorageRetain(DFStorage *storage);
void DFStorageRelease(DFStorage *storage);
DFFileFormat DFStorageFormat(DFStorage *storage);
int DFStorageSave(DFStorage *storage, DFError **error);

int DFStorageRead(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error);
int DFStorageWrite(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error);
int DFStorageExists(DFStorage *storage, const char *path);
int DFStorageDelete(DFStorage *storage, const char *path, DFError **error);
const char **DFStorageList(DFStorage *storage, DFError **error);

#endif
