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

#include "DFPlatform.h"

// This file contains functions that are applicable to Linux (or more generally, any non-Apple Unix platform)

#ifndef WIN32
#ifndef __APPLE__

#include <SDL2/SDL_image.h>

int DFGetImageDimensions(const void *data, size_t len, const char *ext,
                         unsigned int *width, unsigned int *height, char **errmsg)
{
    SDL_Surface *image = IMG_Load_RW(SDL_RWFromMem((void *)data,len),1);
    if (image == NULL) {
        if (errmsg != NULL)
            *errmsg = strdup(IMG_GetError());
        return 0;
    }

    *width = (unsigned int)image->w;
    *height = (unsigned int)image->h;
    SDL_FreeSurface(image);
    return 1;
}

#endif
#endif
