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
#include "WordRPr.h"
#include "WordStyles.h"
#include "WordTheme.h"
#include "CSS.h"
#include "DFHTML.h"
#include "DFString.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

static Tag WordRPR_Children[] = {
    WORD_RSTYLE,
    WORD_RFONTS,
    WORD_B,
    WORD_BCS,
    WORD_I,
    WORD_ICS,
    WORD_CAPS,
    WORD_SMALLCAPS,
    WORD_STRIKE,
    WORD_DSTRIKE,
    WORD_OUTLINE,
    WORD_SHADOW,
    WORD_EMBOSS,
    WORD_IMPRINT,
    WORD_NOPROOF,
    WORD_SNAPTOGRID,
    WORD_VANISH,
    WORD_WEBHIDDEN,
    WORD_COLOR,
    WORD_SPACING,
    WORD_W,
    WORD_KERN,
    WORD_POSITION,
    WORD_SZ,
    WORD_SZCS,
    WORD_HIGHLIGHT,
    WORD_U,
    WORD_EFFECT,
    WORD_BDR,
    WORD_SHD,
    WORD_FITTEXT,
    WORD_VERTALIGN,
    WORD_RTL,
    WORD_CS,
    WORD_EM,
    WORD_LANG,
    WORD_EASTASIANLAYOUT,
    WORD_SPECVANISH,
    WORD_OMATH,
    WORD_RPRCHANGE,
    0,
};

static const char *substituteFontToWord(const char *fontName)
{
    if (DFStringEquals(fontName,"serif"))
        return "Times";
    if (DFStringEquals(fontName,"sans-serif"))
        return "Helvetica";
    if (DFStringEquals(fontName,"cursive"))
        return "Comic Sans MS";
    if (DFStringEquals(fontName,"fantasy"))
        return "Algerian";
    if (DFStringEquals(fontName,"monospace"))
        return "Courier";
    return fontName;
}

static const char *substituteFontFromWord(const char *fontName)
{
    if (DFStringEquals(fontName,"Times"))
        return "serif";
    if (DFStringEquals(fontName,"Helvetica"))
        return "sans-serif";
    if (DFStringEquals(fontName,"Comic Sans MS"))
        return "cursive";
    if (DFStringEquals(fontName,"Algerian"))
        return "fantasy";
    if (DFStringEquals(fontName,"Courier"))
        return "monospace";
    return fontName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                  Run properties (rPr element)                                  //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static struct {
    const char *highlight;
    const char *color;
} highlightColors[] = {
    { "black", "#000000", },
    { "blue", "#0000FF", },
    { "cyan", "#00FFFF", },
    { "darkBlue", "#00008B", },
    { "darkCyan", "#008B8B", },
    { "darkGray", "#A9A9A9", },
    { "darkGreen", "#006400", },
    { "darkMagenta", "#800080", },
    { "darkRed", "#8B0000", },
    { "darkYellow", "#808000", },
    { "green", "#00FF00", },
    { "lightGray", "#D3D3D3", },
    { "magenta", "#FF00FF", },
    //    { "none", "#", },
    { "red", "#FF0000", },
    { "white", "#FFFFFF", },
    { "yellow", "#FFFF00", },
    { NULL, NULL },
};

// FIXME: when setting shading, clear the highlight
static void highlightGet(DFNode *concrete, CSSProperties *properties)
{
    const char *val = DFGetAttribute(concrete,WORD_VAL);
    if (val == NULL)
        return;
    for (int i = 0; highlightColors[i].highlight != NULL; i++) {
        if (!strcmp(val,highlightColors[i].highlight)) {
            CSSPut(properties,"background-color",highlightColors[i].color);
            break;
        }
    }
}

void WordGetRPr(DFNode *concrete, CSSProperties *properties, const char **styleId, struct WordTheme *theme)
{
    if (styleId != NULL)
        *styleId = NULL;
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_RSTYLE:
                if (styleId != NULL)
                    *styleId = DFGetAttribute(child,WORD_VAL);
                break;
            case WORD_B:
            case WORD_BCS: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val == NULL) || Word_parseOnOff(val))
                    CSSPut(properties,"font-weight","bold");
                else
                    CSSPut(properties,"font-weight","normal");
                break;
            }
            case WORD_I:
            case WORD_ICS: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val == NULL) || Word_parseOnOff(val))
                    CSSPut(properties,"font-style","italic");
                else
                    CSSPut(properties,"font-style","normal");
                break;
            }
            case WORD_CAPS: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val == NULL) || Word_parseOnOff(val))
                    CSSPut(properties,"text-transform","uppercase");
                else
                    CSSPut(properties,"text-transform","none");
                break;
            }
            case WORD_SMALLCAPS: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val == NULL) || Word_parseOnOff(val))
                    CSSPut(properties,"font-variant","small-caps");
                else
                    CSSPut(properties,"font-variant","normal");
                break;
            }
            case WORD_U: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val != NULL) && !strcasecmp(val,"none"))
                    CSSSetUnderline(properties,0);
                else
                    CSSSetUnderline(properties,1);
                break;
            }
            case WORD_STRIKE:
            case WORD_DSTRIKE: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if (Word_parseOnOff(val))
                    CSSSetLinethrough(properties,1);
                else
                    CSSSetLinethrough(properties,0);
                break;
            }
            case WORD_VERTALIGN: {
                const char *val = DFGetAttribute(child,WORD_VAL);
                if ((val != NULL) && !strcmp(val,"subscript"))
                    CSSPut(properties,"vertical-align","sub");
                else if ((val != NULL) && !strcmp(val,"superscript"))
                    CSSPut(properties,"vertical-align","super");
                break;
            }
            case WORD_COLOR: {
                const char *value = DFGetAttribute(child,WORD_VAL);
                if ((value != NULL) && isRRGGBB(value)) {
                    char *cssValue = DFFormatString("#%s",value);
                    CSSPut(properties,"color",cssValue);
                    free(cssValue);
                }
                break;
            }
            case WORD_SZ: {
                const char *value = DFGetAttribute(child,WORD_VAL);
                // units: 1/2 pt
                if (value != NULL) {
                    char buf[100];
                    CSSPut(properties,"font-size",DFFormatDoublePt(buf,100,atof(value)/2));
                }
                break;
            }
            case WORD_RFONTS: {
                const char *fontFamily;

                fontFamily = DFGetAttribute(child,WORD_ASCII);
                if (fontFamily == NULL)
                    fontFamily = DFGetAttribute(child,WORD_HANSI);
                if (fontFamily == NULL)
                    fontFamily = DFGetAttribute(child,WORD_EASTASIA);
                if (fontFamily == NULL)
                    fontFamily = DFGetAttribute(child,WORD_CS);

                CSSPut(properties,"font-family",substituteFontFromWord(fontFamily));

                const char *asciiTheme = DFGetAttribute(child,WORD_ASCIITHEME);
                if (asciiTheme != NULL) {
                    if (!strncmp(asciiTheme,"major",5) && (theme->majorFont != NULL))
                        CSSPut(properties,"font-family",theme->majorFont);
                    else if (!strncmp(asciiTheme,"minor",5) && (theme->minorFont != NULL))
                        CSSPut(properties,"font-family",theme->minorFont);
                }

                char *encodedFontFamily = CSSEncodeFontFamily(CSSGet(properties,"font-family"));
                CSSPut(properties,"font-family",encodedFontFamily);
                free(encodedFontFamily);
                break;
            }
            case WORD_SHD:
                WordGetShd(child,properties);
                break;
            case WORD_HIGHLIGHT:
                highlightGet(child,properties);
                break;
        }
    }
}

void WordPutRPr(DFNode *concrete, CSSProperties *newp, const char *newStyleId, struct WordTheme *theme)
{
    DFNode *children[PREDEFINED_TAG_COUNT];
    childrenToArray(concrete,children);

    CSSProperties *oldp = CSSPropertiesNew();
    const char *oldStyleId = NULL;
    WordGetRPr(concrete,oldp,&oldStyleId,theme);

    // Style name
    if (!DFStringEquals(oldStyleId,newStyleId)) {
        if (newStyleId != NULL) {
            children[WORD_RSTYLE] = DFCreateElement(concrete->doc,WORD_RSTYLE);
            DFSetAttribute(children[WORD_RSTYLE],WORD_VAL,newStyleId);
        }
        else {
            children[WORD_RSTYLE] = NULL;
        }
    }

    // Font weight (bold/normal)
    if (!DFStringEquals(CSSGet(oldp,"font-weight"),CSSGet(newp,"font-weight"))) {
        if (DFStringEquals(CSSGet(newp,"font-weight"),"bold")) {
            children[WORD_B] = DFCreateElement(concrete->doc,WORD_B);
            children[WORD_BCS] = DFCreateElement(concrete->doc,WORD_BCS);
        }
        else {
            children[WORD_B] = NULL;
            children[WORD_BCS] = NULL;
        }
    }

    // Font style (italic/normal)
    if (!DFStringEquals(CSSGet(oldp,"font-style"),CSSGet(newp,"font-style"))) {
        if (DFStringEquals(CSSGet(newp,"font-style"),"italic")) {
            children[WORD_I] = DFCreateElement(concrete->doc,WORD_I);
            children[WORD_ICS] = DFCreateElement(concrete->doc,WORD_ICS);
        }
        else {
            children[WORD_I] = NULL;
            children[WORD_ICS] = NULL;
        }
    }

    // Text transform (uppercase/normal)
    if (!DFStringEquals(CSSGet(oldp,"text-transform"),CSSGet(newp,"text-transform"))) {
        if (DFStringEquals(CSSGet(newp,"text-transform"),"uppercase")) {
            children[WORD_CAPS] = DFCreateElement(concrete->doc,WORD_CAPS);
        }
        else if (DFStringEquals(CSSGet(newp,"text-transform"),"none")) {
            children[WORD_CAPS] = DFCreateElement(concrete->doc,WORD_CAPS);
            DFSetAttribute(children[WORD_CAPS],WORD_VAL,"false");
        }
        else {
            children[WORD_CAPS] = NULL;
        }
    }

    // Font variant (small-caps/normal)
    if (!DFStringEquals(CSSGet(oldp,"font-variant"),CSSGet(newp,"font-variant"))) {
        if (DFStringEquals(CSSGet(newp,"font-variant"),"small-caps")) {
            children[WORD_SMALLCAPS] = DFCreateElement(concrete->doc,WORD_SMALLCAPS);
        }
        else if (DFStringEquals(CSSGet(newp,"font-variant"),"normal")) {
            children[WORD_SMALLCAPS] = DFCreateElement(concrete->doc,WORD_SMALLCAPS);
            DFSetAttribute(children[WORD_SMALLCAPS],WORD_VAL,"false");
        }
        else {
            children[WORD_SMALLCAPS] = NULL;
        }
    }

    // Underline
    if (CSSGetUnderline(oldp) != CSSGetUnderline(newp)) {
        if (CSSGetUnderline(newp)) {
            children[WORD_U] = DFCreateElement(concrete->doc,WORD_U);
            DFSetAttribute(children[WORD_U],WORD_VAL,"single");
        }
        else {
            children[WORD_U] = NULL;
        }
    }

    // Strikethrough
    if (CSSGetLinethrough(oldp) != CSSGetLinethrough(newp)) {
        if (CSSGetLinethrough(newp))
            children[WORD_STRIKE] = DFCreateElement(concrete->doc,WORD_STRIKE);
        else
            children[WORD_STRIKE] = NULL;
        children[WORD_DSTRIKE] = NULL;
    }

    // Vertical alignment (subscript/superscript)
    if (!DFStringEquals(CSSGet(oldp,"vertical-align"),CSSGet(newp,"vertical-align"))) {
        if (DFStringEquals(CSSGet(newp,"vertical-align"),"sub")) {
            children[WORD_VERTALIGN] = DFCreateElement(concrete->doc,WORD_VERTALIGN);
            DFSetAttribute(children[WORD_VERTALIGN],WORD_VAL,"subscript");
        }
        else if (DFStringEquals(CSSGet(newp,"vertical-align"),"sup")) {
            children[WORD_VERTALIGN] = DFCreateElement(concrete->doc,WORD_VERTALIGN);
            DFSetAttribute(children[WORD_VERTALIGN],WORD_VAL,"superscript");
        }
        else {
            children[WORD_VERTALIGN] = NULL;
        }
    }

    // Text color
    char *oldColor = CSSHexColor(CSSGet(oldp,"color"),0);
    char *newColor = CSSHexColor(CSSGet(newp,"color"),0);
    if (!DFStringEquals(oldColor,newColor)) {
        if (newColor != NULL) {
            children[WORD_COLOR] = DFCreateElement(concrete->doc,WORD_COLOR);
            DFSetAttribute(children[WORD_COLOR],WORD_VAL,newColor);
        }
        else {
            children[WORD_COLOR] = NULL;
        }
    }
    free(oldColor);
    free(newColor);

    // background-color

    char *oldBackgroundColor = CSSHexColor(CSSGet(oldp,"background-color"),0);
    char *newBackgroundColor = CSSHexColor(CSSGet(newp,"background-color"),0);
    if (!DFStringEquals(oldBackgroundColor,newBackgroundColor)) {
        children[WORD_HIGHLIGHT] = NULL;
        WordPutShd(concrete->doc,&children[WORD_SHD],newBackgroundColor);
    }
    free(oldBackgroundColor);
    free(newBackgroundColor);

    // Font size
    // units: 1/2 pt
    if (!DFStringEquals(CSSGet(oldp,"font-size"),CSSGet(newp,"font-size"))) {
        children[WORD_SZ] = NULL;
        children[WORD_SZCS] = NULL;
        if (CSSGet(newp,"font-size") != NULL) {
            CSSLength length = CSSLengthFromString(CSSGet(newp,"font-size"));
            if (CSSLengthIsValid(length) && (length.units != UnitsPct)) {
                double dval = convertBetweenUnits(length.value,length.units,UnitsPt);
                int ival = (int)dval;
                children[WORD_SZ] = DFCreateElement(concrete->doc,WORD_SZ);
                children[WORD_SZCS] = DFCreateElement(concrete->doc,WORD_SZCS);

                DFFormatAttribute(children[WORD_SZ],WORD_VAL,"%d",ival*2);
                DFFormatAttribute(children[WORD_SZCS],WORD_VAL,"%d",ival*2);
            }
        }
    }

    // Font family
    char *oldFontFamily = CSSDecodeFontFamily(CSSGet(oldp,"font-family"));
    char *newFontFamily = CSSDecodeFontFamily(CSSGet(newp,"font-family"));
    if (!DFStringEquals(oldFontFamily,newFontFamily)) {
        const char *subst = substituteFontToWord(newFontFamily);
        if (subst != NULL) {
            children[WORD_RFONTS] = DFCreateElement(concrete->doc,WORD_RFONTS);
            DFSetAttribute(children[WORD_RFONTS],WORD_ASCII,subst);
            DFSetAttribute(children[WORD_RFONTS],WORD_HANSI,subst);
            DFSetAttribute(children[WORD_RFONTS],WORD_EASTASIA,subst);
            DFSetAttribute(children[WORD_RFONTS],WORD_CS,subst);
        }
        else {
            children[WORD_RFONTS] = NULL;
        }
    }
    free(oldFontFamily);
    free(newFontFamily);
    
    replaceChildrenFromArray(concrete,children,WordRPR_Children);
    CSSPropertiesRelease(oldp);
}
