//
//  TextPackage.h
//  dfutil
//
//  Created by Peter Kelly on 14/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_TextPackage_h
#define dfutil_TextPackage_h

#include "DFError.h"
#include "DFHashTable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           TextPackage                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TextPackage TextPackage;

struct TextPackage {
    size_t retainCount;
    char **keys;
    size_t nkeys;
    DFHashTable *items;
};

TextPackage *TextPackageNewWithFile(const char *filename, DFError **error);
TextPackage *TextPackageNewWithString(const char *string, const char *path, DFError **error);
TextPackage *TextPackageRetain(TextPackage *package);
void TextPackageRelease(TextPackage *package);

#endif
