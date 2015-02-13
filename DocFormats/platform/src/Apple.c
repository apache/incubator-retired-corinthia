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

// This file contains functions that are applicable to iOS and OS X

#ifdef __APPLE__

#include <ImageIO/ImageIO.h>

int DFGetImageDimensions(const void *data, size_t len, const char *ext,
                         unsigned int *width, unsigned int *height, char **errmsg)
{
    // FIXME: Should use ext here to determine the UTI, and pass that in the options directory
    // (the second parameter to CGImageSourceCreateWithData)

    CFDataRef cfdata = CFDataCreate(NULL,data,len);
    CGImageSourceRef imageSrc = CGImageSourceCreateWithData(cfdata,NULL);
    CFRelease(cfdata);

    if (imageSrc == NULL)
        return 0;

    CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(imageSrc,0,NULL);
    if (properties != NULL) {
        CFNumberRef widthNum = CFDictionaryGetValue(properties,kCGImagePropertyPixelWidth);
        CFNumberRef heightNum = CFDictionaryGetValue(properties,kCGImagePropertyPixelHeight);
        if ((widthNum != NULL) && (heightNum != NULL)) {
            CFIndex widthValue = 0;
            CFIndex heightValue = 0;
            CFNumberGetValue(widthNum,kCFNumberCFIndexType,&widthValue);
            CFNumberGetValue(heightNum,kCFNumberCFIndexType,&heightValue);
            CFRelease(properties);
            *width = (unsigned int)widthValue;
            *height = (unsigned int)heightValue;
            return 1;
        }
        CFRelease(properties);
    }
    return 0;
}

#endif
