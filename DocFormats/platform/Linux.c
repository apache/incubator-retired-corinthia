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

#include "DFCommon.h"
#include "DFPlatform.h"

// This file contains functions that are applicable to Linux (or more generally, any non-Apple Unix platform)

#ifndef WIN32
#ifndef __APPLE__

int DFGetImageDimensions(const char *path, unsigned int *width, unsigned int *height, DFError **error)
{
    printf("WARNING: DFGetImageDimensions is not implemented on Linux\n");
    DFErrorFormat(error,"DFGetImageDimensions is not implemented on Linux");
    return 0;
}

#endif
#endif
