//
//  DFFilesystem.h
//  DocFormats
//
//  Created by Peter Kelly on 23/09/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFUtil_h
#define DocFormats_DFUtil_h

#include "DFError.h"
#include "DFHashTable.h"

const char *DFFormatDouble(char *str, size_t size, double value);
const char *DFFormatDoublePct(char *str, size_t size, double value);
const char *DFFormatDoublePt(char *str, size_t size, double value);

char *DFNormalizeWhitespace(const char *input);

int DFFileExists(const char *path);
int DFIsDirectory(const char *path);
int DFCreateDirectory(const char *path, int intermediates, DFError **error);
int DFCopyFile(const char *srcPath, const char *destPath, DFError **error);
int DFDeleteFile(const char *path, DFError **error);
const char **DFContentsOfDirectory(const char *path, int recursive, DFError **error);
int DFPathContentsEqual(const char *path1, const char *path2);

char *DFPathDirName(const char *path);
char *DFPathBaseName(const char *path);
char *DFPathExtension(const char *path);
char *DFPathWithoutExtension(const char *path);
char *DFPathResolveAbsolute(const char *base, const char *relative);
char *DFPathNormalize(const char *path);
char *DFRemovePercentEncoding(const char *encoded);

#endif
