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
#include "WordNumbering.h"
#include "WordStyles.h"
#include "DFDOM.h"
#include "CSS.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordNumLevel                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordNumLevel *WordNumLevelNew(DFNode *element)
{
    const char *ilvl = NULL;
    const char *numFmt = NULL;
    const char *lvlText = NULL;

    ilvl = DFGetAttribute(element,WORD_ILVL);
    for (DFNode *child = element->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_NUMFMT:
                numFmt = DFGetAttribute(child,WORD_VAL);
                break;
            case WORD_LVLTEXT:
                lvlText = DFGetAttribute(child,WORD_VAL);
                break;
        }
    }

    if (ilvl == NULL)
        ilvl = "0";;

    WordNumLevel *level = (WordNumLevel *)calloc(1,sizeof(WordNumLevel));

    level->ilvl = atoi(ilvl);
    level->numFmt = (numFmt != NULL) ? strdup(numFmt) : NULL;
    level->lvlText = (lvlText != NULL) ? strdup(lvlText) : NULL;
    level->element = element;

    return level;
}

static void WordNumLevelFree(WordNumLevel *level)
{
    free(level->numFmt);
    free(level->lvlText);
    free(level);
}

const char *WordNumLevelToListStyleType(WordNumLevel *level)
{
    if (DFStringEquals(level->numFmt,"bullet")) {
        // UTF-8 encoding of chars U+F0B7 (disc) and U+F0A7 (square)
        const char disc[4] = { 0xEF, 0x82, 0xB7, 0 };
        const char square[4] = { 0xEF, 0x82, 0xA7, 0 };
        if (DFStringEquals(level->lvlText,disc))
            return "disc";
        else if (DFStringEquals(level->lvlText,"o"))
            return "circle";
        else if (DFStringEquals(level->lvlText,square))
            return "square";
        else
            return "disc";
    }
    else if (DFStringEquals(level->numFmt,"decimal")) {
        return "decimal";
    }
    else if (DFStringEquals(level->numFmt,"decimalZero")) {
        return "decimal-leading-zero";
    }
    else if (DFStringEquals(level->numFmt,"lowerLetter")) {
        return "lower-alpha";
    }
    else if (DFStringEquals(level->numFmt,"lowerRoman")) {
        return "lower-roman";
    }
    else if (DFStringEquals(level->numFmt,"none")) {
        return "none";
    }
    else if (DFStringEquals(level->numFmt,"upperLetter")) {
        return "upper-alpha";
    }
    else if (DFStringEquals(level->numFmt,"upperRoman")) {
        return "upper-roman";
    }
    return "decimal";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordAbstractNum                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static WordAbstractNum *WordAbstractNumNew(const char *abstractNumId1, DFNode *element1)
{
    WordAbstractNum *abs = (WordAbstractNum *)calloc(1,sizeof(WordAbstractNum));
    abs->abstractNumId = (abstractNumId1 != NULL) ? strdup(abstractNumId1) : NULL;
    abs->element = element1;
    abs->levels = DFHashTableNew(NULL,(DFFreeFunction)WordNumLevelFree);
    return abs;
}

static void WordAbstractNumFree(WordAbstractNum *abs)
{
    free(abs->abstractNumId);
    DFHashTableRelease(abs->levels);
    free(abs);
}

void WordAbstractNumAddLevel(WordAbstractNum *abs, WordNumLevel *numLevel)
{
    DFHashTableAddInt(abs->levels,numLevel->ilvl,numLevel);
}

WordNumLevel *WordAbstractNumGetLevel(WordAbstractNum *abs, int ilvl)
{
    return DFHashTableLookupInt(abs->levels,ilvl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordConcreteNum                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static WordConcreteNum *WordConcreteNumNew(const char *numId, DFNode *element, WordAbstractNum *abstractNum)
{
    WordConcreteNum *con = (WordConcreteNum *)calloc(1,sizeof(WordConcreteNum));
    con->numId = (numId != NULL) ? strdup(numId) : NULL;
    con->element = element;
    con->abstractNum = abstractNum;
    return con;
}

static void WordConcreteNumFree(WordConcreteNum *num)
{
    free(num->numId);
    free(num);
}

WordNumLevel *WordConcreteNumGetLevel(WordConcreteNum *con, int ilvl)
{
    return WordAbstractNumGetLevel(con->abstractNum,ilvl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordNumbering                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void WordNumberingParse(WordNumbering *num);

struct WordNumbering {
    WordPackage *_package;
    DFHashTable *_abstractNums;
    DFHashTable *_concreteNums;
    int _nextAbstractId;
    int _nextConcreteId;
    DFHashTable *_listStyleTypes;
};

static void WordNumberingRegisterType(WordNumbering *num, ListStyleType type, const char *name)
{
    char value[100];
    snprintf(value,100,"%d",type);
    DFHashTableAdd(num->_listStyleTypes,name,value);
}

WordNumbering *WordNumberingNew(WordPackage *package)
{
    WordNumbering *num = (WordNumbering *)calloc(1,sizeof(WordNumbering));
    num->_package = WordPackageRetain(package);
    num->_abstractNums = DFHashTableNew(NULL,(DFFreeFunction)WordAbstractNumFree);
    num->_concreteNums = DFHashTableNew(NULL,(DFFreeFunction)WordConcreteNumFree);
    num->_nextAbstractId = 1;
    num->_nextConcreteId = 1;
    num->_listStyleTypes = DFHashTableNew((DFCopyFunction)strdup,free);

    WordNumberingRegisterType(num,ListStyleTypeDecimal,"decimal");
    WordNumberingRegisterType(num,ListStyleTypeDecimalLeadingZero,"decimal-leading-zero");
    WordNumberingRegisterType(num,ListStyleTypeLowerAlpha,"lower-alpha");
    WordNumberingRegisterType(num,ListStyleTypeLowerRoman,"lower-roman");
    WordNumberingRegisterType(num,ListStyleTypeUpperAlpha,"upper-alpha");
    WordNumberingRegisterType(num,ListStyleTypeUpperRoman,"upper-roman");
    WordNumberingRegisterType(num,ListStyleTypeCircle,"circle");
    WordNumberingRegisterType(num,ListStyleTypeSquare,"square");
    WordNumberingRegisterType(num,ListStyleTypeDisc,"disc");

    WordNumberingParse(num);
    return num;
}

void WordNumberingFree(WordNumbering *num)
{
    DFHashTableRelease(num->_abstractNums);
    DFHashTableRelease(num->_concreteNums);
    DFHashTableRelease(num->_listStyleTypes);
    WordPackageRelease(num->_package);
    free(num);
}

static DFNode *WordNumberingRoot(WordNumbering *num)
{
    if (num->_package->numbering == NULL) {
        // FIXME: Not covered by test cases
        DFDocument *numbering = DFDocumentNewWithRoot(WORD_NUMBERING);
        WordPackageSetNumbering(num->_package,numbering);
        DFDocumentRelease(numbering);
    }
    return num->_package->numbering->root;
}

// UTF-8 encoding of square and bullet characters.
// Given as arrays since Visual C++ doesn't like \u escape sequences.
static char squareSymbol[4] = { 0xEF ,0x82 ,0xA7, 0x00 }; // "\uF0A7"
static char bulletSymbol[4] = { 0xEF ,0x82 ,0xB7, 0x00 }; // "\uF0B8"

WordNumLevel *WordNumberingCreateLevel(WordNumbering *num, const char *type, const char *lvlText, int ilvl, int indent)
{
    const char *font = NULL;
    const char *numFmt = NULL;
    const char *justify = "left";

    const char *typeNum = DFHashTableLookup(num->_listStyleTypes,type);
    ListStyleType typeVal = (typeNum != NULL) ? atoi(typeNum) : ListStyleTypeDisc;
    switch (typeVal) {
        case ListStyleTypeDecimal:
            numFmt = "decimal";
            break;
        case ListStyleTypeDecimalLeadingZero:
            numFmt = "decimalZero";
            break;
        case ListStyleTypeLowerAlpha:
            numFmt = "lowerLetter";
            break;
        case ListStyleTypeLowerRoman:
            numFmt = "lowerRoman";
            justify = "right";
            break;
        case ListStyleTypeUpperAlpha:
            numFmt = "upperLetter";
            break;
        case ListStyleTypeUpperRoman:
            numFmt = "upperRoman";
            justify = "right";
            break;
        case ListStyleTypeCircle:
            numFmt = "bullet";
            lvlText = "o";
            font = "Courier New";
            break;
        case ListStyleTypeSquare:
            numFmt = "bullet";
            lvlText = (const char *)squareSymbol;
            font = "Wingdings";
            break;
        case ListStyleTypeDisc:
        default:
            numFmt = "bullet";
            lvlText = (const char *)bulletSymbol;
            font = "Symbol";
            break;
    }

    if (!indent)
        justify = "left";;

    DFNode *lvlElem = DFCreateElement(num->_package->numbering,WORD_LVL);
    DFFormatAttribute(lvlElem,WORD_ILVL,"%d",ilvl);

    DFNode *startElem = DFCreateChildElement(lvlElem,WORD_START);
    DFSetAttribute(startElem,WORD_VAL,"1");

    DFNode *numFmtElem = DFCreateChildElement(lvlElem,WORD_NUMFMT);
    DFSetAttribute(numFmtElem,WORD_VAL,numFmt);

    DFNode *lvlTextElem = DFCreateChildElement(lvlElem,WORD_LVLTEXT);
    DFSetAttribute(lvlTextElem,WORD_VAL,lvlText);

    DFNode *lvlJcElem = DFCreateChildElement(lvlElem,WORD_LVLJC);
    DFSetAttribute(lvlJcElem,WORD_VAL,justify);

    if (indent) {
        DFNode *pPrElem = DFCreateChildElement(lvlElem,WORD_PPR);
        DFNode *indElem = DFCreateChildElement(pPrElem,WORD_IND);
        DFSetAttribute(indElem,WORD_HANGING,"360");
        DFFormatAttribute(indElem,WORD_LEFT,"%d",(ilvl+1)*720);
    }

    if (font != NULL) {
        DFNode *rPr = DFCreateChildElement(lvlElem,WORD_RPR);
        DFNode *rFonts = DFCreateChildElement(rPr,WORD_RFONTS);
        DFSetAttribute(rFonts,WORD_ASCII,font);
        DFSetAttribute(rFonts,WORD_HANSI,font);
        DFSetAttribute(rFonts,WORD_HINT,"default");
    }
    return WordNumLevelNew(lvlElem);
}

WordAbstractNum *WordNumberingCreateAbstractNum(WordNumbering *num)
{
    int abstractNumId = num->_nextAbstractId++;
    DFNode *root = WordNumberingRoot(num);
    DFNode *abstractNumElem = DFCreateElement(num->_package->numbering,WORD_ABSTRACTNUM);
    DFFormatAttribute(abstractNumElem,WORD_ABSTRACTNUMID,"%d",abstractNumId);

    DFNode *before = root->first;
    while ((before != NULL) && ((before)->tag == WORD_ABSTRACTNUM))
        before = before->next;
    DFInsertBefore(root,abstractNumElem,before);

    char abstractNumIdStr[100];
    snprintf(abstractNumIdStr,100,"%d",abstractNumId);
    WordAbstractNum *abstractNum = WordAbstractNumNew(abstractNumIdStr,abstractNumElem);
    DFHashTableAdd(num->_abstractNums,abstractNumIdStr,abstractNum);
    return abstractNum;
}

WordAbstractNum *WordNumberingCreateAbstractNumWithType(WordNumbering *num, const char *type)
{
    WordAbstractNum *abstractNum = WordNumberingCreateAbstractNum(num);

    for (int ilvl = 0; ilvl < 9; ilvl++) {
        char *lvlText = DFFormatString("%%%d.",ilvl+1);
        WordNumLevel *numLevel = WordNumberingCreateLevel(num,type,lvlText,ilvl,1);
        DFAppendChild(abstractNum->element,numLevel->element);
        WordAbstractNumAddLevel(abstractNum,numLevel);
        free(lvlText);
    }

    return abstractNum;
}

WordConcreteNum *WordNumberingConcreteWithId(WordNumbering *num, const char *numId)
{
    return DFHashTableLookup(num->_concreteNums,numId);
}

WordConcreteNum *WordNumberingAddConcreteWithAbstract(WordNumbering *num, WordAbstractNum *abstractNum)
{
    int concId = num->_nextConcreteId++;
    DFNode *root = WordNumberingRoot(num);
    DFNode *numElem = DFCreateElement(num->_package->numbering,WORD_NUM);
    DFNode *abstractNumIdElem = DFCreateElement(num->_package->numbering,WORD_ABSTRACTNUMID);

    DFFormatAttribute(numElem,WORD_NUMID,"%d",concId);
    DFSetAttribute(abstractNumIdElem,WORD_VAL,abstractNum->abstractNumId);

    DFAppendChild(numElem,abstractNumIdElem);
    DFAppendChild(root,numElem);

    char concIdStr[100];
    snprintf(concIdStr,100,"%d",concId);
    WordConcreteNum *concreteNum = WordConcreteNumNew(concIdStr,numElem,abstractNum);
    DFHashTableAdd(num->_concreteNums,concIdStr,concreteNum);
    return concreteNum;
}

static void WordNumberingParse(WordNumbering *num)
{
    if (num->_package->numbering == NULL)
        return;
    if (num->_package->numbering->root->tag != WORD_NUMBERING)
        return;

    // Find abstract numbering definitions
    for (DFNode *child = num->_package->numbering->root->first; child != NULL; child = child->next) {
        if (child->tag == WORD_ABSTRACTNUM) {
            const char *absIdStr = DFGetAttribute(child,WORD_ABSTRACTNUMID);
            if (absIdStr == NULL)
                continue;
            WordAbstractNum *abstractNum = WordAbstractNumNew(absIdStr,child);
            DFHashTableAdd(num->_abstractNums,absIdStr,abstractNum);
            if (num->_nextAbstractId < atoi(absIdStr)+1)
                num->_nextAbstractId = atoi(absIdStr)+1;

            for (DFNode *anChild = child->first; anChild != NULL; anChild = anChild->next) {
                if (anChild->tag == WORD_LVL) {
                    WordNumLevel *numLevel = WordNumLevelNew(anChild);
                    WordAbstractNumAddLevel(abstractNum,numLevel);
                }
            }
        }
    }

    // Find concrete numbering definitions
    for (DFNode *child = num->_package->numbering->root->first; child != NULL; child = child->next) {
        if (child->tag == WORD_NUM) {
            const char *concIdStr = DFGetAttribute(child,WORD_NUMID);
            if (concIdStr == NULL)
                continue;
            DFNode *absIdElem = DFChildWithTag(child,WORD_ABSTRACTNUMID);
            if (absIdElem == NULL)
                continue;
            const char *absIdStr = DFGetAttribute(absIdElem,WORD_VAL);
            if (absIdStr == NULL)
                continue;

            WordAbstractNum *abstractNum = DFHashTableLookup(num->_abstractNums,absIdStr);
            if (abstractNum == NULL)
                continue;

            WordConcreteNum *concreteNum = WordConcreteNumNew(concIdStr,child,abstractNum);
            DFHashTableAdd(num->_concreteNums,concIdStr,concreteNum);
            if (num->_nextConcreteId < atoi(concIdStr)+1)
                num->_nextConcreteId = atoi(concIdStr)+1;
        }
    }
}

void WordNumberingRemoveConcrete(WordNumbering *num, WordConcreteNum *concrete)
{
    assert(DFHashTableLookup(num->_concreteNums,concrete->numId) == concrete);
    assert(concrete->element->parent != NULL);
    DFRemoveNode(concrete->element);
    DFHashTableRemove(num->_concreteNums,concrete->numId);
}

void WordNumberingRemoveUnusedAbstractNums(WordNumbering *num)
{
    const char **abstractKeys = DFHashTableCopyKeys(num->_abstractNums);
    for (int i = 0; abstractKeys[i]; i++) {
        WordAbstractNum *abstract = DFHashTableLookup(num->_abstractNums,abstractKeys[i]);
        abstract->refCount = 0;
    }

    const char **concreteKeys = DFHashTableCopyKeys(num->_concreteNums);
    for (int i = 0; concreteKeys[i]; i++) {
        WordConcreteNum *concrete = DFHashTableLookup(num->_concreteNums,concreteKeys[i]);
        concrete->abstractNum->refCount++;
    }
    free(concreteKeys);

    for (int i = 0; abstractKeys[i]; i++) {
        WordAbstractNum *abstract = DFHashTableLookup(num->_abstractNums,abstractKeys[i]);
        if (abstract->refCount == 0) {
            assert(abstract->element->parent != NULL);
            DFRemoveNode(abstract->element);
            DFHashTableRemove(num->_abstractNums,abstract->abstractNumId);
        }
    }
    free(abstractKeys);
}
