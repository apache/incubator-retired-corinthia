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

#include "CSS.h"
#include "CSSParser.h"
#include "CSSProperties.h"
#include "CSSSyntax.h"
#include "CSSSelector.h"
#include "DFBuffer.h"
#include "DFString.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *left;
    char *right;
    char *top;
    char *bottom;
} SideValues;

SideValues SideValuesEmpty = { NULL, NULL, NULL, NULL };

static void SideValuesClear(SideValues *values)
{
    free(values->left);
    free(values->right);
    free(values->top);
    free(values->bottom);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           ContentPart                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

const char *ContentPartTypeString(ContentPartType type)
{
    switch (type) {
        case ContentPartNormal:
            return "Normal";
        case ContentPartNone:
            return "None";
        case ContentPartString:
            return "String";
        case ContentPartURI:
            return "URI";
        case ContentPartCounter:
            return "Counter";
        case ContentPartCounters:
            return "Counters";
        case ContentPartAttr:
            return "Attr";
        case ContentPartOpenQuote:
            return "OpenQuote";
        case ContentPartCloseQuote:
            return "CloseQuote";
        case ContentPartNoOpenQuote:
            return "NoOpenQuote";
        case ContentPartNoCloseQuote:
            return "NoCloseQuote";
        default:
            return "?";
    }
}

ContentPart *ContentPartNew(ContentPartType type, const char *value, const char *arg)
{
    ContentPart *part = (ContentPart *)calloc(1,sizeof(ContentPart));
    part->retainCount = 1;
    part->type = type;
    part->value = DFStrDup(value);
    part->arg = DFStrDup(arg);
    return part;
}

ContentPart *ContentPartRetain(ContentPart *part)
{
    if (part != NULL)
        part->retainCount++;
    return part;
}

void ContentPartRelease(ContentPart *part)
{
    if ((part == NULL) || (--part->retainCount > 0))
        return;

    free(part->value);
    free(part->arg);
    free(part);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          ListStyleType                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

const char *ListStyleTypeString(ListStyleType type)
{
    switch (type) {
        case ListStyleTypeDisc:
            return "disc";
        case ListStyleTypeCircle:
            return "circle";
        case ListStyleTypeSquare:
            return "square";
        case ListStyleTypeDecimal:
            return "decimal";
        case ListStyleTypeDecimalLeadingZero:
            return "decimal-leading-zero";
        case ListStyleTypeLowerRoman:
            return "lower-roman";
        case ListStyleTypeUpperRoman:
            return "upper-roman";
        case ListStyleTypeLowerGreek:
            return "lower-greek";
        case ListStyleTypeLowerLatin:
            return "lower-latin";
        case ListStyleTypeUpperLatin:
            return "upper-latin";
        case ListStyleTypeArmenian:
            return "armenian";
        case ListStyleTypeGeorgian:
            return "georgian";
        case ListStyleTypeLowerAlpha:
            return "lower-alpha";
        case ListStyleTypeUpperAlpha:
            return "upper-alpha";
        case ListStyleTypeNone:
        default:
            return "none";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                               CSS                                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFHashTable *CSSParseProperties(const char *input)
{
    CSSParser *parser = CSSParserNew(input);
    DFHashTable *properties = CSSParserProperties(parser);
    CSSParserFree(parser);
    if (properties == NULL)
        return DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    else
        return properties;
}

char *CSSSerializeProperties(DFHashTable *cssProperties)
{
    // Note: The properties *must* be sorted, because ODFAutomaticStyles relies on the resulting
    // CSS text as a key into a hash table for anonymous styles
    DFBuffer *output = DFBufferNew();
    size_t count = 0;
    const char **allKeys = DFHashTableCopyKeys(cssProperties);
    DFSortStringsCaseInsensitive(allKeys);
    for (int i = 0; allKeys[i]; i++) {
        const char *property = allKeys[i];
        if (DFStringHasPrefix(property,"-word") || DFStringHasPrefix(property,"-odf"))
            continue;
        const char *value = DFHashTableLookup(cssProperties,property);
        if (count > 0)
            DFBufferFormat(output,"; %s: %s",property,value);
        else
            DFBufferFormat(output,"%s: %s",property,value);
        count++;
    }
    free(allKeys);
    char *result = strdup(output->data);
    DFBufferRelease(output);
    return result;
}

static void expandTextDecoration(DFHashTable *properties)
{
    const char *value = DFHashTableLookup(properties,"text-decoration");
    if (value != NULL) {
        char *textDecoration = DFLowerCase(value);
        const char **tokens = DFStringTokenize(textDecoration,isspace);
        for (int i = 0; tokens[i]; i++) {
            if (!strcmp(tokens[i],"underline"))
                DFHashTableAdd(properties,"text-decoration-underline","underline");
            if (!strcmp(tokens[i],"overline"))
                DFHashTableAdd(properties,"text-decoration-overline","overline");
            if (!strcmp(tokens[i],"line-through"))
                DFHashTableAdd(properties,"text-decoration-line-through","line-through");
        }
        DFHashTableRemove(properties,"text-decoration");
        free(tokens);
        free(textDecoration);
    }
}

static int splitSides(const char *shorthand, SideValues *sides)
{
    const char **tokens = DFStringTokenize(shorthand,isspace);
    int ok = 0;
    int count = 0;
    for (int i = 0; tokens[i]; i++)
        count++;
    switch (count) {
        case 1:
            sides->top = strdup(tokens[0]);
            sides->bottom = strdup(tokens[0]);
            sides->left = strdup(tokens[0]);
            sides->right = strdup(tokens[0]);
            ok = 1;
            break;
        case 2:
            sides->top = strdup(tokens[0]);
            sides->bottom = strdup(tokens[0]);
            sides->left = strdup(tokens[1]);
            sides->right = strdup(tokens[1]);
            ok = 1;
            break;
        case 3:
            sides->top = strdup(tokens[0]);
            sides->left = strdup(tokens[1]);
            sides->right = strdup(tokens[1]);
            sides->bottom = strdup(tokens[2]);
            ok = 1;
            break;
        case 4:
            sides->top = strdup(tokens[0]);
            sides->right = strdup(tokens[1]);
            sides->bottom = strdup(tokens[2]);
            sides->left = strdup(tokens[3]);
            ok = 1;
            break;
    }
    free(tokens);
    return ok;
}

static void expandBorderSide(const char *prefix, DFHashTable *properties)
{
    const char *shorthand = DFHashTableLookup(properties,prefix);
    if (shorthand != NULL) {
        const char **tokens = DFStringTokenize(shorthand,isspace);
        for (int i = 0; tokens[i]; i++) {
            char *name = NULL;
            if (CSSValueIsBorderStyle(tokens[i]))
                name = DFFormatString("%s-style",prefix);
            else if (CSSValueIsBorderWidth(tokens[i]))
                name = DFFormatString("%s-width",prefix);
            else if (CSSValueIsBorderColor(tokens[i]))
                name = DFFormatString("%s-color",prefix);
            if (name != NULL)
                DFHashTableAdd(properties,name,tokens[i]);
            free(name);
        }
        free(tokens);
        DFHashTableRemove(properties,prefix);
    }
}

static void expandBorderAspect(const char *aspect, DFHashTable *properties)
{
    char *propertyName = DFFormatString("border-%s",aspect);
    const char *shorthand = DFHashTableLookup(properties,propertyName);
    if (shorthand != NULL) {
        SideValues sides = SideValuesEmpty;
        if (splitSides(shorthand,&sides)) {
            char *leftName = DFFormatString("border-left-%s",aspect);
            char *rightName = DFFormatString("border-right-%s",aspect);
            char *topName = DFFormatString("border-top-%s",aspect);
            char *bottomName = DFFormatString("border-bottom-%s",aspect);
            DFHashTableAdd(properties,leftName,sides.left);
            DFHashTableAdd(properties,rightName,sides.right);
            DFHashTableAdd(properties,topName,sides.top);
            DFHashTableAdd(properties,bottomName,sides.bottom);
            free(leftName);
            free(rightName);
            free(topName);
            free(bottomName);
        }
        DFHashTableRemove(properties,propertyName);
        SideValuesClear(&sides);
    }
    free(propertyName);
}

static void expandBorders(DFHashTable *properties)
{
    expandBorderSide("border-left",properties);
    expandBorderSide("border-right",properties);
    expandBorderSide("border-top",properties);
    expandBorderSide("border-bottom",properties);

    expandBorderAspect("width",properties);
    expandBorderAspect("color",properties);
    expandBorderAspect("style",properties);

    // border

    const char *border = DFHashTableLookup(properties,"border");
    if (border != NULL) {
        const char **tokens = DFStringTokenize(border,isspace);
        for (int i = 0; tokens[i]; i++) {
            const char *token = tokens[i];
            if (CSSValueIsBorderStyle(token)) {
                DFHashTableAdd(properties,"border-left-style",token);
                DFHashTableAdd(properties,"border-right-style",token);
                DFHashTableAdd(properties,"border-top-style",token);
                DFHashTableAdd(properties,"border-bottom-style",token);
            }
            else if (CSSValueIsBorderWidth(token)) {
                DFHashTableAdd(properties,"border-left-width",token);
                DFHashTableAdd(properties,"border-right-width",token);
                DFHashTableAdd(properties,"border-top-width",token);
                DFHashTableAdd(properties,"border-bottom-width",token);
            }
            else if (CSSValueIsBorderColor(token)) {
                DFHashTableAdd(properties,"border-left-color",token);
                DFHashTableAdd(properties,"border-right-color",token);
                DFHashTableAdd(properties,"border-top-color",token);
                DFHashTableAdd(properties,"border-bottom-color",token);
            }
        }
        free(tokens);
        DFHashTableRemove(properties,"border");
    }

    // border-radius

    const char *radius = DFHashTableLookup(properties,"border-radius");
    if (radius != NULL) {
        DFHashTableAdd(properties,"border-top-left-radius",radius);
        DFHashTableAdd(properties,"border-top-right-radius",radius);
        DFHashTableAdd(properties,"border-bottom-left-radius",radius);
        DFHashTableAdd(properties,"border-bottom-right-radius",radius);
        DFHashTableRemove(properties,"border-radius");
    }
}

static void expandPadding(DFHashTable *properties)
{
    const char *shorthand = DFHashTableLookup(properties,"padding");
    if (shorthand != NULL) {
        SideValues sides = SideValuesEmpty;
        if (splitSides(shorthand,&sides)) {
            DFHashTableAdd(properties,"padding-left",sides.left);
            DFHashTableAdd(properties,"padding-right",sides.right);
            DFHashTableAdd(properties,"padding-top",sides.top);
            DFHashTableAdd(properties,"padding-bottom",sides.bottom);
        }
        DFHashTableRemove(properties,"padding");
        SideValuesClear(&sides);
    }
}

static void expandMargins(DFHashTable *properties)
{
    const char *shorthand = DFHashTableLookup(properties,"margin");
    if (shorthand != NULL) {
        SideValues sides = SideValuesEmpty;
        if (splitSides(shorthand,&sides)) {
            DFHashTableAdd(properties,"margin-left",sides.left);
            DFHashTableAdd(properties,"margin-right",sides.right);
            DFHashTableAdd(properties,"margin-top",sides.top);
            DFHashTableAdd(properties,"margin-bottom",sides.bottom);
        }
        DFHashTableRemove(properties,"margin");
        SideValuesClear(&sides);
    }
}

void CSSExpandProperties(DFHashTable *properties)
{
    expandTextDecoration(properties);
    expandBorders(properties);
    expandPadding(properties);
    expandMargins(properties);
}

DFHashTable *CSSCollapseProperties(CSSProperties *expanded)
{
    DFHashTable *collapsed = DFHashTableCopy(expanded->hashTable);
    const char *underline = DFHashTableLookup(collapsed,"text-decoration-underline");
    const char *overline = DFHashTableLookup(collapsed,"text-decoration-overline");
    const char *lineThrough = DFHashTableLookup(collapsed,"text-decoration-line-through");
    if ((underline != NULL) || (overline != NULL) || (lineThrough != NULL)) {
        DFBuffer *buffer = DFBufferNew();
        if (underline != NULL)
            DFBufferAppendString(buffer," underline");
        if (overline != NULL)
            DFBufferAppendString(buffer," overline");
        if (lineThrough != NULL)
            DFBufferAppendString(buffer," line-through");
        char *trimmed = DFStringTrimWhitespace(buffer->data);
        DFHashTableAdd(collapsed,"text-decoration",trimmed);
        DFHashTableRemove(collapsed,"text-decoration-underline");
        DFHashTableRemove(collapsed,"text-decoration-overline");
        DFHashTableRemove(collapsed,"text-decoration-line-through");
        free(trimmed);
        DFBufferRelease(buffer);
    }

    // Margins

    const char *marginLeft = CSSGet(expanded,"margin-left");
    const char *marginRight = CSSGet(expanded,"margin-right");
    const char *marginTop = CSSGet(expanded,"margin-top");
    const char *marginBottom = CSSGet(expanded,"margin-bottom");
    if ((marginLeft != NULL) && (marginRight != NULL) && (marginTop != NULL) && (marginBottom != NULL) &&
        DFStringEquals(marginLeft,marginRight) &&
        DFStringEquals(marginLeft,marginTop) &&
        DFStringEquals(marginLeft,marginBottom)) {

        DFHashTableAdd(collapsed,"margin",marginLeft);
        DFHashTableRemove(collapsed,"margin-left");
        DFHashTableRemove(collapsed,"margin-right");
        DFHashTableRemove(collapsed,"margin-top");
        DFHashTableRemove(collapsed,"margin-bottom");
    }

    // Padding

    const char *paddingLeft = CSSGet(expanded,"padding-left");
    const char *paddingRight = CSSGet(expanded,"padding-right");
    const char *paddingTop = CSSGet(expanded,"padding-top");
    const char *paddingBottom = CSSGet(expanded,"padding-bottom");

    if ((paddingLeft != NULL) && (paddingRight != NULL) && (paddingTop != NULL) && (paddingBottom != NULL) &&
        DFStringEquals(paddingLeft,paddingRight) &&
        DFStringEquals(paddingLeft,paddingTop) &&
        DFStringEquals(paddingLeft,paddingBottom)) {

        DFHashTableAdd(collapsed,"padding",paddingLeft);
        DFHashTableRemove(collapsed,"padding-left");
        DFHashTableRemove(collapsed,"padding-right");
        DFHashTableRemove(collapsed,"padding-top");
        DFHashTableRemove(collapsed,"padding-bottom");
    }

    // Border radius

    const char *borderTopLeftRadius = CSSGet(expanded,"border-top-left-radius");
    const char *borderTopRightRadius = CSSGet(expanded,"border-top-right-radius");
    const char *borderBottomLeftRadius = CSSGet(expanded,"border-bottom-left-radius");
    const char *borderBottomRightRadius = CSSGet(expanded,"border-bottom-right-radius");

    if ((borderTopLeftRadius != NULL) && (borderTopRightRadius != NULL) &&
        (borderBottomLeftRadius != NULL) && (borderBottomRightRadius != NULL) &&
        DFStringEquals(borderTopLeftRadius,borderTopRightRadius) &&
        DFStringEquals(borderTopLeftRadius,borderBottomLeftRadius) &&
        DFStringEquals(borderTopLeftRadius,borderBottomRightRadius)) {

        DFHashTableAdd(collapsed,"border-radius",borderTopLeftRadius);
        DFHashTableRemove(collapsed,"border-top-left-radius");
        DFHashTableRemove(collapsed,"border-top-right-radius");
        DFHashTableRemove(collapsed,"border-bottom-left-radius");
        DFHashTableRemove(collapsed,"border-bottom-right-radius");
    }

    // Borders

    DFBuffer *borderLeft = DFBufferNew();
    DFBuffer *borderRight = DFBufferNew();
    DFBuffer *borderTop = DFBufferNew();
    DFBuffer *borderBottom = DFBufferNew();

    const char *borderLeftWidth = CSSGet(expanded,"border-left-width");
    const char *borderRightWidth = CSSGet(expanded,"border-right-width");
    const char *borderTopWidth = CSSGet(expanded,"border-top-width");
    const char *borderBottomWidth = CSSGet(expanded,"border-bottom-width");

    const char *borderLeftStyle = CSSGet(expanded,"border-left-style");
    const char *borderRightStyle = CSSGet(expanded,"border-right-style");
    const char *borderTopStyle = CSSGet(expanded,"border-top-style");
    const char *borderBottomStyle = CSSGet(expanded,"border-bottom-style");

    const char *borderLeftColor = CSSGet(expanded,"border-left-color");
    const char *borderRightColor = CSSGet(expanded,"border-right-color");
    const char *borderTopColor = CSSGet(expanded,"border-top-color");
    const char *borderBottomColor = CSSGet(expanded,"border-bottom-color");

    if (borderLeftWidth != NULL)
        DFBufferFormat(borderLeft," %s",borderLeftWidth);
    if (borderRightWidth != NULL)
        DFBufferFormat(borderRight," %s",borderRightWidth);
    if (borderTopWidth != NULL)
        DFBufferFormat(borderTop," %s",borderTopWidth);
    if (borderBottomWidth != NULL)
        DFBufferFormat(borderBottom," %s",borderBottomWidth);

    if (borderLeftStyle != NULL)
        DFBufferFormat(borderLeft," %s",borderLeftStyle);
    if (borderRightStyle != NULL)
        DFBufferFormat(borderRight," %s",borderRightStyle);
    if (borderTopStyle != NULL)
        DFBufferFormat(borderTop," %s",borderTopStyle);
    if (borderBottomStyle != NULL)
        DFBufferFormat(borderBottom," %s",borderBottomStyle);

    if (borderLeftColor != NULL)
        DFBufferFormat(borderLeft," %s",borderLeftColor);
    if (borderRightColor != NULL)
        DFBufferFormat(borderRight," %s",borderRightColor);
    if (borderTopColor != NULL)
        DFBufferFormat(borderTop," %s",borderTopColor);
    if (borderBottomColor != NULL)
        DFBufferFormat(borderBottom," %s",borderBottomColor);

    if ((borderLeft->len > 0) &&
        !strcmp(borderLeft->data,borderRight->data) &&
        !strcmp(borderLeft->data,borderTop->data) &&
        !strcmp(borderLeft->data,borderBottom->data)) {
        DFHashTableAdd(collapsed,"border",&borderLeft->data[1]);
    }
    else {
        if (borderLeft->len > 0)
            DFHashTableAdd(collapsed,"border-left",&borderLeft->data[1]);
        if (borderRight->len > 0)
            DFHashTableAdd(collapsed,"border-right",&borderRight->data[1]);
        if (borderTop->len > 0)
            DFHashTableAdd(collapsed,"border-top",&borderTop->data[1]);
        if (borderBottom->len > 0)
            DFHashTableAdd(collapsed,"border-bottom",&borderBottom->data[1]);
    }

    DFHashTableRemove(collapsed,"border-left-width");
    DFHashTableRemove(collapsed,"border-right-width");
    DFHashTableRemove(collapsed,"border-top-width");
    DFHashTableRemove(collapsed,"border-bottom-width");

    DFHashTableRemove(collapsed,"border-left-style");
    DFHashTableRemove(collapsed,"border-right-style");
    DFHashTableRemove(collapsed,"border-top-style");
    DFHashTableRemove(collapsed,"border-bottom-style");

    DFHashTableRemove(collapsed,"border-left-color");
    DFHashTableRemove(collapsed,"border-right-color");
    DFHashTableRemove(collapsed,"border-top-color");
    DFHashTableRemove(collapsed,"border-bottom-color");

    DFBufferRelease(borderLeft);
    DFBufferRelease(borderRight);
    DFBufferRelease(borderTop);
    DFBufferRelease(borderBottom);

    return collapsed;
}

DFArray *CSSParseContent(const char *content)
{
    CSSParser *parser = CSSParserNew(content);
    DFArray *result = CSSParserContent(parser);
    CSSParserFree(parser);
    return result;
}

// Converts a string(selector) -> hashtable(properties) dictionary to a
// string(selector) -> string(propertyText)

static DFHashTable *builtTextRules(DFHashTable *input)
{
    DFHashTable *result = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);

    const char **allSelectors = DFHashTableCopyKeys(input);
    for (int selIndex = 0; allSelectors[selIndex]; selIndex++) {
        const char *selector = allSelectors[selIndex];

        DFBuffer *text = DFBufferNew();

        DFHashTable *cssProperties = DFHashTableLookup(input,selector);
        assert(cssProperties != NULL);

        const char **allNames = DFHashTableCopyKeys(cssProperties);
        DFSortStringsCaseInsensitive(allNames);
        for (int nameIndex = 0; allNames[nameIndex]; nameIndex++) {
            const char *name = allNames[nameIndex];
            const char *value = DFHashTableLookup(cssProperties,name);
            DFBufferFormat(text,"    %s: %s;\n",name,value);
        }
        free(allNames);

        DFHashTableAdd(result,selector,text->data);
        DFBufferRelease(text);
    }
    free(allSelectors);
    return result;
}

// Collates multiple rules with the same text into a single rule
// For example:
//    .foo { color: blue; }
//    .bar { color: blue; }
// becomes:
//    .bar, .foo { color: blue; }

static DFHashTable *combineTextRules(DFHashTable *separate)
{
    DFHashTable *reverse = DFHashTableNew((DFCopyFunction)DFArrayRetain,(DFFreeFunction)DFArrayRelease);

    const char **separateKeys = DFHashTableCopyKeys(separate);
    for (int i = 0; separateKeys[i]; i++) {
        const char *selector = separateKeys[i];
        const char *text = DFHashTableLookup(separate,selector);

        DFArray *array = DFHashTableLookup(reverse,text);
        if (array == NULL) {
            array = DFArrayNew((DFCopyFunction)strdup,free);
            DFHashTableAdd(reverse,text,array);
            DFArrayRelease(array);
        }
        DFArrayAppend(array,(void *)selector);
    }
    free(separateKeys);

    DFHashTable *combined = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    const char **reverseKeys = DFHashTableCopyKeys(reverse);
    for (int keyIndex = 0; reverseKeys[keyIndex]; keyIndex++) {
        const char *text = reverseKeys[keyIndex];
        DFArray *array = DFHashTableLookup(reverse,text);
        const char **selectors = DFStringArrayFlatten(array);
        DFSortStringsCaseInsensitive(selectors);
        DFBuffer *combinedSelector = DFBufferNew();
        for (int i = 0; selectors[i]; i++) {
            if (i > 0)
                DFBufferFormat(combinedSelector,", ");
            DFBufferFormat(combinedSelector,"%s",selectors[i]);
        }
        DFHashTableAdd(combined,combinedSelector->data,text);
        DFBufferRelease(combinedSelector);
        free(selectors);
    }
    free(reverseKeys);

    DFHashTableRelease(reverse);
    return combined;
}

char *CSSCopyStylesheetTextFromRules(DFHashTable *rules)
{
    DFBuffer *output = DFBufferNew();
    DFHashTable *separate = builtTextRules(rules);
    DFHashTable *textRules = combineTextRules(separate);

    const char **allSelectors = DFHashTableCopyKeys(textRules);
    DFSortStringsCaseInsensitive(allSelectors);
    for (int i = 0; allSelectors[i]; i++) {
        const char *selector = allSelectors[i];
        if (i > 0)
            DFBufferFormat(output,"\n");;
        const char *text = DFHashTableLookup(textRules,selector);
        assert(text != NULL);
        DFBufferFormat(output,"%s {\n%s}\n",selector,text);
    }
    free(allSelectors);
    DFHashTableRelease(separate);
    DFHashTableRelease(textRules);
    char *result = strdup(output->data);
    DFBufferRelease(output);
    return result;
}

static int rgbValue(const char *untrimmed)
{
    char *str = DFStringTrimWhitespace(untrimmed);
    double d;
    if (DFStringHasSuffix(str,"%")) // FIXME: Not covered by tests
        d = atof(str)/100.0;
    else
        d = atof(str)/255.0;

    int i = (int)(d*255);
    if (i < 0)
        i = 0;
    if (i > 255)
        i = 255;
    free(str);
    return i;
}

int parseRGB(const char *input, int *r, int *g, int *b)
{
    size_t len = strlen(input);

    if (!DFStringHasPrefix(input,"rgb("))
        return 0;

    if (!DFStringHasSuffix(input,")"))
        return 0;

    char *argtext = DFSubstring(input,4,len-1);
    const char **args = DFStringSplit(argtext,",",0);
    int ok = 0;
    if (DFStringArrayCount(args) == 3) {
        *r = rgbValue(args[0]);
        *g = rgbValue(args[1]);
        *b = rgbValue(args[2]);
        ok = 1;
    }
    free(argtext);
    free(args);

    return ok;
}

static struct {
    const char *name;
    const char *hex;
} cssColorNames[] = {
    {"maroon", "#800000"},
    {"red", "#ff0000"},
    {"orange", "#ffA500"},
    {"yellow", "#ffff00"},
    {"olive", "#808000"},
    {"purple", "#800080"},
    {"fuchsia", "#ff00ff"},
    {"white", "#ffffff"},
    {"lime", "#00ff00"},
    {"green", "#008000"},
    {"navy", "#000080"},
    {"blue", "#0000ff"},
    {"aqua", "#00ffff"},
    {"teal", "#008080"},
    {"black", "#000000"},
    {"silver", "#c0c0c0"},
    {"gray", "#808080"},
    {NULL, NULL}
};

char *CSSHexColor(const char *color, int includeHash)
{
    if (color == NULL)
        return NULL;

    char *result = NULL;

    // color: #f00
    if ((strlen(color) == 4) && (color[0] == '#')) {
        // FIXME: Not covered by tests
        char *upper = DFUpperCase(color);
        char r = upper[1];
        char g = upper[2];
        char b = upper[3];
        result = DFFormatString("#%c%c%c%c%c%c",r,r,g,g,b,b);
        free(upper);
    }
    // color: #ff0000
    else if ((strlen(color) == 7) && (color[0] == '#')) {
        result = DFUpperCase(color);
    }
    // color: rgb(255,0,0)
    // color: rgb(100%, 0%, 0%)
    else if (DFStringHasPrefix(color,"rgb(") && DFStringHasSuffix(color,")")) {
        int r = 0, g = 0, b = 0;
        if (parseRGB(color,&r,&g,&b))
            result = DFFormatString("#%02X%02X%02X",r,g,b);
    }
    // color: red
    else {
        for (int i = 0; cssColorNames[i].name != NULL; i++) {
            if (DFStringEqualsCI(cssColorNames[i].name,color)) {
                result = DFUpperCase(cssColorNames[i].hex);
                break;
            }
        }
    }

    if (!includeHash && (result != NULL)) {
        char *nohash = strdup(&result[1]);
        free(result);
        result = nohash;
    }

    return result;
}

char *CSSEncodeFontFamily(const char *input)
{
    if (input == NULL)
        return NULL;
    if (DFStringContainsWhitespace(input))
        return DFQuote(input);
    else
        return strdup(input);
}

char *CSSDecodeFontFamily(const char *input)
{
    if (input == NULL)
        return NULL;
    else
        return DFUnquote(input);
}

static const char *inlinePropertyNames[] = {
    "font-weight",
    "font-style",
    "text-decoration-underline",
    "text-decoration-line-through",
    "text-decoration-vertical-align",
    "color",
    "font-size",
    "font-family",
    NULL,
};

int CSSIsInlineProperty(const char *name)
{
    for (int i = 0; inlinePropertyNames[i]; i++) {
        if (!strcmp(name,inlinePropertyNames[i]))
            return 1;
    }
    return 0;
}

PageSize CSSParsePageSize(const char *input)
{
    PageSize result = PageSizeUnknown;
    char *normalized = DFStringNormalizeWhitespace(input);
    if (DFStringEqualsCI(normalized,"a4 portrait"))
        result = PageSizeA4Portrait;
    else if (DFStringEqualsCI(normalized,"a4 landscape"))
        result = PageSizeA4Landscape;
    else if (DFStringEqualsCI(normalized,"letter portrait"))
        result = PageSizeLetterPortrait;
    else if (DFStringEqualsCI(normalized,"letter landscape"))
        result = PageSizeLetterLandscape;
    free(normalized);
    return result;
}

static int isNMStart(uint32_t ch)
{
    return ((ch == '_') ||
            ((ch >= 'a') && (ch <= 'z')) ||
            ((ch >= 'A') && (ch <= 'Z')));
}

static int isNMChar(uint32_t ch)
{
    return ((ch == '_') ||
            ((ch >= 'a') && (ch <= 'z')) ||
            ((ch >= 'A') && (ch <= 'Z')) ||
            ((ch >= '0') && (ch <= '9')) ||
            (ch == '-'));
}

static void appendUTF32Char(DFBuffer *buf, uint32_t ch)
{
    DFBufferAppendData(buf,(const void *)&ch,sizeof(ch));
}

static unsigned int fromHexChar(uint32_t ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    if ((ch >= 'a') && (ch <= 'f'))
        return 10 + ch - 'a';
    if ((ch >= 'A') && (ch <= 'F'))
        return 10 + ch - 'A';
    return 0;
}

static void CSSEscapeIdentifier(const uint32_t *chars, size_t len, DFBuffer *output)
{
    size_t pos = 0;

    if ((pos < len) && (chars[pos] == '-')) {
        if (len == 1) {
            appendUTF32Char(output,'\\');
            appendUTF32Char(output,'2');
            appendUTF32Char(output,'d');
        }
        else {
            appendUTF32Char(output,'-');
        }
        pos++;
    }

    size_t start = pos;

    while (pos < len) {
        if ((pos == start) && isNMStart(chars[pos])) {
            appendUTF32Char(output,chars[pos]);
            pos++;
        }
        else if ((pos > start) && isNMChar(chars[pos])) {
            appendUTF32Char(output,chars[pos]);
            pos++;
        }
        else if (chars[pos] == 0) {
            appendUTF32Char(output,'\\');
            appendUTF32Char(output,'0');
            appendUTF32Char(output,' ');
            pos++;
        }
        else {
            unsigned int value = chars[pos++];

            appendUTF32Char(output,'\\');

            uint32_t bytes[6];
            int byteCount = 0;
            while ((value > 0) && (byteCount < 6)){
                unsigned int nibble = value & 0xF;
                if (nibble >= 0xA)
                    bytes[5-byteCount] = 'a'+nibble-0xA;
                else
                    bytes[5-byteCount] = '0'+nibble;
                value >>= 4;
                byteCount++;
            }
            DFBufferAppendData(output,(const void *)&bytes[6-byteCount],byteCount*sizeof(uint32_t));
            appendUTF32Char(output,' ');
        }
    }

    // Remove trailing spaces
    size_t outLen = output->len/sizeof(uint32_t);
    uint32_t *outChars = (uint32_t *)output->data;
    while ((outLen > 0) && (outChars[outLen-1] == ' ')) {
        output->len -= sizeof(uint32_t);
        outLen--;
    }
}

static void CSSUnescapeIdentifier(const uint32_t *chars, size_t len, DFBuffer *output)
{
    size_t pos = 0;

    if ((pos < len) && (chars[pos] == '-')) {
        appendUTF32Char(output,'-');
        pos++;
    }

    size_t start = pos;

    while (pos < len) {
        if (chars[pos] == '\\') {
            pos++;

            uint32_t value = 0;
            int numDigits = 0;
            while ((numDigits < 6) && (pos < len) && DFCharIsHex(chars[pos])) {
                value = value*16 + fromHexChar(chars[pos]);
                pos++;
                numDigits++;
            }

            appendUTF32Char(output,value);

            if (pos < len) {
                switch (chars[pos]) {
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\f':
                        pos++;
                        break;
                    case '\r':
                        if ((pos + 1 < len) && (chars[pos+1] == '\n'))
                            pos += 2;
                        else
                            pos++;
                        break;
                }
            }
        }
        else if ((pos == start) && isNMStart(chars[pos])) {
            appendUTF32Char(output,chars[pos]);
            pos++;
        }
        else if ((pos > start) && isNMChar(chars[pos])) {
            appendUTF32Char(output,chars[pos]);
            pos++;
        }
        else {
            pos++;
        }
    }
}

char *CSSEscapeIdent(const char *cunescaped)
{
    if (cunescaped == NULL)
        return NULL;;

    uint32_t *unescaped32 = DFUTF8To32(cunescaped);
    size_t len32 = DFUTF32Length(unescaped32);
    DFBuffer *output = DFBufferNew();

    CSSEscapeIdentifier(unescaped32,len32,output);
    appendUTF32Char(output,0);
    assert((output->len % 4) == 0);
    char *result = DFUTF32to8((const uint32_t *)output->data);

    free(unescaped32);
    DFBufferRelease(output);
    return result;
}

char *CSSUnescapeIdent(const char *cescaped)
{
    if (cescaped == NULL)
        return NULL;;

    uint32_t *escaped32 = DFUTF8To32(cescaped);
    size_t len32 = DFUTF32Length(escaped32);
    DFBuffer *output = DFBufferNew();

    CSSUnescapeIdentifier(escaped32,len32,output);
    appendUTF32Char(output,0);
    assert((output->len % 4) == 0);
    char *result = DFUTF32to8((const uint32_t *)output->data);

    free(escaped32);
    DFBufferRelease(output);
    return result;
}

void CSSParseSelector(const char *cinput, char **result, char **suffix)
{
    size_t len = strlen(cinput);
    size_t pos = 0;

    // Parse element name
    while ((pos < len) && (cinput[pos] != '.') && (cinput[pos] != ' ') && (cinput[pos] != ':'))
        pos++;

    char *elementName = DFSubstring(cinput,0,pos);

    // Parse class name
    if ((pos < len) && (cinput[pos] == '.')) {
        pos++;
        size_t classStart = pos;
        while (pos < len) {
            if (cinput[pos] == '\\') {
                pos++;
                while ((pos < len) && DFCharIsHex(cinput[pos]))
                    pos++;
                if ((pos < len) && (cinput[pos] == ' '))
                    pos++;
            }
            else if ((cinput[pos] == ' ') || (cinput[pos] == ':')) {
                break;
            }
            else {
                pos++;
            }
        }

        char *className = DFSubstring(cinput,classStart,pos);
        char *unescapedClassName = CSSUnescapeIdent(className);
        *result = DFFormatString("%s.%s",elementName,unescapedClassName);
        free(unescapedClassName);
        free(className);
    }
    else {
        *result = strdup(elementName);
    }

    // Parse suffix
    // FIXME: ignore spaces at start?
    *suffix = strdup(&cinput[pos]);

    free(elementName);
}
