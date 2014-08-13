//
//  Plain.h
//  dfutil
//
//  Created by Peter Kelly on 24/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_Plain_h
#define dfutil_Plain_h

#include "DFError.h"
#include "DFHashTable.h"
#include "DFDOM.h"
#include "WordPackage.h"

char *Word_toPlain(WordPackage *package, DFHashTable *parts);
int Word_fromPlain(WordPackage *package, const char *plain, const char *path, DFError **error);
char *HTML_toPlain(DFDocument *doc, const char *imagePath, DFError **error);
DFDocument *HTML_fromPlain(const char *plain, const char *path, const char *htmlPath, DFError **error);

#endif
