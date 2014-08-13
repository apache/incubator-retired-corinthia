//
//  DFZipFile.h
//  DocFormats
//
//  Created by Peter Kelly on 26/12/11.
//  Copyright (c) 2011-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFZipFile_h
#define DocFormats_DFZipFile_h

#include "DFError.h"

int DFUnzip(const char *zipFilename, const char *sourceDir, DFError **error);
int DFZip(const char *zipFilename, const char *sourceDir, DFError **error);

#endif
