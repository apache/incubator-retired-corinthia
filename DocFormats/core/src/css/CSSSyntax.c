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

#include "CSSSyntax.h"
#include "DFCommon.h"
#include <string.h>

static int matchString(const char **strptr, const char *against)
{
    const char *str = *strptr;
    while (*against != 0) {
        if (*str != *against)
            return 0;
        str++;
        against++;
    }
    *strptr = str;
    return 1;
}

static int matchDigitPlus(const char **strptr)
{
    const char *str = *strptr;
    int numDigits = 0;
    while ((*str >= '0') && (*str <= '9')) {
        str++;
        numDigits++;
    }
    if (numDigits == 0)
        return 0;
    *strptr = str;
    return 1;
}

static int matchNumber(const char **strptr)
{
    const char *str = *strptr;
    if ((*str == '-') || (*str == '+'))
        str++;
    if (!matchDigitPlus(&str))
        return 0;
    if (*str == '.') {
        str++;
        if (!matchDigitPlus(&str))
            return 0;
    }
    *strptr = str;
    return 1;
}

static int matchUnit(const char **strptr)
{
    return (matchString(strptr,"em") ||
            matchString(strptr,"ex") ||
            matchString(strptr,"in") ||
            matchString(strptr,"cm") ||
            matchString(strptr,"mm") ||
            matchString(strptr,"pt") ||
            matchString(strptr,"pc") ||
            matchString(strptr,"px") ||
            matchString(strptr,"%"));
}

static int matchColorName(const char **strptr)
{
    return (matchString(strptr,"maroon") ||
            matchString(strptr,"red") ||
            matchString(strptr,"orange") ||
            matchString(strptr,"yellow") ||
            matchString(strptr,"olive") ||
            matchString(strptr,"purple") ||
            matchString(strptr,"fuchsia") ||
            matchString(strptr,"white") ||
            matchString(strptr,"lime") ||
            matchString(strptr,"green") ||
            matchString(strptr,"navy") ||
            matchString(strptr,"blue") ||
            matchString(strptr,"aqua") ||
            matchString(strptr,"teal") ||
            matchString(strptr,"black") ||
            matchString(strptr,"silver") ||
            matchString(strptr,"gray"));
}

static int matchHexColor(const char **strptr)
{
    const char *str = *strptr;
    if (*str != '#')
        return 0;
    str++;
    int numDigits = 0;
    while (((*str >= '0') && (*str <= '9')) ||
           ((*str >= 'a') && (*str <= 'f')) ||
           ((*str >= 'A') && (*str <= 'F'))) {
        str++;
        numDigits++;
    }
    if (numDigits == 0)
        return 0;
    *strptr = str;
    return 1;
}

static int matchRgbColor(const char **strptr)
{
    const char *str = *strptr;
    if (!matchString(&str,"rgb("))
        return 0;
    if (!matchDigitPlus(&str))
        return 0;
    if (!matchString(&str,","))
        return 0;
    if (!matchDigitPlus(&str))
        return 0;
    if (!matchString(&str,","))
        return 0;
    if (!matchDigitPlus(&str))
        return 0;
    if (!matchString(&str,")"))
        return 0;
    *strptr = str;
    return 1;
}

static int matchColor(const char **strptr)
{
    return (matchHexColor(strptr) || matchRgbColor(strptr) || matchColorName(strptr));
}

int CSSValueIsNumber(const char *str)
{
    if (str == NULL)
        return 0;
    return (matchNumber(&str) && (*str == 0));
}

int CSSValueIsLength(const char *str)
{
    if (str == NULL)
        return 0;
    return (matchNumber(&str) && matchUnit(&str) && (*str == 0));
}

int CSSValueIsColor(const char *str)
{
    if (str == NULL)
        return 0;
    return (matchColor(&str) && (*str == 0));
}

int CSSValueIsBorderStyle(const char *str)
{
    if (str == NULL)
        return 0;
    return (!strcmp(str,"none") ||
            !strcmp(str,"hidden") ||
            !strcmp(str,"dotted") ||
            !strcmp(str,"dashed") ||
            !strcmp(str,"solid") ||
            !strcmp(str,"double") ||
            !strcmp(str,"groove") ||
            !strcmp(str,"ridge") ||
            !strcmp(str,"inset") ||
            !strcmp(str,"outset"));
}

int CSSValueIsBorderWidth(const char *str)
{
    if (str == NULL)
        return 0;
    return (!strcmp(str,"thin") || !strcmp(str,"medium") || !strcmp(str,"thick") || CSSValueIsLength(str));
}

int CSSValueIsBorderColor(const char *str)
{
    if (str == NULL)
        return 0;
    return (!strcmp(str,"transparent") || CSSValueIsColor(str));
}
