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
#include "DFString.h"
#include "DFBuffer.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"
#include "DFFilesystem.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int pointerIndex;
    int storageIndex;
    const char **pointers;
    char *storage;
} DFArrayBuilder;

DFArrayBuilder DFArrayBuilderEmpty = { 0, 0, NULL, NULL };

static void DFArrayBuilderBegin(DFArrayBuilder *builder)
{
    builder->pointerIndex = 0;
    builder->storageIndex = 0;
}

static void DFArrayBuilderAdd(DFArrayBuilder *builder, const char *str, size_t start, size_t end)
{
    assert(start <= end);
    size_t toklen = end - start;
    if ((builder->pointers != NULL) && (builder->storage != NULL)) {
        builder->pointers[builder->pointerIndex] = &builder->storage[builder->storageIndex];
        memcpy(&builder->storage[builder->storageIndex],&str[start],toklen);
        builder->storage[builder->storageIndex+toklen] = '\0';
    }

    builder->pointerIndex += 1;
    builder->storageIndex += toklen + 1;
}

static void DFArrayBuilderAllocate(DFArrayBuilder *builder)
{
    int pointerBytes = (builder->pointerIndex + 1)*sizeof(char *);
    void *mem = malloc(pointerBytes + builder->storageIndex);
    builder->pointers = mem;
    builder->storage = (char *)mem + pointerBytes;
}

static void DFArrayBuilderFinish(DFArrayBuilder *builder)
{
    builder->pointers[builder->pointerIndex] = NULL;
}

#define isstart1(c) (((c) & 0x80) == 0x00) // 0xxxxxxx
#define isstart2(c) (((c) & 0xE0) == 0xC0) // 110xxxxx
#define isstart3(c) (((c) & 0xF0) == 0xE0) // 1110xxxx
#define isstart4(c) (((c) & 0xF8) == 0xF0) // 11110xxx
#define iscont(c)   (((c) & 0xC0) == 0x80) // 10xxxxxx

uint32_t DFNextChar(const char *str, size_t *offsetp)
{
    size_t pos = *offsetp;
    const uint8_t *input = (const uint8_t *)str;

    // If we're part-way through a multi-byte character, skip to the beginning of the next one
    while ((input[pos] != 0) && iscont(input[pos]))
        pos++;

    // End of string
    if (input[pos] == 0) {
        *offsetp = pos;
        return 0;
    }

    if (isstart1(input[pos])) {
        // 1-byte character: 0xxxxxxx
        uint32_t a = input[pos];
        *offsetp = pos+1;
        return a;
    }
    else if (isstart2(input[pos]) && iscont(input[pos+1])) {
        // 2-byte character: 110xxxxx 10xxxxxx
        uint32_t a = input[pos+1] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+0] & 0x1F; // b = bits 6-10
        *offsetp = pos+2;
        return (a << 0) | (b << 6);
    }
    else if (isstart3(input[pos]) && iscont(input[pos+1]) && iscont(input[pos+2])) {
        // 3-byte character: 1110xxxx 10xxxxxx 10xxxxxx
        uint32_t a = input[pos+2] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+1] & 0x3F; // b = bits 6-11
        uint32_t c = input[pos+0] & 0x0F; // c = bits 12-15
        *offsetp = pos+3;
        return (a << 0) | (b << 6) | (c << 12);
    }
    else if (isstart4(input[pos]) && iscont(input[pos+1]) && iscont(input[pos+2]) && iscont(input[pos+3])) {
        // 4-byte character: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        uint32_t a = input[pos+3] & 0x3F; // a = bits 0-5
        uint32_t b = input[pos+2] & 0x3F; // b = bits 6-11
        uint32_t c = input[pos+1] & 0x3F; // c = bits 12-17
        uint32_t d = input[pos+0] & 0x07; // d = bits 18-20
        *offsetp = pos+4;
        return (a << 0) | (b << 6) | (c << 12) | (d << 18);
    }
    else {
        // Invalid UTF8 byte sequence
        *offsetp = pos;
        return 0;
    }
}

uint32_t DFPrevChar(const char *string, size_t *offsetp)
{
    size_t pos = *offsetp;
    const uint8_t *input = (const uint8_t *)string;

    if (pos == 0)
        return 0;

    // Move to the first byte of the previous character
    do {
        pos--;
    } while ((pos > 0) && iscont(input[pos]));

    *offsetp = pos;
    size_t ignore = pos;
    return DFNextChar(string,&ignore);
}

int DFStringCompare(const char *a, const char *b)
{
    if ((a == NULL) && (b == NULL))
        return 0;
    else if ((a == NULL) && (b != NULL))
        return -1;
    else if ((a != NULL) && (b == NULL))
        return 1;
    else
        return strcmp(a,b);
}

int DFStringCompareCI(const char *a, const char *b)
{
    if ((a == NULL) && (b == NULL))
        return 0;
    else if ((a == NULL) && (b != NULL))
        return -1;
    else if ((a != NULL) && (b == NULL))
        return 1;
    else
        return strcasecmp(a,b);
}

int DFStringEquals(const char *a, const char *b)
{
    return (DFStringCompare(a,b) == 0);
}

int DFStringEqualsCI(const char *a, const char *b)
{
    return (DFStringCompareCI(a,b) == 0);
}

int DFStringHasPrefix(const char *str, const char *prefix)
{
    if ((str == NULL) || (prefix == NULL))
        return 0;
    if (strlen(str) < strlen(prefix))
        return 0;
    if (!strncmp(str,prefix,strlen(prefix)))
        return 1;
    return 0;
}

int DFStringHasSuffix(const char *str, const char *suffix)
{
    if ((str == NULL) || (suffix == NULL)) {
        return 0;
    }

    size_t slen = strlen(str);
    size_t suffixlen = strlen(suffix);
    if (slen < suffixlen)
        return 0;
    return !memcmp(&str[slen-suffixlen],suffix,suffixlen);
}

int DFStringIsWhitespace(const char *str)
{
    size_t pos = 0;
    uint32_t ch;
    while ((ch = DFNextChar(str,&pos)) != 0) {
        if (!DFCharIsWhitespaceOrNewline(ch))
            return 0;
    }
    return 1;
}

int DFStringContainsWhitespace(const char *str)
{
    size_t pos = 0;
    uint32_t ch;
    while ((ch = DFNextChar(str,&pos)) != 0) {
        if (DFCharIsWhitespaceOrNewline(ch))
            return 1;
    }
    return 0;
}

char *DFStringTrimWhitespace(const char *str)
{
    size_t startpos = 0;
    size_t pos = 0;
    uint32_t ch;
    do {
        startpos = pos;
        ch = DFNextChar(str,&pos);
    } while ((ch != 0) && DFCharIsWhitespaceOrNewline(ch));

    size_t endpos = strlen(str);

    pos = strlen(str);
    do {
        endpos = pos;
        ch = DFPrevChar(str,&pos);
    } while ((ch != 0) && DFCharIsWhitespaceOrNewline(ch));

    return DFSubstring(str,startpos,endpos);
}

char *DFStringTrimLeadingWhitespace(const char *str)
{
    size_t startpos = 0;
    size_t pos = 0;
    uint32_t ch;
    do {
        startpos = pos;
        ch = DFNextChar(str,&pos);
    } while ((ch != 0) && DFCharIsWhitespaceOrNewline(ch));

    return strdup(&str[startpos]);
}

char *DFStringNormalizeWhitespace(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t inputLen = strlen(input);
    size_t outputLen = 0;

    char *output = (char *)malloc(inputLen+1);

    size_t start = 0;
    while ((start < inputLen) && isspace(input[start]))
        start++;
    size_t end = inputLen;
    while ((end > start) && isspace(input[end-1]))
        end--;
    int haveSpace = 0;
    for (size_t pos = start; pos < end; pos++) {
        if (isspace(input[pos])) {
            if (!haveSpace) {
                output[outputLen++] = ' ';
                haveSpace = 1;
            }
        }
        else {
            output[outputLen++] = input[pos];
            haveSpace = 0;
        }
    }

    assert(outputLen <= inputLen);

    output[outputLen] = '\0';

    return output;
}

char *DFSubstring(const char *str, size_t start, size_t end)
{
    if (str == NULL)
        return NULL;

    for (size_t pos = 0; (pos < start) && (pos < end); pos++) {
        if (str[pos] == 0) {
            start = pos;
            end = pos;
            break;
        }
    }

    if (end < start)
        end = start;

    char *substring = (char *)malloc(end-start+1);
    memcpy(substring,&str[start],end-start);
    substring[end-start] = '\0';
    return substring;
}

char *DFStrDup(const char *str)
{
    if (str != NULL)
        return strdup(str);
    else
        return NULL;
}

char *DFUpperCase(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    char *result = strdup(input);
    for (size_t i = 0; i < len; i++) {
        // Avoid calling toupper with chars from UTF-8 multibyte sequences
        if ((result[i] >= 'a') && result[i] <= 'z')
            result[i] = toupper(result[i]);
    }
    return result;
}

char *DFLowerCase(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    char *result = strdup(input);
    for (size_t i = 0; i < len; i++) {
        // Avoid calling tolower with chars from UTF-8 multibyte sequences
        if ((result[i] >= 'A') && result[i] <= 'Z')
            result[i] = tolower(result[i]);
    }
    return result;
}


char *DFVFormatString(const char *format, va_list ap)
{
    va_list ap2;
    va_copy(ap2,ap);
    size_t nchars = vsnprintf(NULL,0,format,ap2);
    va_end(ap2);

    char *result = (char *)malloc(nchars+1);

    va_copy(ap2,ap);
    vsnprintf(result,nchars+1,format,ap2);
    va_end(ap2);

    return result;
}

char *DFFormatString(const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    char *result = DFVFormatString(format,ap);
    va_end(ap);
    return result;
}

static void DFStringTokenizeInternal(const char *str, int (*isseparator)(int c), DFArrayBuilder *builder)
{
    size_t len = strlen(str);
    size_t start = 0;
    size_t pos = 0;

    DFArrayBuilderBegin(builder);

    for (; pos <= len; pos++) {
        if ((pos == len) || isseparator(str[pos])) {
            if (pos > start)
                DFArrayBuilderAdd(builder,str,start,pos);
            start = pos+1;
        }
    }
}

const char **DFStringTokenize(const char *str, int (*isseparator)(int c))
{
    if (isseparator == NULL) {
        isseparator = isspace;
    }

    DFArrayBuilder builder = DFArrayBuilderEmpty;

    // Determine how many strings there are, and how many bytes we need to allocate for the strings
    DFStringTokenizeInternal(str,isseparator,&builder);

    // Allocate a single block of memory to hold both the strings and the pointers to them ,thus
    // necessitating only a single call to free after the caller has finished with the result
    DFArrayBuilderAllocate(&builder);

    // Build the arrays of strings and pointers
    DFStringTokenizeInternal(str,isseparator,&builder);
    DFArrayBuilderFinish(&builder);

    return builder.pointers;
}

static void DFStringSplitInternal(const char *str, const char *separator, int lastEmpty, DFArrayBuilder *builder)
{
    size_t len = strlen(str);
    size_t start = 0;
    size_t pos = 0;
    size_t separatorLen = strlen(separator);

    DFArrayBuilderBegin(builder);

    while (pos <= len) {
        if ((pos == len) || !strncmp(&str[pos],separator,separatorLen)) {
            if ((pos < len) || lastEmpty || (pos > start))
                DFArrayBuilderAdd(builder,str,start,pos);
            pos += separatorLen;
            start = pos;
        }
        else {
            pos++;
        }
    }
}

const char **DFStringSplit(const char *str, const char *separator, int lastEmpty)
{
    DFArrayBuilder builder = DFArrayBuilderEmpty;
    DFStringSplitInternal(str,separator,lastEmpty,&builder);
    DFArrayBuilderAllocate(&builder);
    DFStringSplitInternal(str,separator,lastEmpty,&builder);
    DFArrayBuilderFinish(&builder);
    return builder.pointers;
}

void DFStringArrayFlattenInternal(DFArray *array, DFArrayBuilder *builder)
{
    DFArrayBuilderBegin(builder);

    size_t count = DFArrayCount(array);
    for (size_t i = 0; i < count; i++) {
        const char *item = (const char *)DFArrayItemAt(array,i);
        DFArrayBuilderAdd(builder,item,0,strlen(item));
    }
}

const char **DFStringArrayFlatten(DFArray *array)
{
    DFArrayBuilder builder = DFArrayBuilderEmpty;
    DFStringArrayFlattenInternal(array,&builder);
    DFArrayBuilderAllocate(&builder);
    DFStringArrayFlattenInternal(array,&builder);
    DFArrayBuilderFinish(&builder);
    return builder.pointers;
}

size_t DFStringArrayCount(const char **array)
{
    size_t count = 0;
    while (array[count] != NULL)
        count++;
    return count;
}

char *DFAppendPathComponent(const char *path1, const char *path2)
{
    char *unnormalized;
    if (strlen(path1) == 0)
        unnormalized = strdup(path2);
    else if (path1[strlen(path1)-1] == '/')
        unnormalized = DFFormatString("%s%s",path1,path2);
    else
        unnormalized = DFFormatString("%s/%s",path1,path2);
    char *normalized = DFPathNormalize(unnormalized);
    free(unnormalized);

    size_t len = strlen(normalized);
    while ((len > 0) && (normalized[len-1] == '/')) {
        normalized[len-1] = '\0';
        len--;
    }

    return normalized;
}

char *DFStringReplace(const char *input, const char *match, const char *replacement)
{
    size_t inputLen = strlen(input);
    size_t matchLen = strlen(match);

    if (matchLen == 0)
        return strdup(input); // protect against infinite loop

    struct DFBuffer *output = DFBufferNew();

    size_t i = 0;
    while (i < inputLen) {
        if (!strncmp(&input[i],match,matchLen)) {
            DFBufferFormat(output,"%s",replacement);
            i += matchLen;
        }
        else {
            DFBufferFormat(output,"%c",input[i]);
            i++;
        }
    }

    char *result = strdup(output->data);
    DFBufferRelease(output);
    return result;
}

char *DFQuote(const char *in)
{
    if (in == NULL) {
        return NULL;
    }

    // The maximum length of the quoted string is 2n + 2, where n is the number of characters in
    // the original string. This is because each character could potentially need a two-character
    // escape, and because we add a quote at the beginning and end of the string. We also need 1
    // extra byte for the NULL terminator, i.e. 2n + 3 bytes in total.

    // UTF-8 characters pass through untouched.

    size_t inlen = strlen(in);
    char *out = (char*)malloc(2*inlen+3);
    size_t outlen = 0;
    out[outlen++] = '"';
    for (size_t i = 0; i < inlen; i++) {
        char c = in[i];
        switch (c) {
            case '"':
                out[outlen++] = '\\';
                out[outlen++] = '"';
                break;
            case '\\':
                out[outlen++] = '\\';
                out[outlen++] = '\\';
                break;
            case '\b':
                out[outlen++] = '\\';
                out[outlen++] = 'b';
                break;
            case '\f':
                out[outlen++] = '\\';
                out[outlen++] = 'f';
                break;
            case '\n':
                out[outlen++] = '\\';
                out[outlen++] = 'n';
                break;
            case '\r':
                out[outlen++] = '\\';
                out[outlen++] = 'r';
                break;
            case '\t':
                out[outlen++] = '\\';
                out[outlen++] = 't';
                break;
            default:
                out[outlen++] = c;
                break;
        }
    }
    out[outlen++] = '"';
    out[outlen++] = '\0';
    assert(outlen <= 2*inlen+3);
    return out;
}

char *DFUnquote(const char *in)
{
    if (in == NULL)
        return NULL;

    if ((in[0] != '"') && (in[0] != '\''))
        return strdup(in);
    char quote = in[0];

    size_t inlen = strlen(in);
    char *out = (char *)malloc(inlen+1);
    size_t outlen = 0;
    size_t i = 1;
    for (; i < inlen; i++) {
        char c = in[i];
        if ((i+1 == inlen) && (c == quote))
            continue;
        if ((c == '\\') && (i+1 < inlen)) {
            i++;
            c = in[i];
            switch (c) {
                case 'b':
                    out[outlen++] = '\b';
                    break;
                case 'f':
                    out[outlen++] = '\f';
                    break;
                case 'n':
                    out[outlen++] = '\n';
                    break;
                case 'r':
                    out[outlen++] = '\r';
                    break;
                case 't':
                    out[outlen++] = '\t';
                    break;
                default:
                    out[outlen++] = c;
                    break;
            }
        }
        else {
            out[outlen++] = c;
        }
    }
    out[outlen++] = '\0';

    assert(outlen <= inlen);
    return out;
}

char *DFSpacesToUnderscores(const char *input)
{
    size_t len = strlen(input);
    char *output = strdup(input);
    for (size_t i = 0; i < len; i++) {
        if (output[i] == ' ')
            output[i] = '_';
    }
    return output;
}

char *DFUnderscoresToSpaces(const char *input)
{
    size_t len = strlen(input);
    char *output = strdup(input);
    for (size_t i = 0; i < len; i++) {
        if (output[i] == '_')
            output[i] = ' ';
    }
    return output;
}

const char *DFFormatDouble(char *str, size_t size, double value)
{
    snprintf(str,size,"%f",value);
    size_t len = strlen(str);
    while ((len > 0) && str[len-1] == '0') {
        len--;
    }
    if ((len > 0) && str[len-1] == '.')
        len--;
    str[len] = '\0';
    return str;
}

const char *DFFormatDoublePct(char *str, size_t size, double value)
{
    DFFormatDouble(str,size,value);
    size_t len = strlen(str);
    if (len+1 < size)
        str[len++] = '%';
    str[len] = '\0';
    return str;
}

const char *DFFormatDoublePt(char *str, size_t size, double value)
{
    DFFormatDouble(str,size,value);
    size_t len = strlen(str);
    if (len+1 < size)
        str[len++] = 'p';
    if (len+1 < size)
        str[len++] = 't';
    str[len] = '\0';
    return str;
}

static int cscompare(const void *val1, const void *val2)
{
    const char *str1 = *(const char **)val1;
    const char *str2 = *(const char **)val2;
    return strcmp(str1,str2);
}

static int cicompare(const void *val1, const void *val2)
{
    const char *str1 = *(const char **)val1;
    const char *str2 = *(const char **)val2;
    return strcasecmp(str1,str2);
}

static int countStrings(const char **strings)
{
    int i = 0;
    while (strings[i])
        i++;
    return i;
}

void DFSortStringsCaseSensitive(const char **strings)
{
    int count = countStrings(strings);
    qsort((void *)strings,count,sizeof(const char *),cscompare);
}

void DFSortStringsCaseInsensitive(const char **strings)
{
    int count = countStrings(strings);
    qsort((void *)strings,count,sizeof(const char *),cicompare);
}

char *DFStringReadFromFile(const char *filename, DFError **error)
{
    DFBuffer *buffer = DFBufferReadFromFile(filename,error);
    if (buffer == NULL)
        return NULL;
    char *result = strdup(buffer->data);
    DFBufferRelease(buffer);
    return result;
}

int DFStringWriteToFile(const char *str, const char *filename, DFError **error)
{
    size_t len = strlen(str);
    return DFWriteDataToFile(str,len,filename,error);
}

size_t DFUTF32Length(const uint32_t *str)
{
    if (str == NULL) {
        return 0;
    }

    size_t size = 0;
    while (str[size] != 0)
        size++;
    return size;
}

uint32_t *DFUTF8To32(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t inpos = 0;
    size_t outlen = 0;
    while (DFNextChar(input,&inpos) != 0)
        outlen++;

    uint32_t *output = (uint32_t *)malloc((outlen+1)*sizeof(uint32_t));
    inpos = 0;
    for (size_t outpos = 0; outpos < outlen; outpos++)
        output[outpos] = DFNextChar(input,&inpos);
    output[outlen] = '\0';

    return output;
}

static size_t DFUTF32to8n(const uint32_t *input, char *soutput)
{
    unsigned char *output = (unsigned char *)soutput;

    size_t inlen = DFUTF32Length(input);
    size_t outlen = 0;

    for (size_t pos = 0; pos < inlen; pos++) {
        uint32_t value = input[pos];
        if (value <= 0x7F) { // Up to 7 bits
            // 7 bits encoded as 1 byte
            // 0aaaaaaa

            if (output != NULL) {
                output[outlen+0] = value & 0x7F; // a = bits 0-7
            }

            outlen += 1;
        }
        else if (value <= 0x7FF) {
            // 11 bits encoded as 2 bytes
            // 110bbbbb 10aaaaaa

            if (output != NULL) {
                output[outlen+0] = 0xC0 | ((value >> 6) & 0x1F); // b = bits 6-10
                output[outlen+1] = 0x80 | ((value >> 0) & 0x3F); // a = bits 0-5
            }

            outlen += 2;
        }
        else if (value <= 0xFFFF) { // Up to 16 bits
            // 16 bits encoded as 3 bytes
            // 1110cccc 10bbbbbb 10aaaaaa

            if (output != NULL) {
                output[outlen+0] = 0xE0 | ((value >> 12) & 0x0F); // c = bits 12-15
                output[outlen+1] = 0x80 | ((value >> 6) & 0x3F);  // b = bits 6-11
                output[outlen+2] = 0x80 | ((value >> 0) & 0x3F);  // a = bits 0-5
            }

            outlen += 3;
        }
        else if (value <= 0x10FFFF) {
            // 21 bits encoded as 4 bytes
            // 11110ddd 10cccccc 10bbbbbb 10aaaaaa

            if (output != NULL) {
                output[outlen+0] = 0xF0 | ((value >> 18) & 0x07); // d = bits 18-20
                output[outlen+1] = 0x80 | ((value >> 12) & 0x3F); // c = bits 12-17
                output[outlen+2] = 0x80 | ((value >> 6) & 0x3F);  // b = bits 6-11
                output[outlen+3] = 0x80 | ((value >> 0) & 0x3F);  // a = bits 0-5
            }

            outlen += 4;
        }

        // If above 0x10FFFF, skip
    }

    return outlen;
}

char *DFUTF32to8(const uint32_t *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t outlen = DFUTF32to8n(input,NULL);
    char *output = (char *)malloc(outlen+1);
    DFUTF32to8n(input,output);
    output[outlen] = '\0';

    return output;
}

// UTF-8 spec: http://tools.ietf.org/html/rfc3629
