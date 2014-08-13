//
//  WordField.h
//  DocFormats
//
//  Created by Peter Kelly on 29/11/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordField_h
#define DocFormats_WordField_h

#include "WordPackage.h"

const char **Word_parseField(const char *cstr);

int Word_simplifyFields(WordPackage *package);

#endif
