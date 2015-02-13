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
#include "WordConverter.h"
#include "WordBookmark.h"
#include "WordField.h"
#include "WordStyles.h"
#include "WordSheet.h"
#include "WordNotes.h"
#include "WordNumbering.h"
#include "WordSection.h"
#include "WordSettings.h"
#include "WordObjects.h"
#include "WordLists.h"
#include "WordGC.h"
#include "WordLenses.h"
#include "WordCaption.h"
#include "WordWhitespace.h"
#include "WordTheme.h"
#include "OPC.h"
#include "DFDOM.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "DFBDT.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSLength.h"
#include "CSSSelector.h"
#include "CSSClassNames.h"
#include "CSSSheet.h"
#include "CSSStyle.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int isWhitespaceRun(DFNode *run)
{
    for (DFNode *child = run->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_RPR:
                break;
            case WORD_T: {
                char *str = DFNodeTextToString(child);
                int isWhitespace = DFStringIsWhitespace(str);
                free(str);
                if (!isWhitespace)
                    return 0;
                break;
            }
            default:
                return 0;
        }
    }
    return 1;
}

int Word_isFigureParagraph(DFNode *p)
{
    // A paragraph is a figure if it contains only a single run, and that run contains a drawing
    if ((p == NULL) || (p->tag != WORD_P))
        return 0;;

    DFNode *run = NULL;
    int runCount = 0;
    for (DFNode *child = p->first; child != NULL; child = child->next) {
        if (child->tag == WORD_R) {
            if (isWhitespaceRun(child))
                continue;
            run = child;
            runCount++;
        }
    }

    if (runCount != 1)
        return 0;

    for (DFNode *child = run->first; child != NULL; child = child->next) {
        switch (child->tag) {
            case WORD_DRAWING:
            case WORD_OBJECT:
            case WORD_PICT:
                return 1;
        }
    }

    return 0;
}

int Word_isEquationParagraph(DFNode *p)
{
    if ((p == NULL) || (p->tag != WORD_P))
        return 0;

    for (DFNode *child = p->first; child != NULL; child = child->next) {
        if (child->tag == MATH_OMATHPARA)
            return 1;
    }

    return 0;
}

static int attributesEqual(DFNode *elemA, DFNode *elemB)
{
    if (elemA->attrsCount != elemB->attrsCount)
        return 0;

    int count = elemA->attrsCount;
    for (int ai = 0; ai < count; ai++) {
        DFAttribute *attrA = &elemA->attrs[ai];
        int found = 0;
        for (int bi = 0; bi < count; bi++) {
            DFAttribute *attrB = &elemB->attrs[bi];
            if (attrA->tag == attrB->tag) {
                if (strcmp(attrA->value,attrB->value))
                    return 0;
                found = 1;
                break;
            }
        }
        if (!found)
            return 0;
    }

    return 1;
}

static int nodesEqual(DFNode *a, DFNode *b)
{
    if ((a == NULL) && (b == NULL))
        return 1;

    if ((a == NULL) || (b == NULL))
        return 0;

    if (a->tag != b->tag)
        return 0;

    if (a->tag < MIN_ELEMENT_TAG)
        return 0;;

    // First check if the number and type of children are the same
    DFNode *aChild = a->first;
    DFNode *bChild = b->first;
    while ((aChild != NULL) || (bChild != NULL)) {
        if ((aChild != NULL) && (bChild == NULL))
            return 0;
        if ((aChild == NULL) && (bChild != NULL))
            return 0;
        if (aChild->tag != bChild->tag)
            return 0;
        aChild = aChild->next;
        bChild = bChild->next;
    }

    // Next check the attributes
    if (!attributesEqual(a,b))
        return 0;

    // Now check the *content* of the children. We do this after the above as it is more expensive.
    aChild = a->first;
    bChild = b->first;
    while ((aChild != NULL) || (bChild != NULL)) {
        if (!nodesEqual(aChild,bChild))
            return 0;
        aChild = aChild->next;
        bChild = bChild->next;
    }
    return 1;
}

static void Word_mergeRunsRecursive(DFNode *node)
{
    DFNode *current = node->first;
    while (current != NULL) {
        DFNode *next = current->next;

        if ((current->tag == WORD_R) && (next != NULL) && (next->tag == WORD_R)) {
            DFNode *currentRPr = DFChildWithTag(current,WORD_RPR);
            DFNode *nextRPr = DFChildWithTag(next,WORD_RPR);
            if (nodesEqual(currentRPr,nextRPr)) {
                while (next->first != NULL) {
                    if (next->first->tag == WORD_RPR)
                        DFRemoveNode(next->first);
                    else
                        DFAppendChild(current,next->first);
                }
                DFRemoveNode(next);
                continue;
            }
        }

        current = next;
    }

    for (current = node->first; current != NULL; current = current->next)
        Word_mergeRunsRecursive(current);
}

static void Word_mergeRuns(WordPackage *package)
{
    if (package->document != NULL)
        Word_mergeRunsRecursive(package->document->docNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                  HTML pre- and post-processing                                 //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void Word_addContentParts(DFNode *child, const char *content, WordCaption *caption)
{
    if (content == NULL)
        return;;
    DFNode *nextSibling = child->first;
    DFArray *parts = CSSParseContent(content);
    for (size_t i = 0; i < DFArrayCount(parts); i++) {
        ContentPart *part = DFArrayItemAt(parts,i);
        switch (part->type) {
            case ContentPartString: {
                DFNode *text = DFCreateTextNode(child->doc,part->value);
                if (strlen(part->value) > 0) {
                    DFNode *span = DFCreateElement(child->doc,HTML_SPAN);
                    DFAppendChild(span,text);
                    DFInsertBefore(child,span,nextSibling);
                }
                break;
            }
            case ContentPartCounter: {
                if (DFStringEquals(part->value,"figure")) {
                    DFNode *span = DFCreateElement(child->doc,HTML_SPAN);
                    DFSetAttribute(span,HTML_CLASS,DFFieldClass);
                    DFCreateChildTextNode(span," SEQ Figure \\* ARABIC ");
                    DFInsertBefore(child,span,nextSibling);
                    caption->number = span;
                }
                else if (DFStringEquals(part->value,"table")) {
                    DFNode *span = DFCreateElement(child->doc,HTML_SPAN);
                    DFSetAttribute(span,HTML_CLASS,DFFieldClass);
                    DFCreateChildTextNode(span," SEQ Table \\* ARABIC ");
                    DFInsertBefore(child,span,nextSibling);
                    caption->number = span;
                }
                break;
            default:
                break;
            }
        }
    }
    DFArrayRelease(parts);
}

static void Word_preProcessHTML(WordConverter *word, DFNode *node)
{
    switch (node->tag) {
        case HTML_TABLE:
        case HTML_FIGURE: {
            DFNode *next;
            for (DFNode *child = node->first; child != NULL; child = next) {
                next = child->next;

                if ((child->tag != HTML_CAPTION) && (child->tag != HTML_FIGCAPTION))
                    continue;

                WordCaption *caption = WordCaptionNew(child);
                WordObjectsSetCaption(word->objects,caption,node);
                caption->contentStart = child->first;
                WordCaptionRelease(caption);

                const char *className = DFGetAttribute(child,HTML_CLASS);
                CSSStyle *style;
                if (child->tag == HTML_CAPTION)
                    style = CSSSheetLookupElement(word->styleSheet,"caption",className,0,0);
                else
                    style = CSSSheetLookupElement(word->styleSheet,"figcaption",className,0,0);

                CSSProperties *before = CSSStyleBefore(style);
                if (CSSGet(before,"content") != NULL)
                    Word_addContentParts(child,CSSGet(before,"content"),caption);

                child->tag = HTML_P;
                DFSetAttribute(child,HTML_CLASS,"Caption");
                DFInsertBefore(node->parent,child,node->next);
                Word_preProcessHTML(word,child);
            }

            // The HTML normalization process ensures that apart from the <figcaption> element,
            // all children of a <figure> are paragraphs or containers. Currently the editor only
            // lets you create figures that contain a single image, so it's always a single
            // paragraph. Since the HTML <figure> element gets mapped to a single <w:p> element
            // by WordParagraphLens, we want to make sure it only contains inline children.

            for (DFNode *child = node->first; child != NULL; child = next) {
                next = child->next;
                if (HTML_isParagraphTag(child->tag))
                    DFRemoveNodeButKeepChildren(child);
            }

            // FIXME: Handle <div>, <pre>, lists, tables etc which could also theoretically
            // exist inside the <figure> element

            break;
        }
        case HTML_NAV: {
            const char *className = DFGetAttribute(node,HTML_CLASS);
            const char *instr = NULL;
            if (DFStringEquals(className,DFTableOfContentsClass))
                instr = " TOC \\o \"1-3\" ";
            else if (DFStringEquals(className,DFListOfFiguresClass))
                instr = " TOC \\c \"Figure\" ";
            else if (DFStringEquals(className,DFListOfTablesClass))
                instr = " TOC \\c \"Table\" ";

            if (instr != NULL) {
                DFNode *p = DFCreateElement(word->html,HTML_P);
                DFNode *field = DFCreateChildElement(p,HTML_SPAN);
                DFSetAttribute(field,HTML_CLASS,DFFieldClass);
                DFCreateChildTextNode(field,instr);
                DFInsertBefore(node->parent,p,node);
                DFRemoveNode(node);
            }
            break;
        }
    }

    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        Word_preProcessHTML(word,child);
    }
}

static void Word_preProcessHTMLDoc(WordConverter *word, DFDocument *doc)
{
    WordPreProcessHTMLLists(word);
    Word_preProcessHTML(word,doc->docNode);
}

static int isSeqField(DFNode *node)
{
    if (node->tag != HTML_SPAN)
        return 0;
    if (!DFStringEquals(DFGetAttribute(node,HTML_CLASS),DFFieldClass))
        return 0;
    char *instr = DFNodeTextToString(node);
    const char **args = Word_parseField(instr);
    int result = (args[0] != NULL) && !strcmp(args[0],"SEQ");
    free(args);
    free(instr);
    return result;
}

static DFNode *findSeqChild(DFNode *parent)
{
    for (DFNode *child = parent->first; child != NULL; child = child->next) {
        if (isSeqField(child))
            return child;;
        DFNode *result = findSeqChild(child);
        if (result != NULL)
            return result;
    }
    return NULL;
}

static void extractPrefixRecursive(DFNode *node, const char *counterName, DFBuffer *result,
                                   int *foundSeq, int *foundContent)
{
    if (isSeqField(node)) {
        if (result->len > 0)
            DFBufferFormat(result," ");
        DFBufferFormat(result,"counter(%s)",counterName);
        *foundSeq = 1;
        DFRemoveNode(node);
        return;
    }

    if (node->tag == DOM_TEXT) {
        size_t valueLen = strlen(node->value);
        size_t pos = 0;

        if (*foundSeq) {
            size_t offset = 0;
            uint32_t ch;
            do {
                pos = offset;
                ch = DFNextChar(node->value,&offset);
            } while ((ch != 0) && (DFCharIsWhitespaceOrNewline(ch) || DFCharIsPunctuation(ch)));
        }
        else {
            pos = valueLen;
        }

        if (pos == valueLen) {
            if (result->len > 0)
                DFBufferFormat(result," ");
            char *quotedValue = DFQuote(node->value);
            DFBufferFormat(result,"%s",quotedValue);
            free(quotedValue);
            DFRemoveNode(node);
            if (*foundSeq)
                *foundContent = 1;
            return;
        }
        else if (pos > 0) {
            char *first = DFSubstring(node->value,0,pos);
            char *rest = DFSubstring(node->value,pos,valueLen);
            if (result->len > 0)
                DFBufferFormat(result," ");
            char *quotedFirst = DFQuote(first);
            DFBufferFormat(result,"%s",quotedFirst);
            free(quotedFirst);
            DFSetNodeValue(node,rest);
            if (*foundSeq)
                *foundContent = 1;
            free(first);
            free(rest);
            return;
        }
    }

    int wasEmpty = (node->first == NULL);
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        if (*foundContent)
            break;
        extractPrefixRecursive(child,counterName,result,foundSeq,foundContent);
    }
    int isEmpty = (node->first == NULL);
    if ((node->tag == HTML_SPAN) && isEmpty && !wasEmpty)
        DFRemoveNode(node);
}

static char *extractPrefix(DFNode *node, const char *counterName)
{
    if (findSeqChild(node) == NULL)
        return NULL;;
    DFBuffer *result = DFBufferNew();
    int foundSeq = 0;
    int foundContent = 0;
    extractPrefixRecursive(node,counterName,result,&foundSeq,&foundContent);
    char *str = strdup(result->data);
    DFBufferRelease(result);
    return str;
}

static void Word_postProcessHTML(WordConverter *conv, DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;

        switch (child->tag) {
            case HTML_SPAN: {
                const char *className = DFGetAttribute(child,HTML_CLASS);
                if (DFStringEquals(className,DFBookmarkClass)) {
                    if (child->first != NULL)
                        next = child->first;
                    DFRemoveNodeButKeepChildren(child);
                }
                break;
            }
            case HTML_CAPTION: {
                const char *counterName = NULL;

                if ((child->prev != NULL) && (child->prev->tag == HTML_FIGURE) &&
                    (DFChildWithTag(child->prev,HTML_FIGCAPTION) == NULL)) {
                    child->tag = HTML_FIGCAPTION;
                    counterName = "figure";
                    DFAppendChild(child->prev,child);
                }
                else if ((child->prev != NULL) && (child->prev->tag == HTML_TABLE) &&
                         (DFChildWithTag(child->prev,HTML_CAPTION) == NULL)) {
                    counterName = "table";
                    DFInsertBefore(child->prev,child,child->prev->first);
                }
                else if ((child->next != NULL) && (child->next->tag == HTML_FIGURE) &&
                         (DFChildWithTag(child->next,HTML_FIGCAPTION) == NULL)) {
                    child->tag = HTML_FIGCAPTION;
                    counterName = "figure";
                    DFInsertBefore(child->next,child,child->next->first);
                }
                else if ((child->next != NULL) && (child->next->tag == HTML_TABLE) &&
                         (DFChildWithTag(child->next,HTML_CAPTION) == NULL)) {
                    counterName = "table";
                    DFSetAttribute(child,HTML_STYLE,"caption-side: top");
                    DFInsertBefore(child->next,child,child->next->first);
                }

                if (counterName != NULL) {
                    char *beforeText = extractPrefix(child,counterName);
                    if (beforeText != NULL) {
                        CSSStyle *style = CSSSheetLookupElement(conv->styleSheet,DFNodeName(child),NULL,1,0);
                        if (CSSGet(CSSStyleBefore(style),"content") == NULL) {
                            CSSPut(CSSStyleRule(style),"counter-increment",counterName);
                            CSSPut(CSSStyleBefore(style),"content",beforeText);
                        }
                    }
                    free(beforeText);
                }
                break;
            }
            case HTML_NAV: {
                if (HTML_isParagraphTag(node->tag)) {

                    if (child->prev != NULL) {
                        DFNode *beforeP = DFCreateElement(conv->html,node->tag);
                        while (child->prev != NULL)
                            DFInsertBefore(beforeP,child->prev,beforeP->first);
                        DFInsertBefore(node->parent,beforeP,node);
                    }
                    DFInsertBefore(node->parent,child,node);

                    if ((node->first == NULL) ||
                        ((node->first->tag == HTML_BR) && (node->first->next == NULL))) {
                        DFRemoveNode(node);
                        return;
                    }
                    next = NULL;
                }
                break;
            }
        }
    }

    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        Word_postProcessHTML(conv,child);
    }
}

static void Word_postProcessHTMLDoc(WordConverter *conv)
{
    WordPostProcessHTMLLists(conv);
    Word_postProcessHTML(conv,conv->html->docNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordConverter                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static WordConverter *WordConverterNew(DFDocument *html, DFStorage *abstractStorage, WordPackage *package)
{
    WordConverter *converter = (WordConverter *)calloc(1,sizeof(WordConverter));
    converter->html = DFDocumentRetain(html);
    converter->abstractStorage = DFStorageRetain(abstractStorage);
    assert(DFStorageFormat(converter->abstractStorage) == DFFileFormatHTML);
    converter->idPrefix = strdup("word");
    converter->package = WordPackageRetain(package);
    converter->styles = WordSheetNew(converter->package->styles);
    converter->numbering = WordNumberingNew(converter->package);
    converter->theme = WordThemeNew(converter->package);
    converter->mainSection = WordSectionNew();
    converter->objects = WordObjectsNew(converter->package);
    converter->footnotes = WordNoteGroupNewFootnotes(converter->package->footnotes);
    converter->endnotes = WordNoteGroupNewEndnotes(converter->package->endnotes);
    converter->supportedContentTypes = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(converter->supportedContentTypes,"jpg","image/jpeg");
    DFHashTableAdd(converter->supportedContentTypes,"jpeg","image/jpeg");
    DFHashTableAdd(converter->supportedContentTypes,"tif","image/tiff");
    DFHashTableAdd(converter->supportedContentTypes,"tiff","image/tiff");
    DFHashTableAdd(converter->supportedContentTypes,"gif","image/gif");
    DFHashTableAdd(converter->supportedContentTypes,"bmp","image/bmp");
    DFHashTableAdd(converter->supportedContentTypes,"png","image/png");
    converter->warnings = DFBufferNew();
    return converter;
}

static void WordConverterFree(WordConverter *converter)
{
    DFDocumentRelease(converter->html);
    DFStorageRelease(converter->abstractStorage);
    free(converter->idPrefix);
    WordSheetFree(converter->styles);
    WordNumberingFree(converter->numbering);
    WordThemeFree(converter->theme);
    WordSectionFree(converter->mainSection);
    WordObjectsFree(converter->objects);
    WordNoteGroupRelease(converter->footnotes);
    WordNoteGroupRelease(converter->endnotes);
    DFHashTableRelease(converter->supportedContentTypes);
    DFBufferRelease(converter->warnings);
    CSSSheetRelease(converter->styleSheet);
    WordPackageRelease(converter->package);
    free(converter);
}

DFNode *WordConverterCreateAbstract(WordGetData *get, Tag tag, DFNode *concrete)
{
    DFNode *element = DFCreateElement(get->conv->html,tag);
    if (concrete != NULL) {
        char *idStr;
        if (concrete->doc == get->conv->package->document)
            idStr = DFFormatString("%s%u",get->conv->idPrefix,concrete->seqNo);
        else
            idStr = DFFormatString("%s%u-%s",get->conv->idPrefix,concrete->seqNo,DFNodeName(concrete->doc->root));
        DFSetAttribute(element,HTML_ID,idStr);
        free(idStr);
    }
    return element;
}

DFNode *WordConverterGetConcrete(WordPutData *put, DFNode *abstract)
{
    // Is the abstract node an element, and does it have an id that matches the prefix used for
    // conversion? That is, does it look like it has a corresponding node in the concrete document?
    if ((abstract == NULL) || (abstract->tag < MIN_ELEMENT_TAG))
        return NULL;;
    const char *idStr = DFGetAttribute(abstract,HTML_ID);
    if ((idStr == NULL) || !DFStringHasPrefix(idStr,put->conv->idPrefix))
        return NULL;;

    // Determine the node sequence number and the document based on the id attribute.
    // The format of the attribute is <prefix><seqno>(-<docname>)?, where
    //
    //     <prefix>  is the BDT prefix we use to identify nodes that match the original document
    //     <seqno>   is an integer uniquely identifying a node in a given document
    //     <docname> is the name of the document, either footnotes or endnotes. If absent, it is
    //               the main content document (that is, document.xml)
    //
    // Note that the sequence number only makes sense within the context of a specific document. It
    // is possible to have two different nodes in different documents that have the same sequence number.
    // It is for this reason that the id string identifies both the node and the document.

    size_t idLen = strlen(idStr);
    size_t prefixLen = strlen(put->conv->idPrefix);

    unsigned int seqNo = 0;
    size_t pos = prefixLen;
    while ((pos < idLen) && (idStr[pos] >= '0') && (idStr[pos] <= '9'))
        seqNo = seqNo*10 + (idStr[pos++] - '0');

    const char *docName = NULL;
    if ((pos < idLen) && (idStr[pos] == '-')) {
        pos++;
        docName = &idStr[pos];
    }

    DFDocument *doc = NULL;
    if (docName == NULL)
        doc = put->conv->package->document;
    else if (!strcmp(docName,"footnotes"))
        doc = put->conv->package->footnotes;
    else if (!strcmp(docName,"endnotes"))
        doc = put->conv->package->endnotes;
    else
        return NULL;

    // Check to see if we have a node in the concrete document matching that sequence number
    DFNode *node = DFNodeForSeqNo(doc,seqNo);

    // Only return the node if it's actually an element
    if ((node == NULL) || (node->tag < MIN_ELEMENT_TAG))
        return NULL;
    return node;
}

int WordConverterGet(DFDocument *html, DFStorage *abstractStorage, WordPackage *package, DFError **error)
{
    if (package->document == NULL) {
        DFErrorFormat(error,"document.xml not found");
        return 0;
    }

    DFNode *wordDocument = DFChildWithTag(package->document->docNode,WORD_DOCUMENT);
    if (wordDocument == NULL) {
        DFErrorFormat(error,"word:document not found");
        return 0;
    }

    int haveFields = Word_simplifyFields(package);
    Word_mergeRuns(package);

    WordConverter *converter = WordConverterNew(html,abstractStorage,package);
    converter->haveFields = haveFields;
    WordAddNbsps(converter->package->document);
    WordFixLists(converter);

    CSSSheetRelease(converter->styleSheet);
    converter->styleSheet = WordParseStyles(converter);
    WordObjectsCollapseBookmarks(converter->objects);
    WordObjectsScan(converter->objects);
    WordObjectsAnalyzeBookmarks(converter->objects,converter->styles);

    WordGetData get;
    get.conv = converter;
    DFNode *abstract = WordDocumentLens.get(&get,wordDocument);
    DFAppendChild(converter->html->docNode,abstract);
    Word_postProcessHTMLDoc(converter);

    HTMLAddExternalStyleSheet(converter->html,"reset.css");
    char *cssText = CSSSheetCopyCSSText(converter->styleSheet);
    HTMLAddInternalStyleSheet(converter->html,cssText);
    free(cssText);

    HTML_safeIndent(converter->html->docNode,0);

    int ok = 1;
    if (converter->warnings->len > 0) {
        DFErrorFormat(error,"%s",converter->warnings->data);
        ok = 0;
    }

    WordConverterFree(converter);
    return ok;
}

static void buildListMapFromHTML(WordPutData *put, DFNode *node)
{
    if (node->tag == HTML_P) {
        const char *htmlId = DFGetAttribute(node,CONV_LISTNUM);
        DFNode *conElem = (htmlId != NULL) ? WordConverterGetConcrete(put,node) : NULL;
        DFNode *pPrElem = (conElem != NULL) ? DFChildWithTag(conElem,WORD_PPR) : NULL;
        DFNode *numPrElem = (pPrElem != NULL) ? DFChildWithTag(pPrElem,WORD_NUMPR) : NULL;
        DFNode *numIdElem = (numPrElem != NULL) ? DFChildWithTag(numPrElem,WORD_NUMID) : NULL;
        const char *numId = (numIdElem != NULL) ? DFGetAttribute(numIdElem,WORD_VAL) : NULL;

        if (numId != NULL) {
            const char *existingHtmlId = DFHashTableLookup(put->htmlIdByNumId,numId);
            const char *existingNumId = DFHashTableLookup(put->numIdByHtmlId,htmlId);
            if ((existingHtmlId == NULL) && (existingNumId == NULL)) {
                DFHashTableAdd(put->htmlIdByNumId,numId,htmlId);
                DFHashTableAdd(put->numIdByHtmlId,htmlId,numId);

                WordConcreteNum *num = WordNumberingConcreteWithId(put->conv->numbering,numId);
                if (num != NULL)
                    num->referenceCount++;
            }
        }
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        buildListMapFromHTML(put,child);
}

static void updateListTypes(WordPutData *put)
{
    const char **htmlIds = DFHashTableCopyKeys(put->numIdByHtmlId);
    for (int i = 0; htmlIds[i]; i++) {
        const char *htmlId = htmlIds[i];
        const char *numId = DFHashTableLookup(put->numIdByHtmlId,htmlId);
        WordConcreteNum *num = WordNumberingConcreteWithId(put->conv->numbering,numId);
        if (num == NULL)
            continue; // FIXME: remove entry from both maps so it is re-created
        DFNode *listNode = DFNodeForSeqNo(put->conv->html,(unsigned int)atoi(htmlId));
        assert(listNode != NULL);

        const char *htmlType = DFGetAttribute(listNode,CONV_LISTTYPE);
        const char *htmlIlvl = DFGetAttribute(listNode,CONV_ILVL);

        WordNumLevel *level = WordConcreteNumGetLevel(num,atoi(htmlIlvl));
        if (level == NULL)
            continue; // FIXME: remove entry from both maps so it is re-created

        const char *wordType = WordNumLevelToListStyleType(level);

        if (!DFStringEquals(wordType,htmlType)) {
            // Make a copy of numId, as it may be freed during the first call to DFHashTableRemove
            char *numIdCopy = strdup(numId);
            DFHashTableRemove(put->numIdByHtmlId,htmlId);
            DFHashTableRemove(put->htmlIdByNumId,numIdCopy);
            free(numIdCopy);
            if (num->referenceCount == 1)
                WordNumberingRemoveConcrete(put->conv->numbering,num);
        }
    }
    free(htmlIds);
}

static void addMissingDefaultStyles(WordConverter *converter)
{
    if (CSSSheetDefaultStyleForFamily(converter->styleSheet,StyleFamilyParagraph) == NULL) {
        CSSStyle *style = CSSSheetLookupElement(converter->styleSheet,"p","Normal",1,0);
        CSSSheetSetDefaultStyle(converter->styleSheet,style,StyleFamilyParagraph);
    }
    if (CSSSheetDefaultStyleForFamily(converter->styleSheet,StyleFamilyCharacter) == NULL) {
        CSSStyle *style = CSSSheetLookupElement(converter->styleSheet,"span","DefaultParagraphFont",1,0);
        CSSStyleSetDisplayName(style,"Default Paragraph Font");
        CSSSheetSetDefaultStyle(converter->styleSheet,style,StyleFamilyCharacter);
    }
    if (CSSSheetDefaultStyleForFamily(converter->styleSheet,StyleFamilyTable) == NULL) {
        CSSStyle *style = CSSSheetLookupElement(converter->styleSheet,"table","Normal_Table",1,0);
        CSSStyleSetDisplayName(style,"Normal Table");
        CSSPut(CSSStyleCell(style),"padding-left","5.4pt");
        CSSPut(CSSStyleCell(style),"padding-right","5.4pt");
        CSSPut(CSSStyleCell(style),"padding-top","0pt");
        CSSPut(CSSStyleCell(style),"padding-bottom","0pt");
        CSSSheetSetDefaultStyle(converter->styleSheet,style,StyleFamilyTable);
    }
}

int WordConverterPut(DFDocument *html, DFStorage *abstractStorage, WordPackage *package, DFError **error)
{
    if (package->document == NULL) {
        DFErrorFormat(error,"document.xml not found");
        return 0;
    }

    DFNode *wordDocument = DFChildWithTag(package->document->docNode,WORD_DOCUMENT);
    if (wordDocument == NULL) {
        DFErrorFormat(error,"word:document not found");
        return 0;
    }

    HTML_normalizeDocument(html);
    HTML_pushDownInlineProperties(html->docNode);

    WordConverter *converter = WordConverterNew(html,abstractStorage,package);

    // FIXME: Need a more reliable way of telling whether this is a new document or not - it could be that the
    // document already existed (with styles set up) but did not have any content
    DFNode *wordBody = DFChildWithTag(wordDocument,WORD_BODY);
    int creating = ((wordBody == NULL) || (wordBody->first == NULL));

    converter->haveFields = Word_simplifyFields(converter->package);
    Word_mergeRuns(converter->package);

    assert(converter->package->styles);

    CSSSheetRelease(converter->styleSheet);
    converter->styleSheet = CSSSheetNew();

    char *cssText = HTMLCopyCSSText(converter->html);
    CSSSheetUpdateFromCSSText(converter->styleSheet,cssText);
    free(cssText);

    addMissingDefaultStyles(converter);
    CSSEnsureReferencedStylesPresent(converter->html,converter->styleSheet);
    if (creating)
        CSSSetHTMLDefaults(converter->styleSheet);
    CSSEnsureUnique(converter->styleSheet,converter->html,creating);

    CSSStyle *pageStyle = CSSSheetLookupElement(converter->styleSheet,"@page",NULL,0,0);
    CSSStyle *bodyStyle = CSSSheetLookupElement(converter->styleSheet,"body",NULL,1,0);
    CSSProperties *page = (pageStyle != NULL) ? CSSPropertiesRetain(CSSStyleRule(pageStyle)) : CSSPropertiesNew();
    CSSProperties *body = (bodyStyle != NULL) ? CSSPropertiesRetain(CSSStyleRule(bodyStyle)) : CSSPropertiesNew();

    if (CSSGet(body,"margin-left") == NULL)
        CSSPut(body,"margin-left","10%");
    if (CSSGet(body,"margin-right") == NULL)
        CSSPut(body,"margin-right","10%");
    if (CSSGet(body,"margin-top") == NULL)
        CSSPut(body,"margin-top","10%");
    if (CSSGet(body,"margin-bottom") == NULL)
        CSSPut(body,"margin-bottom","10%");

    WordSectionUpdateFromCSSPage(converter->mainSection,page,body);

    WordPutData put;
    put.conv = converter;
    put.contentDoc = converter->package->document;
    put.numIdByHtmlId = DFHashTableNew((DFCopyFunction)strdup,free);
    put.htmlIdByNumId = DFHashTableNew((DFCopyFunction)strdup,free);

    // Make sure we update styles.xml from the CSS stylesheet *before* doing any conversion of the content,
    // since the latter requires a full mapping of CSS selectors to styleIds to be in place.
    WordUpdateStyles(converter,converter->styleSheet);

    Word_preProcessHTMLDoc(converter,converter->html);
    buildListMapFromHTML(&put,converter->html->docNode);
    updateListTypes(&put);
    WordBookmarks_removeCaptionBookmarks(converter->package->document);
    WordObjectsCollapseBookmarks(converter->objects);
    WordObjectsScan(converter->objects);
    Word_setupBookmarkLinks(&put);
    WordObjectsAnalyzeBookmarks(converter->objects,converter->styles);
    WordDocumentLens.put(&put,converter->html->root,wordDocument);
    WordObjectsExpandBookmarks(converter->objects);
    WordRemoveNbsps(converter->package->document);

    // Make sure the updateFields flag is set
    Word_updateSettings(converter->package,converter->haveFields);

    // Remove any abstract numbering definitions that are no longer referenced from concrete
    // numbering definitions
    WordNumberingRemoveUnusedAbstractNums(converter->numbering);

    // Remove any relationships and images that have been removed from the HTML file and no longer
    // have any other references pointing to them
    WordGarbageCollect(converter->package);

    CSSPropertiesRelease(page);
    CSSPropertiesRelease(body);
    DFHashTableRelease(put.numIdByHtmlId);
    DFHashTableRelease(put.htmlIdByNumId);

    int ok = 1;
    if (converter->warnings->len > 0) {
        DFErrorFormat(error,"%s",converter->warnings->data);
        ok = 0;
    }

    WordConverterFree(converter);
    return ok;
}

void WordConverterWarning(WordConverter *converter, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFBufferVFormat(converter->warnings,format,ap);
    va_end(ap);
}

char *WordStyleIdForStyle(CSSStyle *style)
{
    const char *selector = style->selector;
    char *resStyleId = NULL;

    if (!strcmp(selector,"table.Normal_Table"))
        return strdup("TableNormal");
    if (!strcmp(selector,"table.Table_Grid"))
        return strdup("TableGrid");
    if (!strcmp(selector,"span.Default_Paragraph_Font"))
        return strdup("DefaultParagraphFont");
    if (!strcmp(selector,"p.List_Paragraph"))
        return strdup("ListParagraph");

    int headingLevel = CSSSelectorHeadingLevel(selector);
    if (headingLevel != 0) {
        char *prefix = DFFormatString("heading_%d",headingLevel);
        if ((style->className != NULL) && DFStringHasPrefix(style->className,prefix)) {
            char *rest = DFSubstring(style->className,strlen(prefix),strlen(style->className));
            char *result = DFFormatString("Heading%d%s",headingLevel,rest);
            free(rest);
            free(prefix);
            return result;
        }
        free(prefix);
    }

    if (!strcmp(selector,"span.Heading1Char"))
        return strdup("Heading1Char");
    if (!strcmp(selector,"span.Heading2Char"))
        return strdup("Heading2Char");
    if (!strcmp(selector,"span.Heading3Char"))
        return strdup("Heading3Char");
    if (!strcmp(selector,"span.Heading4Char"))
        return strdup("Heading4Char");
    if (!strcmp(selector,"span.Heading5Char"))
        return strdup("Heading5Char");
    if (!strcmp(selector,"span.Heading6Char"))
        return strdup("Heading6Char");
    if (!strcmp(selector,"span.Heading7Char"))
        return strdup("Heading7Char");
    if (!strcmp(selector,"span.Heading8Char"))
        return strdup("Heading8Char");
    if (!strcmp(selector,"span.Heading9Char"))
        return strdup("Heading9Char");

    char *className = CSSSelectorCopyClassName(selector);
    switch (CSSSelectorGetTag(selector)) {
        case HTML_FIGURE: {
            resStyleId = DFStrDup("Figure");
            break;
        }
        case HTML_CAPTION: {
            resStyleId = DFStrDup("Caption");
            break;
        }
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6: {
            if ((className == NULL) || (strlen(className) == 0)) {
                int level = CSSSelectorHeadingLevel(selector);
                if ((level >= 1) && (level <= 6)) {
                    // FIXME: we shouldn't rely on the specific word "Heading" here - instead using the localised name
                    // FIXME: not covered by tests
                    resStyleId = DFFormatString("Heading%d",level);
                }
            }
            else {
                resStyleId = DFStrDup(className);
            }
            break;
        }
        case HTML_P:
            resStyleId = DFStrDup(className);
            break;
        case HTML_SPAN:
            resStyleId = DFStrDup(className);
            break;
        case HTML_TABLE:
            resStyleId = DFStrDup(className);
            break;
    }
    free(className);

    if (resStyleId == NULL) {
        // Note: selector here may start with . (i.e. applies to all elements)
        // FIXME: not covered by tests
        resStyleId = strdup(selector);
    }

    return resStyleId;
}

StyleFamily WordStyleFamilyForSelector(const char *selector)
{
    switch (CSSSelectorGetTag(selector)) {
        case HTML_FIGURE:
        case HTML_CAPTION:
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
            return StyleFamilyParagraph;
        case HTML_P: {
            char *className = CSSSelectorCopyClassName(selector);
            StyleFamily family = (className != NULL) ? StyleFamilyParagraph : StyleFamilySpecial;
            free(className);
            return family;
        }
        case HTML_SPAN:
            return StyleFamilyCharacter;
        case HTML_TABLE:
            return StyleFamilyTable;
        default:
            return StyleFamilySpecial;
    }
}

void childrenToArray(DFNode *node, DFNode **children)
{
    bzero(children,PREDEFINED_TAG_COUNT*sizeof(DFNode *));
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if ((child->tag >= MIN_ELEMENT_TAG) && (child->tag < PREDEFINED_TAG_COUNT))
            children[child->tag] = child;
    }
}

void replaceChildrenFromArray(DFNode *node, DFNode **children, Tag *tags)
{
    while (node->first != NULL)
        DFRemoveNode(node->first);

    for (int i = 0; tags[i] != 0; i++) {
        if (children[tags[i]])
            DFAppendChild(node,children[tags[i]]);
    }
}
