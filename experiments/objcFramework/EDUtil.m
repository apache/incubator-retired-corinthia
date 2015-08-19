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

// This file comes from the portion of the UX Write editor that
// works on both Apple platforms (that is, it can run on either
// OS X or iOS). It's in the repository for illustrative purposes
// only, to assist with the creation of the framework for the
// Corinthia editor UI. The code does not compile independently in
// its present form.

#import "EDUtil.h"
#import <FileClient/FCError.h>
#include <iconv.h>

int debugIndent = 0;

void debug(NSString *format, ...)
{
    for (int i = 0; i < debugIndent; i++)
        printf("    ");
    va_list ap;
    va_start(ap,format);
    NSString *message = [[NSString alloc] initWithFormat: format arguments: ap];
    va_end(ap);
    printf("%s",message.UTF8String);
}

DFHashTable *HashTableFromNSDictionary(NSDictionary *dict)
{
    if (dict == NULL)
        return NULL;

    DFHashTable *hashTable = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (NSString *key in dict.allKeys) {
        NSString *value = [dict objectForKey: key];
        assert([value isKindOfClass: [NSString class]]);
        DFHashTableAdd(hashTable,key.UTF8String,value.UTF8String);
    }
    return hashTable;
}

NSDictionary *NSDictionaryFromHashTable(DFHashTable *hashTable)
{
    if (hashTable == NULL)
        return NULL;
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity: 0];
    const char **keys = DFHashTableCopyKeys(hashTable);
    for (int i = 0; keys[i]; i++) {
        const char *key = keys[i];
        const char *value = DFHashTableLookup(hashTable,key);
        [dict setObject: NSStringFromC(value) forKey: NSStringFromC(key)];
    }
    free(keys);
    return dict;
}

NSDictionary *NSDictionaryFromNestedHashTable(DFHashTable *outerHash)
{
    NSMutableDictionary *outerDict = [NSMutableDictionary dictionaryWithCapacity: 0];
    const char **allOuterKeys = DFHashTableCopyKeys(outerHash);
    for (int outerIndex = 0; allOuterKeys[outerIndex]; outerIndex++) {
        const char *outerKey = allOuterKeys[outerIndex];
        DFHashTable *innerHash = DFHashTableLookup(outerHash,outerKey);
        assert(innerHash != NULL);
        NSDictionary *innerDict = NSDictionaryFromHashTable(innerHash);
        [outerDict setObject: innerDict forKey: NSStringFromC(outerKey)];
    }
    free(allOuterKeys);
    return outerDict;
}

#define UTF16IsLeadSurrogate(_c)  (((_c) >= 0xD800) && ((_c) <= 0xDBFF))
#define UTF16IsTrailSurrogate(_c) (((_c) >= 0xDC00) && ((_c) <= 0xDFFF))

typedef struct DFUTF16Buffer DFUTF16Buffer;

struct DFUTF16Buffer {
    size_t alloc;
    size_t len;
    char *data;
};

static void DFUTF16BufferInit(DFUTF16Buffer *buf, size_t alloc)
{
    buf->alloc = alloc;
    buf->len = 0;
    buf->data = (char *)malloc(buf->alloc*sizeof(char));
}

static void DFUTF16BufferDestroy(DFUTF16Buffer *buf)
{
    free(buf->data);
}

static void DFUTF16BufferExpand(DFUTF16Buffer *buf)
{
    buf->alloc = (buf->alloc == 0) ? 1 : buf->alloc*2;
    buf->data = (char *)realloc(buf->data,buf->alloc*sizeof(char));
}

void DFCharEncodingConvert(iconv_t ic, void *data, size_t len, DFUTF16Buffer *output)
{
    iconv(ic,NULL,NULL,NULL,NULL); // Reset converter state

    char *inbuf = (char *)data;
    size_t inbytesleft = len;

    output->len = 0;

    while (inbytesleft > 0) {
        const char *oldInbuf = inbuf;
        char *outbuf = &output->data[output->len];
        size_t outbytesleft = output->alloc - output->len;
        size_t r = iconv(ic,&inbuf,&inbytesleft,&outbuf,&outbytesleft);
        if (r == ((size_t)-1)) {
            if ((errno == EILSEQ) || (errno == EINVAL)) {
                // Invalid or incomplete multibyte sequence; skip it
                inbuf++;
                inbytesleft--;
                iconv(ic,NULL,NULL,NULL,NULL); // Reset converter state

                // We still want the output that was generated
                output->len = outbuf - output->data;
                assert(output->len <= output->alloc);

                continue;
            }
        }

        if (oldInbuf == inbuf) {
            DFUTF16BufferExpand(output);
        }
        else {
            output->len = outbuf - output->data;
            assert(output->len <= output->alloc);
        }
    }
}

int DFCharEncodingFixChars(uint16_t *chars, NSUInteger len)
{
    int changed = 0;

    NSUInteger i = 0;
    while (i < len) {
        if (UTF16IsLeadSurrogate(chars[i])) {
            if ((i+1 < len) && UTF16IsTrailSurrogate(chars[i+1])) {
                // VALID: Surrogate pair
                i += 2;
            }
            else {
                // INVALID: Missing trail surrogate
                chars[i] = '?';
                changed = 1;
                i += 1;
            }
        }
        else if (UTF16IsTrailSurrogate(chars[i])) {
            // INVALID: Unexpected trail surrogate
            chars[i] = '?';
            changed = 1;
            i += 1;
        }
        else {
            // VALID: Single character
            i += 1;
        }
    }

    return changed;
}

NSString *DFCharEncodingFixNSString(NSString *str)
{
    NSUInteger len = str.length;
    uint16_t *chars = (uint16_t *)malloc(len*sizeof(uint16_t));
    [str getCharacters: chars range: NSMakeRange(0,len)];
    if (DFCharEncodingFixChars(chars,len))
        str = [NSString stringWithCharacters: chars length: len];
    free(chars);
    return str;
}

static void printHex(const void *data, size_t len)
{
    const char *chars = (const char *)data;
    for (size_t i = 0; i < len; i++) {
        printf("%02X ",(unsigned char)chars[i]);
    }
    printf("\n");
}

static void printChars(const void *data, size_t len)
{
    const char *chars = (const char *)data;
    for (size_t i = 0; i < len; i++) {
        if ((chars[i] >= 32) && (chars[i] <= 127))
            printf("%c  ",chars[i]);
        else
            printf(".  ");
    }
    printf("\n");
}

void DFCharEncodingTest(void)
{
    iconv_t *UTF8toUCS2 = iconv_open("UCS-2-INTERNAL","UTF-8");
    assert(UTF8toUCS2 != ((iconv_t)-1));
    printf("u8tou16 = %p\n",UTF8toUCS2);

    iconv_t *UCS2toUTF8 = iconv_open("UTF-8","UCS-2-INTERNAL");
    assert(UCS2toUTF8 != ((iconv_t)-1));
    printf("UCS2toUTF8 = %p\n",UCS2toUTF8);

    char *input = "Hello 多语种多多网站";
    size_t inputLen = strlen(input);
    printf("%s\n",input);

    printHex(input,inputLen);
    printChars(input,inputLen);
    printf("----\n");

    NSString *str = [NSString stringWithCString: input encoding: NSUTF8StringEncoding];
    uint16_t *chars = (uint16_t *)calloc(1,str.length*sizeof(uint16_t));
    [str getCharacters: chars range: NSMakeRange(0,str.length)];
    printHex(chars,str.length*sizeof(uint16_t));
    printChars(chars,str.length*sizeof(uint16_t));
    free(chars);

    printf("----\n");
    DFUTF16Buffer ucs2;
    DFUTF16BufferInit(&ucs2,0);
    DFCharEncodingConvert(UTF8toUCS2,input,inputLen,&ucs2);
    printHex(ucs2.data,ucs2.len);
    printChars(ucs2.data,ucs2.len);


    printf("----\n");
    DFUTF16Buffer utf8;
    DFUTF16BufferInit(&utf8,0);
    DFCharEncodingConvert(UCS2toUTF8,ucs2.data,ucs2.len,&utf8);
    printHex(utf8.data,utf8.len);
    printChars(utf8.data,utf8.len);

    char *temp = (char*)malloc(utf8.len+1);
    memcpy(temp,utf8.data,utf8.len);
    temp[utf8.len] = '\0';
    printf("%s\n",temp);
    free(temp);
    
    DFUTF16BufferDestroy(&ucs2);
    DFUTF16BufferDestroy(&utf8);
}

NSString *DFFixStringEncoding(NSString *str)
{
    return DFCharEncodingFixNSString(str);
}

NSString *NSStringFromC(const char *cstr)
{
    if (cstr == NULL)
        return NULL;
    else
        return [NSString stringWithUTF8String: cstr];
}

void NSErrorSetFromDFError(NSError **nserror, DFError *dferror)
{
    if ((nserror == NULL) || (dferror == NULL))
        return;
    [NSError set: nserror format: @"%@", NSStringFromC(DFErrorMessage(&dferror))];
}

void DFErrorReleaseToNSError(DFError *dferror, NSError **nserror)
{
    NSErrorSetFromDFError(nserror,dferror);
}

void DFErrorSetFromNSError(DFError **dferror, NSError *nserror)
{
    if ((nserror == NULL) || (dferror == NULL))
        return;
    DFErrorFormat(dferror,"%s",FCErrorDescription(nserror).UTF8String);
}

BOOL EDStringEquals(NSString *a, NSString *b)
{
    if ((a == NULL) && (b == NULL))
        return YES;
    else if ((a != NULL) && (b != NULL))
        return [a isEqualToString: b];
    else
        return NO;
}

NSString *EDEncodeFontFamily(NSString *input)
{
    char *cresult = CSSEncodeFontFamily(input.UTF8String);
    NSString *nsresult = NSStringFromC(cresult);
    free(cresult);
    return nsresult;
}

NSString *EDDecodeFontFamily(NSString *input)
{
    char *cresult = CSSDecodeFontFamily(input.UTF8String);
    NSString *nsresult = NSStringFromC(cresult);
    free(cresult);
    return nsresult;
}
