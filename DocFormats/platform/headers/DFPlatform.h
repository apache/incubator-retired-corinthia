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

#ifndef DocFormats_DFPlatform_h
#define DocFormats_DFPlatform_h

#include "DFError.h"
#include "DFArray.h"

int DFMkdirIfAbsent(const char *path, DFError **error);
int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFArray *array, DFError **error);
int DFGetImageDimensions(const char *path, unsigned int *width, unsigned int *height, DFError **error);

#define DF_ONCE_INIT 0
typedef int DFOnce;
typedef void (*DFOnceFunction)(void);
void DFInitOnce(DFOnce *once, DFOnceFunction fun);

#endif
