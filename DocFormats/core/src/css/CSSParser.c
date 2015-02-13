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

#include "CSSParser.h"
#include "CSS.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int matchPast(CSSParser *p, uint16_t endChar, int *invalid);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSParser                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CSSParser {
    char *chars;
    size_t length;
    size_t pos;
};

static void CSSParserSetError(CSSParser *p, const char *format, ...)
{
    // This isn't really needed
}

CSSParser *CSSParserNew(const char *cinput)
{
    CSSParser *p = (CSSParser *)calloc(1,sizeof(CSSParser));
    if (cinput == NULL)
        cinput = "";

    p->chars = strdup(cinput);
    p->length = strlen(cinput);

    return p;
}

void CSSParserFree(CSSParser *p)
{
    free(p->chars);
    free(p);
}

static int match(CSSParser *p, uint16_t c)
{
    if ((p->pos < p->length) && (p->chars[p->pos] == c)) {
        p->pos++;
        return 1;
    }
    else {
        return 0;
    }
}

static int matchTwo(CSSParser *p, uint16_t c1, uint16_t c2)
{
    if ((p->pos+1 < p->length) && (p->chars[p->pos] == c1) && (p->chars[p->pos+1] == c2)) {
        p->pos += 2;
        return 1;
    }
    else {
        return 0;
    }
}

static int matchEscape(CSSParser *p)
{
    if (match(p,'\\')) {
        int hexCount = 0;
        for (; hexCount < 6; hexCount++) {
            if ((p->pos < p->length) && DFCharIsHex(p->chars[p->pos])) {
                hexCount++;
                p->pos++;
            }
        }
        if (hexCount > 0) {
            if (matchTwo(p,'\r','\n'))
                return 1;
            else if (match(p,' '))
                return 1;
            else if (match(p,'\t'))
                return 1;
            else if (match(p,'\r'))
                return 1;
            else if (match(p,'\n'))
                return 1;
            else if (match(p,'\f'))
                return 1;
            else
                return 1;
        }
        else {
            p->pos++;
            return 1;
        }
    }
    else {
        return 0;
    }
}

static int matchString(CSSParser *p, int *invalid)
{
    uint16_t closeChar = 0;

    if (match(p,'"'))
        closeChar = '"';
    else if (match(p,'\''))
        closeChar = '\'';
    else
        return 0;

    while (p->pos < p->length) {
        if (match(p,'\n')) {
            *invalid = 1;
            return 1;
        }
        else if (matchEscape(p)) {
            // nothing to do; pos already advanced
        }
        else if (match(p,closeChar)) {
            return 1;
        }
        else {
            p->pos++;
        }
    }

    *invalid = 1;
    return 1;
}

static int matchGeneric(CSSParser *p, int *invalid)
{
    if (p->pos >= p->length)
        return 1;

    switch (p->chars[p->pos]) {
        case '"':
        case '\'':
            return matchString(p,invalid);
        case '(':
            p->pos++;
            return matchPast(p,')',invalid);
        case '[':
            p->pos++;
            return matchPast(p,']',invalid);
        case '{':
            p->pos++;
            return matchPast(p,'}',invalid);
        default:
            p->pos++;
            return 1;
    }
}

static int matchPast(CSSParser *p, uint16_t endChar, int *invalid)
{
    while (p->pos < p->length) {
        if (match(p,endChar))
            return 1;
        if (!matchGeneric(p,invalid))
            return 0;
    }
    CSSParserSetError(p,"Missing %c", (char)endChar);
    return 0;
}

static int matchBefore(CSSParser *p, uint16_t terminator, int *invalid)
{
    while (p->pos < p->length) {
        if (p->chars[p->pos] == terminator) {
            return 1;
        }
        else if (!matchGeneric(p,invalid))
            return 0;
    }
    return 1;
}

static char *trimmedSubstring(CSSParser *p, size_t start, size_t pos)
{
    char *untrimmed = DFSubstring(p->chars,start,pos);
    char *trimmed = DFStringTrimWhitespace(untrimmed);
    free(untrimmed);
    return trimmed;
}

DFHashTable *CSSParserRules(CSSParser *p)
{
    DFHashTable *result = DFHashTableNew((DFCopyFunction)strdup,free);
    while (p->pos < p->length) {
        size_t start = p->pos;
        int invalid = 0;
        if (!matchBefore(p,'{',&invalid))
            return result;
        char *selectors = trimmedSubstring(p,start,p->pos);
        if (strlen(selectors) == 0) {
            free(selectors);
            break;
        }
        if (!match(p,'{')) {
            CSSParserSetError(p,"Expected {");
            free(selectors);
            return result;
        }
        start = p->pos;
        if (!matchBefore(p,'}',&invalid)) {
            free(selectors);
            return result;
        }
        char *ruleBody = trimmedSubstring(p,start,p->pos);
        if (!match(p,'}')) {
            CSSParserSetError(p,"Expected }");
        }

        DFHashTableAdd(result,selectors,ruleBody);
        free(selectors);
        free(ruleBody);
    }
    return result;
}

DFArray *CSSParserSelectors(CSSParser *p)
{
    DFArray *result = DFArrayNew((DFCopyFunction)strdup,free);
    while (p->pos < p->length) {
        size_t start = p->pos;
        int invalid = 0;
        if (!matchBefore(p,',',&invalid))
            return 0;
        char *selector = trimmedSubstring(p,start,p->pos);
        if (strlen(selector) == 0) {
            free(selector);
            continue;
        }
        DFArrayAppend(result,selector);
        match(p,',');
        free(selector);
    }
    return result;
}

DFHashTable *CSSParserProperties(CSSParser *p)
{
    DFHashTable *result = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    while (p->pos < p->length) {
        size_t start = p->pos;
        int invalid = 0;
        if (!matchBefore(p,':',&invalid)) {
            DFHashTableRelease(result);
            return NULL;
        }
        char *name = trimmedSubstring(p,start,p->pos);
        if (strlen(name) == 0) {
            free(name);
            break;
        }
        if (!match(p,':')) {
            CSSParserSetError(p,"Missing :");
            DFHashTableRelease(result);
            free(name);
            return NULL;
        }
        start = p->pos;
        if (!matchBefore(p,';',&invalid)) {
            DFHashTableRelease(result);
            free(name);
            return NULL;
        }
        if (!invalid) {
            char *value = trimmedSubstring(p,start,p->pos);
            DFHashTableAdd(result,name,value);
            free(value);
        }
        match(p,';');
        free(name);
    }
    return result;
}

static int matchSpecific(CSSParser *p, const char *string)
{
    size_t start = p->pos;
    size_t stringlen = strlen(string);
    for (size_t i = 0; i < stringlen; i++) {
        if (p->pos == p->length) {
            p->pos = start;
            return 0;
        }
        if ((p->chars[p->pos] != tolower(string[i])) &&
            (p->chars[p->pos] != toupper(string[i]))) {
            p->pos = start;
            return 0;
        }
        p->pos++;
    }
    return 1;
}

static int matchIdent(CSSParser *p)
{
    size_t start = p->pos;
    while (p->pos < p->length) {
        if (((p->chars[p->pos] >= 'a') && (p->chars[p->pos] <= 'z')) ||
            ((p->chars[p->pos] >= 'A') && (p->chars[p->pos] <= 'Z')) ||
            (p->chars[p->pos] == '_') ||
            ((p->pos > start) && (p->chars[p->pos] == '-')) ||
            ((p->pos > start) && (p->chars[p->pos] >= '0') && (p->chars[p->pos <= '9']))) {
            p->pos++;
        }
        else {
            break;
        }
    }
    return (p->pos > start);
}

static int matchWhitespace(CSSParser *p)
{
    size_t start = p->pos;
    while ((p->pos < p->length) && isspace(p->chars[p->pos]))
        p->pos++;
    return (p->pos > start);
}

DFArray *CSSParserContent(CSSParser *p)
{
    DFArray *result = DFArrayNew((DFCopyFunction)ContentPartRetain,(DFFreeFunction)ContentPartRelease);
    matchWhitespace(p);
    while (p->pos < p->length) {
        size_t start = p->pos;
        switch (p->chars[p->pos]) {
            case '"':
            case '\'': {
                int invalid = 0;
                if (!matchString(p,&invalid))
                    return result;
                char *quotedValue = trimmedSubstring(p,start,p->pos);
                char *unquotedValue = DFUnquote(quotedValue);
                ContentPart *part = ContentPartNew(ContentPartString,unquotedValue,NULL);
                DFArrayAppend(result,part);
                ContentPartRelease(part);
                free(quotedValue);
                free(unquotedValue);
                if ((p->pos < p->length) && !matchWhitespace(p))
                    return result;
                break;
            }
            case 'C':
            case 'c': {
                if (matchSpecific(p,"counters(")) {
                    // Not yet supported
                    return result;
                }
                else if (matchSpecific(p,"counter(")) {
                    size_t nameStart = p->pos;
                    if (!matchIdent(p))
                        return result;
                    char *name = DFSubstring(p->chars,nameStart,p->pos);

                    char *type = NULL;
                    if (match(p,',')) {
                        size_t typeStart = p->pos;
                        if (!matchIdent(p)) {
                            free(name);
                            free(type);
                            return result;
                        }
                        type = DFSubstring(p->chars,typeStart,p->pos);
                    }

                    ContentPart *part = ContentPartNew(ContentPartCounter,name,type);
                    DFArrayAppend(result,part);
                    ContentPartRelease(part);

                    if (!match(p,')')) {
                        free(name);
                        free(type);
                        return result;
                    }

                    if ((p->pos < p->length) && !matchWhitespace(p)) {
                        free(name);
                        free(type);
                        return result;
                    }

                    free(name);
                    free(type);
                }
                else {
                    return result;
                }
                break;
            }
            default:
                return result;
        }
    }
    return result;
}
