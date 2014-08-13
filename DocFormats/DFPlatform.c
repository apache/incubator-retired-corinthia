//
//  DFPlatform.c
//  DocFormats
//
//  Created by Peter Kelly on 25/07/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "DFPlatform.h"
#include "DFCommon.h"

#ifdef __APPLE__

#include <ImageIO/ImageIO.h>

int DFPlatformGetImageDimensions(const char *path, unsigned int *width, unsigned int *height)
{
    CFStringRef srcPath = CFStringCreateWithBytes(kCFAllocatorDefault,(const UInt8 *)path,
                                                  strlen(path),kCFStringEncodingUTF8,0);
    if (srcPath == NULL)
        return 0;

    CFURLRef srcURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,srcPath,kCFURLPOSIXPathStyle,0);
    CFRelease(srcPath);
    if (srcURL == NULL) {
        CFRelease(srcPath);
        return 0;
    }

    CGImageSourceRef imageSrc = CGImageSourceCreateWithURL(srcURL,NULL);
    CFRelease(srcURL);
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

#else

int DFPlatformGetImageDimensions(const char *path, unsigned int *width, unsigned int *height)
{
    return 0;
}

#endif
