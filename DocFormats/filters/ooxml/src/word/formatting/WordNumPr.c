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
#include "WordNumPr.h"
#include "WordNumbering.h"
#include "WordSheet.h"
#include "CSS.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *parseLvlText(const char *input, WordConcreteNum *num)
{
    DFBuffer *result = DFBufferNew();
    size_t len = strlen(input);
    size_t pos = 0;

    int endsWithWhitespace = 0;
    size_t start = 0;
    while (1) {
        if ((pos >= len) || (input[pos] == '%')) {
            if (pos > start) {
                char *str = DFSubstring(input,start,pos);
                if (result->len > 0)
                    DFBufferFormat(result," ");
                char *quotedStr = DFQuote(str);
                DFBufferFormat(result,"%s",quotedStr);
                free(quotedStr);
                endsWithWhitespace = (strlen(str) > 0) && isspace(str[strlen(str)-1]);
                free(str);
            }
            start = pos;
        }
        if (pos >= len)
            break;
        if ((input[pos] == '%') && (pos+1 < len) &&
            (input[pos+1] >= '1') && (input[pos+1] <= '6')) {
            int ilvl = input[pos+1] - '1';

            if (result->len > 0)
                DFBufferFormat(result," ");
            char *counterName = DFFormatString("h%d",ilvl+1);

            WordNumLevel *numLevel = WordConcreteNumGetLevel(num,ilvl);
            const char *type = (numLevel != NULL) ? WordNumLevelToListStyleType(numLevel) : NULL;
            if ((type != NULL) && !DFStringEquals(type,"decimal"))
                DFBufferFormat(result,"counter(%s,%s)",counterName,type);
            else
                DFBufferFormat(result,"counter(%s)",counterName);
            free(counterName);
            endsWithWhitespace = 0;

            pos += 2;
            start = pos;
        }
        else {
            pos++;
        }
    }

    if ((result->len > 0) && !endsWithWhitespace)
        DFBufferFormat(result," \" \"");
    char *str = strdup(result->data);
    DFBufferRelease(result);
    return str;
}

void WordGetNumPrStyle(DFNode *numPr, CSSStyle *style, WordConverter *converter)
{
    DFNode *ilvlElem = DFChildWithTag(numPr,WORD_ILVL);
    DFNode *numIdElem = DFChildWithTag(numPr,WORD_NUMID);
    const char *ilvlStr = (ilvlElem != NULL) ? DFGetAttribute(ilvlElem,WORD_VAL) : NULL;
    const char *numIdStr = (numIdElem != NULL) ? DFGetAttribute(numIdElem,WORD_VAL) : NULL;
    if (numIdStr == NULL)
        return;

    int ilvl = (ilvlStr != NULL) ? atoi(ilvlStr) : 0;
    WordConcreteNum *num = WordNumberingConcreteWithId(converter->numbering,numIdStr);
    if (num == NULL) {
        return;
    }

    CSSProperties *before = CSSStyleBefore(style);
    CSSProperties *during = CSSStyleRule(style);

    WordNumLevel *level = WordConcreteNumGetLevel(num,ilvl);
    if ((level != NULL) && (level->lvlText != NULL)) {
        char *parsed = parseLvlText(level->lvlText,num);
        CSSPut(before,"content",parsed);
        free(parsed);
    }

    char *mainCounterName = DFFormatString("h%d",ilvl+1);
    CSSPut(during,"counter-increment",mainCounterName);
    free(mainCounterName);

    DFBuffer *reset = DFBufferNew();
    for (int i = ilvl+1; i <= 5; i++) {
        char *counterName = DFFormatString("h%d",i+1);
        if (i == ilvl+1)
            DFBufferFormat(reset,"%s",counterName);
        else
            DFBufferFormat(reset," %s",counterName);
        free(counterName);
    }
    CSSPut(during,"counter-reset",reset->data);
    DFBufferRelease(reset);
}

typedef struct WordNumInfo WordNumInfo;

struct WordNumInfo {
    char *cssType; // Value range: CSS list-style-type
    char *cssLvlText; // Values: Word's lvlText format
    char *wordNumId;
    char *wordIlvl;
    WordNumLevel *wordLevel;
};

static WordNumInfo *WordNumInfoNew(void)
{
    return (WordNumInfo *)calloc(1,sizeof(WordNumInfo));
}

static void WordNumInfoFree(WordNumInfo *info)
{
    free(info->cssType);
    free(info->cssLvlText);
    free(info->wordNumId);
    free(info->wordIlvl);
    free(info);
}

void updateNumbering(WordConverter *converter, CSSSheet *cssSheet)
{
    DFHashTable *infoByStyleId = DFHashTableNew(NULL,(DFFreeFunction)WordNumInfoFree);
    WordSheet *wordSheet = converter->styles;

    int cssLevelNumbered[6];
    int wordLevelNumbered[6];
    for (int i = 1; i <= 6; i++) {
        cssLevelNumbered[i-1] = 0;
        wordLevelNumbered[i-1] = 0;
    }

    const char **cssSelectors = CSSSheetCopySelectors(cssSheet);
    for (int i = 0; cssSelectors[i]; i++) {
        const char *selector = cssSelectors[i];
        if (WordStyleFamilyForSelector(selector) != StyleFamilyParagraph)
            continue;

        WordNumInfo *info = WordNumInfoNew();
        DFHashTableAdd(infoByStyleId,selector,info);

        CSSStyle *cssStyle = CSSSheetLookupSelector(cssSheet,selector,0,0);
        if ((cssStyle != NULL) && (CSSGet(CSSStyleBefore(cssStyle),"content") != NULL)) {
            char *elementName = CSSSelectorCopyElementName(selector);

            if ((cssStyle->headingLevel >= 1) && (cssStyle->headingLevel <= 6))
                cssLevelNumbered[cssStyle->headingLevel-1] = 1;;

            DFArray *contentParts = CSSParseContent(CSSGet(CSSStyleBefore(cssStyle),"content"));
            DFBuffer *format = DFBufferNew();
            for (size_t partIndex = 0; partIndex < DFArrayCount(contentParts); partIndex++) {
                ContentPart *part = DFArrayItemAt(contentParts,partIndex);
                if (part->type == ContentPartCounter) {
                    if (DFStringEquals(part->value,elementName)) {
                        free(info->cssType);
                        if (part->arg != NULL)
                            info->cssType = strdup(part->arg);
                        else
                            info->cssType = strdup("decimal");
                    }
                    if ((strlen(part->value) == 2) && (part->value[0] == 'h'))
                        DFBufferFormat(format,"%%%c",part->value[1]);
                }
                else if (part->type == ContentPartString) {
                    // FIXME: Need test case for string that actually contains %
                    char *noPercent = DFStringReplace(part->value,"%","");
                    DFBufferFormat(format,"%s",noPercent);
                    free(noPercent);
                }
            }
            DFArrayRelease(contentParts);

            if ((format->len > 0) && isspace(format->data[format->len-1])) {
                format->data[format->len-1] = '\0';
                format->len--;
            }

            info->cssLvlText = strdup(format->data);
            free(elementName);
            DFBufferRelease(format);
        }

        WordStyle *wordStyle = WordSheetStyleForSelector(wordSheet,selector);
        if (wordStyle != NULL) {
            DFNode *pPr = DFChildWithTag(wordStyle->element,WORD_PPR);
            DFNode *numPr = DFChildWithTag(pPr,WORD_NUMPR);
            const char *numId = DFGetChildAttribute(numPr,WORD_NUMID,WORD_VAL);
            const char *ilvl = DFGetChildAttribute(numPr,WORD_ILVL,WORD_VAL);
            if (numId != NULL) {
                if (ilvl == NULL)
                    ilvl = "0";;
                WordConcreteNum *concreteNum = WordNumberingConcreteWithId(converter->numbering,numId);
                if (concreteNum != NULL) {
                    WordNumLevel *level = WordConcreteNumGetLevel(concreteNum,atoi(ilvl));
                    if (level != NULL) {
                        info->wordNumId = strdup(numId);
                        info->wordIlvl = strdup(ilvl);
                        info->wordLevel = level;

                        int ilvlValue = atoi(ilvl);
                        if ((ilvlValue >= 0) && (ilvlValue <= 5))
                            wordLevelNumbered[ilvlValue] = 1;
                    }
                }
            }
        }
    }
    free(cssSelectors);

    // For any CSS heading styles that are missing either a type or level text, clear the
    // corresponding word documents, so these are removed from styles.xml if they previously
    // existed.
    cssSelectors = DFHashTableCopyKeys(infoByStyleId);
    for (int i = 0; cssSelectors[i]; i++) {
        const char *selector = cssSelectors[i];
        WordNumInfo *info = DFHashTableLookup(infoByStyleId,selector);
        if ((info != NULL) && (info->cssLvlText == NULL)) {
            free(info->wordNumId);
            free(info->wordIlvl);
            info->wordLevel = NULL;
            info->wordNumId = NULL;
            info->wordIlvl = NULL;
        }
    }
    free(cssSelectors);

    int totalCSSNumbered = 0;
    int totalWordNumbered = 0;

    for (int i = 0; i <= 5; i++) {
        if (cssLevelNumbered[i])
            totalCSSNumbered++;
        if (wordLevelNumbered[i])
            totalWordNumbered++;
    }

    if ((totalCSSNumbered > 0) && (totalWordNumbered < 6)) {
        // We don't have all 6 heading levels. Recreate everything
        WordAbstractNum *abstractNum = WordNumberingCreateAbstractNum(converter->numbering);
        WordConcreteNum *concreteNum = WordNumberingAddConcreteWithAbstract(converter->numbering,abstractNum);
        char *prevType = NULL;
        char *prevLvlText = NULL;

        DFHashTable *selectorsByLevel = DFHashTableNew(NULL,NULL);
        const char **allSelectors = CSSSheetCopySelectors(cssSheet);
        for (int i = 0; allSelectors[i]; i++) {
            CSSStyle *style = CSSSheetLookupSelector(cssSheet,allSelectors[i],0,0);
            if ((style->headingLevel >= 1) && (style->headingLevel <= 6)) {
                int level = style->headingLevel - 1;
                SelectorList *item = (SelectorList *)calloc(1,sizeof(SelectorList));
                item->selector = strdup(style->selector);
                item->next = DFHashTableLookupInt(selectorsByLevel,level);
                DFHashTableAddInt(selectorsByLevel,level,item);
            }
        }
        free(allSelectors);

        DFHashTable *numStylesByLevel = DFHashTableNew((DFCopyFunction)CSSStyleRetain,(DFFreeFunction)CSSStyleRelease);

        for (int i = 0; i < 6; i++) {
            SelectorList *list = DFHashTableLookupInt(selectorsByLevel,i);
            SelectorList *next;
            for (SelectorList *item = list; item != NULL; item = next) {
                next = item->next;
                CSSStyle *style = CSSSheetLookupSelector(cssSheet,item->selector,0,0);
                // FIXME: need to do this comparison based on lvlText
                if ((CSSGet(CSSStyleBefore(style),"content") != NULL) &&
                    !DFStringEquals(CSSGet(CSSStyleBefore(style),"content"),"none") &&
                    !DFStringEquals(CSSGet(CSSStyleBefore(style),"content"),"\"\"")) {
                    DFHashTableAddInt(numStylesByLevel,i,style);
                }
                free(item->selector);
                free(item);
            }
        }

        for (int i = 0; i < 6; i++) {
            CSSStyle *style = DFHashTableLookupInt(numStylesByLevel,i);
            char *curType = NULL;
            char *curLvlText = NULL;
            WordNumInfo *info = NULL;


            if (style != NULL) {
                info = DFHashTableLookup(infoByStyleId,style->selector);
                if (info == NULL) {
                    info = WordNumInfoNew();
                    DFHashTableAdd(infoByStyleId,style->selector,info);
                }
                curType = strdup(info->cssType);
                curLvlText = strdup(info->cssLvlText);
            }


            if (curType == NULL)
                curType = (prevType != NULL) ? strdup(prevType) : strdup("decimal");

            if ((curLvlText == NULL) || (strlen(curLvlText) == 0)) {
                free(curLvlText);
                if ((prevLvlText == NULL) || (strlen(prevLvlText) == 0))
                    curLvlText = DFFormatString("%%%d",i+1);
                else
                    curLvlText = DFFormatString("%s.%%%d",prevLvlText,i+1);
            }

            WordNumLevel *numLevel = WordNumberingCreateLevel(converter->numbering,curType,curLvlText,i,0);
            DFAppendChild(abstractNum->element,numLevel->element);
            WordAbstractNumAddLevel(abstractNum,numLevel);

            if (info != NULL) {
                info->wordLevel = numLevel;
                assert(info->wordLevel != NULL);
                free(info->wordNumId);
                free(info->wordIlvl);
                info->wordNumId = strdup(concreteNum->numId);
                info->wordIlvl = DFFormatString("%d",i);
            }

            free(prevType);
            free(prevLvlText);
            prevType = curType;
            prevLvlText = curLvlText;
        }
        free(prevType);
        free(prevLvlText);
        DFHashTableRelease(selectorsByLevel);
        DFHashTableRelease(numStylesByLevel);
    }

    cssSelectors = DFHashTableCopyKeys(infoByStyleId);
    for (int i = 0; cssSelectors[i]; i++) {
        const char *selector = cssSelectors[i];
        WordNumInfo *info = DFHashTableLookup(infoByStyleId,selector);
        CSSStyle *cssStyle = CSSSheetLookupSelector(cssSheet,selector,0,0);
        if (cssStyle != NULL) {
            CSSPut(CSSStyleRule(cssStyle),"-word-numId",info->wordNumId);
            if (DFStringEquals(info->wordIlvl,"0"))
                CSSPut(CSSStyleRule(cssStyle),"-word-ilvl",NULL);
            else
                CSSPut(CSSStyleRule(cssStyle),"-word-ilvl",info->wordIlvl);
        }
    }
    free(cssSelectors);
    DFHashTableRelease(infoByStyleId);
}
