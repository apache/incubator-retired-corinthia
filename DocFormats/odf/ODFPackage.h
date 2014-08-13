//
//  ODFPackage.h
//  DocFormats
//
//  Created by Peter Kelly on 20/06/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_ODFPackage_h
#define DocFormats_ODFPackage_h

#include "DFXMLForward.h"
#include "DFError.h"
#include "ODFManifest.h"
#include "ODFSheet.h"

typedef struct ODFPackage ODFPackage;

struct ODFPackage {
    size_t retainCount;
    char *tempPath;
    DFDocument *contentDoc;
    DFDocument *metaDoc;
    DFDocument *settingsDoc;
    DFDocument *stylesDoc;
    ODFManifest *manifest;
    ODFSheet *sheet;
};

ODFPackage *ODFPackageNew(const char *tempPath, DFError **error);
ODFPackage *ODFPackageRetain(ODFPackage *package);
void ODFPackageRelease(ODFPackage *package);
int ODFPackageSave(ODFPackage *package, DFError **error);

#endif
