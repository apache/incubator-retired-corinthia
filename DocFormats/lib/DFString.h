//
//  DFString.h
//  DocFormats
//
//  Created by Peter Kelly on 20/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFString_h
#define DocFormats_DFString_h

#include "DFError.h"
#include "DFArray.h"
#include "DFTypes.h"

uint32_t DFNextChar(const char *strr, size_t *offset);
uint32_t DFPrevChar(const char *str, size_t *offset);
int DFStringCompare(const char *a, const char *b);
int DFStringCompareCI(const char *a, const char *b);
int DFStringEquals(const char *a, const char *b);
int DFStringEqualsCI(const char *a, const char *b);
int DFStringHasPrefix(const char *str, const char *prefix);
int DFStringHasSuffix(const char *str, const char *suffix);
int DFStringIsWhitespace(const char *str);
int DFStringContainsWhitespace(const char *str);
char *DFStringTrimWhitespace(const char *str);
char *DFStringTrimLeadingWhitespace(const char *str);
char *DFSubstring(const char *str, size_t start, size_t end);
char *DFStrDup(const char *str);
char *DFUpperCase(const char *input);
char *DFLowerCase(const char *input);
char *DFVFormatString(const char *format, va_list ap);
char *DFFormatString(const char *format, ...) __attribute__((format(printf,1,2)));
const char **DFStringTokenize(const char *str, int (*isseparator)(int c));
const char **DFStringSplit(const char *str, const char *separator, int lastEmpty);
const char **DFStringArrayFlatten(DFArray *array);
size_t DFStringArrayCount(const char **array);
char *DFAppendPathComponent(const char *path1, const char *path2);
char *DFStringReplace(const char *input, const char *match, const char *replacement);
char *DFQuote(const char *str);
char *DFUnquote(const char *str);
char *DFSpacesToUnderscores(const char *input);
char *DFUnderscoresToSpaces(const char *input);

void DFSortStringsCaseSensitive(const char **strings);
void DFSortStringsCaseInsensitive(const char **strings);

char *DFStringReadFromFile(const char *filename, DFError **error);
int DFStringWriteToFile(const char *str, const char *filename, DFError **error);

size_t DFUTF32Length(const uint32_t *str);
uint32_t *DFUTF8To32(const char *utf8);
char *DFUTF32to8(const uint32_t *utf32);

#endif
