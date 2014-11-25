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

#ifndef DocFormats_DFCommon_h
#define DocFormats_DFCommon_h

#include "DFTypes.h"

#ifdef WIN32
#define snprintf _snprintf
#define strcasecmp _stricmp
#define bzero(mem,size) memset(mem,0,size)
#pragma warning(disable: 4090) // 'function': different 'const' qualifiers
#pragma warning(disable: 4996) // The POSIX name for this item is deprecated
#else // not WIN32
#endif

#ifndef S_ISDIR
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)     /* directory */
#endif

#endif
