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
#include "WordCommonPr.h"
#include "WordStyles.h"
#include "WordSheet.h"
#include "CSS.h"
#include "DFDOM.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

int Word_parseOnOff(const char *value)
{
    return ((value == NULL) ||
            !strcmp(value,"true") ||
            !strcmp(value,"on") ||
            !strcmp(value,"1"));
}

char *twipsFromCSS(const char *str, int relativeTwips)
{
    CSSLength length = CSSLengthFromString(str);
    if (!CSSLengthIsValid(length))
        return NULL;
    switch (length.units) {
        case UnitsIn:
        case UnitsCm:
        case UnitsMm:
        case UnitsPt:
        case UnitsPc:
        case UnitsPx: {
            double pts = convertBetweenUnits(length.value,length.units,UnitsPt);
            int twips = (int)round(pts*20);
            return DFFormatString("%d",twips);
        }
        case UnitsPct: {
            int twips = (int)round((length.value/100.0)*relativeTwips);
            return DFFormatString("%d",twips);
        }
        default:
            return NULL;
    }
}

void updateTwipsFromLength(DFNode *element, Tag attr, const char *value, int relativeTwips)
{
    CSSLength length = CSSLengthFromString(value);
    double relativePts = relativeTwips/20.0;
    if (CSSLengthIsValid(length)) {
        double pts = CSSLengthToPts(length,relativePts);
        int twips = (int)round(pts*20);
        DFFormatAttribute(element,attr,"%d",twips);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                             Border and shading properties (common)                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void WordGetShd(DFNode *concrete, CSSProperties *properties)
{
    const char *fill = DFGetAttribute(concrete,WORD_FILL);
    if (isRRGGBB(fill)) {
        char *value = DFFormatString("#%s",fill);
        CSSPut(properties,"background-color",value);
        free(value);
    }
}

void WordPutShd(DFDocument *doc, DFNode **shd, const char *hexColor)
{
    if (hexColor == NULL) {
        *shd = NULL;
    }
    else {
        *shd = DFCreateElement(doc,WORD_SHD);
        DFSetAttribute(*shd,WORD_FILL,hexColor);
        DFSetAttribute(*shd,WORD_VAL,"clear");
    }
}

void WordGetBorder(DFNode *concrete, const char *side, CSSProperties *properties)
{
    const char *val = DFGetAttribute(concrete,WORD_VAL);
    const char *sz = DFGetAttribute(concrete,WORD_SZ); // units: 1/8th of a point
    //    const char *space = DFGetAttribute(concrete,WORD_SPACE);
    const char *color = DFGetAttribute(concrete,WORD_COLOR);
    if (val == NULL)
        return;

    char *widthName = DFFormatString("border-%s-width",side);
    char *styleName = DFFormatString("border-%s-style",side);
    char *colorName = DFFormatString("border-%s-color",side);

    double szPts = 0;
    if (sz != NULL)
        szPts = atof(sz)/8.0;
    if (szPts < 1.0) {
        CSSPut(properties,widthName,"thin");
    }
    else {
        char buf[100];
        CSSPut(properties,widthName,DFFormatDoublePt(buf,100,szPts));
    }
    CSSPut(properties,styleName,"solid");

    char *value;
    if ((color != NULL) && isRRGGBB(color))
        value = DFFormatString("#%s",color);
    else
        value = strdup("black");
    CSSPut(properties,colorName,value);
    free(value);

    free(widthName);
    free(styleName);
    free(colorName);
}

void WordPutBorder(DFDocument *doc, CSSProperties *oldp, CSSProperties *newp, DFNode **childp,
                   Tag tag, const char *side)
{
    char *widthName = DFFormatString("border-%s-width",side);
    char *styleName = DFFormatString("border-%s-style",side);
    char *colorName = DFFormatString("border-%s-color",side);

    const char *oldWidth = CSSGet(oldp,widthName);
    const char *oldStyle = CSSGet(oldp,styleName);
    const char *oldColor = CSSGet(oldp,colorName);

    const char *newWidth = CSSGet(newp,widthName);
    const char *newStyle = CSSGet(newp,styleName);
    const char *newColor = CSSGet(newp,colorName);

    if (!DFStringEquals(oldWidth,newWidth) ||
        !DFStringEquals(oldStyle,newStyle) ||
        !DFStringEquals(oldColor,newColor)) {

        if ((newWidth == NULL) &&
            ((newStyle == NULL) || !strcmp(newStyle,"hidden")) &&
            (newColor == NULL)) {
            *childp = NULL;
        }
        else {
            *childp = DFCreateElement(doc,tag);
            if (newColor != NULL) {
                char *newHexColor = CSSHexColor(newColor,1);
                if (isHashRRGGBB(newHexColor)) {
                    const char *color = &newHexColor[1];
                    DFSetAttribute(*childp,WORD_COLOR,color);
                }
                else if (newHexColor != NULL) {
                    DFSetAttribute(*childp,WORD_COLOR,"auto");
                }
                free(newHexColor);
            }

            DFSetAttribute(*childp,WORD_VAL,"single");

            if ((newWidth == NULL) || DFStringEquals(newWidth,"thin"))
                newWidth = "0.5pt";
            else if (DFStringEquals(newWidth,"medium"))
                newWidth = "2.5pt";
            else if (DFStringEquals(newWidth,"thick"))
                newWidth = "4pt";

            double pts = 0.0;
            CSSLength length = CSSLengthFromString(newWidth);
            if (CSSLengthIsValid(length)) {
                pts = convertBetweenUnits(length.value,length.units,UnitsPt);
            }

            if (pts < 0.5)
                pts = 0.5;

            // sz: measurements: 8ths of a point
            int sz = (int)round(pts*8);
            DFFormatAttribute(*childp,WORD_SZ,"%d",sz);
        }
    }

    free(widthName);
    free(styleName);
    free(colorName);
}
