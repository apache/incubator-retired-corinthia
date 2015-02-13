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
#include "DFHTML.h"
#include "DFFilesystem.h"
#include "DFDOM.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "CSSSelector.h"
#include "CSSLength.h"
#include "CSSSheet.h"
#include "DFHTDocument.h"
#include "DFTidyHelper.h"
#include "DFNameMap.h"
#include "DFXML.h"
#include "DFString.h"
#include "DFCharacterSet.h"
#include "DFCommon.h"

static DFNode *HTML_findHead(DFDocument *doc)
{
    for (DFNode *child1 = doc->docNode->first; child1 != NULL; child1 = child1->next) {
        if (child1->tag != HTML_HTML)
            return NULL;
        for (DFNode *child2 = child1->first; child2 != NULL; child2 = child2->next) {
            if (child2->tag == HTML_HEAD) {
                return child2;
            }
        }
    }
    return NULL;
}

void HTMLAddExternalStyleSheet(DFDocument *doc, const char *href)
{
    DFNode *head = HTML_findHead(doc);
    if (head == NULL)
        return;;
    DFNode *link = DFCreateElement(doc,HTML_LINK);
    DFSetAttribute(link,HTML_REL,"stylesheet");
    DFSetAttribute(link,HTML_HREF,href);
    DFAppendChild(head,link);
}

void HTMLAddInternalStyleSheet(DFDocument *doc, const char *cssText)
{
    DFNode *head = HTML_findHead(doc);
    if (head == NULL)
        return;;
    DFNode *style = DFCreateElement(doc,HTML_STYLE);
    DFAppendChild(style,DFCreateTextNode(doc,cssText));
    DFAppendChild(head,style);
}

char *HTMLCopyCSSText(DFDocument *doc)
{
    DFNode *head = DFChildWithTag(doc->root,HTML_HEAD);
    if (head == NULL)
        return strdup("");;
    DFNode *style = DFChildWithTag(head,HTML_STYLE);
    if (style == NULL)
        return strdup("");
    return DFNodeTextToString(style);
}

int HTML_isContainerTag(Tag tag)
{
    switch (tag) {
        case HTML_ARTICLE:
        case HTML_SECTION:
        case HTML_NAV:
        case HTML_ASIDE:
        case HTML_HGROUP:
        case HTML_HEADER:
        case HTML_FOOTER:
        case HTML_ADDRESS:
        case HTML_DIV:
            return 1;
        default:
            return 0;
    }
}

int HTML_isBlockLevelTag(Tag tag)
{
    switch (tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_UL:
        case HTML_OL:
        case HTML_LI:
        case HTML_TABLE:
        case HTML_CAPTION:
        case HTML_TR:
        case HTML_TD:
        case HTML_TH:
        case HTML_THEAD:
        case HTML_TBODY:
        case HTML_TFOOT:
        case HTML_FIGURE:
        case HTML_FIGCAPTION:
            return 1;
        default:
            return HTML_isContainerTag(tag);
    }
}

int HTML_isParagraphTag(Tag tag)
{
    switch (tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
            return 1;
        default:
            return 0;
    }
}

int HTML_isListTag(Tag tag)
{
    switch (tag) {
        case HTML_UL:
        case HTML_OL:
            return 1;
        default:
            return 0;
    }
}

int HTML_isSpecialSpan(DFNode *node)
{
    if (node->tag != HTML_SPAN)
        return 0;
    return DFStringHasPrefix(DFGetAttribute(node,HTML_CLASS),"uxwrite-");
}

int HTML_isContentNode(DFNode *node)
{
    switch (node->tag) {
        case DOM_TEXT:
            return 1;
        case HTML_SPAN:
            return HTML_isSpecialSpan(node);
        case HTML_IMG:
            return 1;
        case HTML_BR:
            return 1;
        default:
            return 0;
    }
}

int HTML_nodeHasContent(DFNode *node)
{
    if (DFIsWhitespaceNode(node))
        return 0;
    if (HTML_isContentNode(node))
        return 1;
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if (HTML_nodeHasContent(child))
            return 1;
    }
    return 0;
}

int HTML_nodeIsHyperlink(DFNode *node)
{
    const char *href = DFGetAttribute(node,HTML_HREF);
    return ((node->tag == HTML_A) && (href != NULL) && !DFStringHasPrefix(href,"#"));
}

static char *indentString(int depth)
{
    char *str = (char *)malloc(1+depth*2+1);
    str[0] = '\n';
    memset(&str[1],' ',depth*2);
    str[1+depth*2] = '\0';
    return str;
}

static DFNode *createIndentNode(DFDocument *doc, int depth)
{
    char *str = indentString(depth);
    DFNode *node = DFCreateTextNode(doc,str);
    free(str);
    return node;
}

void HTML_safeIndent(DFNode *node, int depth)
{
    switch (node->tag) {
        case DOM_DOCUMENT:
        case HTML_HTML:
        case HTML_HEAD:
        case HTML_BODY:
        case HTML_DIV:
        case HTML_UL:
        case HTML_OL:
        case HTML_LI:
        case HTML_NAV:
        case HTML_FIGURE:
        case HTML_TABLE:
        case HTML_COLGROUP:
        case HTML_THEAD:
        case HTML_TBODY:
        case HTML_TFOOT:
        case HTML_TR:
        case HTML_TH:
        case HTML_TD: {
            DFNode *next;
            for (DFNode *child = node->first; child != NULL; child = next) {
                next = child->next;
                DFNode *text = createIndentNode(node->doc,depth);
                DFInsertBefore(node,text,child);
            }
            if (node->tag != DOM_DOCUMENT) {
                DFNode *text = createIndentNode(node->doc,depth-1);
                DFAppendChild(node,text);
            }
            break;
        }
        case HTML_SCRIPT:
        case HTML_STYLE: {
            char *nodeContent = DFNodeTextToString(node);
            char *trimmedContent = DFStringTrimWhitespace(nodeContent);
            while (node->first != NULL)
                DFRemoveNode(node->first);
            char *indentStr = indentString(depth-1);
            char *revisedStr = DFFormatString("\n%s%s",trimmedContent,indentStr);
            DFAppendChild(node,DFCreateTextNode(node->doc,revisedStr));
            free(indentStr);
            free(revisedStr);
            free(nodeContent);
            free(trimmedContent);
            break;
        }
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_FIGCAPTION:
        case HTML_CAPTION: {
            if (node->first != NULL) {
                char *indentBefore = indentString(depth);
                char *indentAfter = indentString(depth-1);
                DFNode *before = DFCreateTextNode(node->doc,indentBefore);
                DFNode *after = DFCreateTextNode(node->doc,indentAfter);
                DFInsertBefore(node,before,node->first);
                DFAppendChild(node,after);
                free(indentBefore);
                free(indentAfter);
            }
            break;
        }
    }
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next = child->next;
        HTML_safeIndent(child,depth+1);
    }
}

void HTMLBreakBDTRefs(DFNode *node, const char *idPrefix)
{
    if (node->tag >= MIN_ELEMENT_TAG) {
        const char *seqNo = DFGetAttribute(node,HTML_ID);
        if ((seqNo != NULL) && DFStringHasPrefix(seqNo,idPrefix)) {
            char *newNodeId = DFFormatString("x%s",&seqNo[strlen(idPrefix)]);
            DFSetAttribute(node,HTML_ID,newNodeId);
            free(newNodeId);
        }
    }
    for (DFNode *child = node->first; child != NULL; child = child->next)
        HTMLBreakBDTRefs(child,idPrefix);
}

CSSSize HTML_getImageDimensions(DFNode *img)
{
    CSSSize dimensions;
    bzero(&dimensions,sizeof(CSSSize));

    const char *styleAttr = DFGetAttribute(img,HTML_STYLE);
    if (styleAttr != NULL) {
        CSSProperties *properties = CSSPropertiesNewWithString(styleAttr);
        if (CSSGet(properties,"width") != NULL)
            dimensions.width = CSSLengthFromString(CSSGet(properties,"width"));
        if (CSSGet(properties,"height") != NULL)
            dimensions.height = CSSLengthFromString(CSSGet(properties,"height"));
        CSSPropertiesRelease(properties);
    }

    if (!CSSLengthIsValid(dimensions.width)) {
        const char *widthAttr = DFGetAttribute(img,HTML_WIDTH);
        if (widthAttr != NULL) {
            if (DFStringHasSuffix(widthAttr,"%"))
                dimensions.width = CSSLengthFromValue(atof(widthAttr),UnitsPct);
            else
                dimensions.width = CSSLengthFromValue(atof(widthAttr),UnitsPx);
        }
    }

    if (CSSLengthIsValid(dimensions.height)) {
        const char *heightAttr = DFGetAttribute(img,HTML_HEIGHT);
        if (heightAttr != NULL) {
            if (DFStringHasSuffix(heightAttr,"%"))
                dimensions.height = CSSLengthFromValue(atof(heightAttr),UnitsPct);
            else
                dimensions.height = CSSLengthFromValue(atof(heightAttr),UnitsPx);
        }
    }

    return dimensions;
}

int isRRGGBB(const char *str)
{
    if (str == NULL)
        return 0;;

    size_t len = strlen(str);
    if (len != 6)
        return 0;

    for (size_t i = 0; i < len; i++) {
        if (!DFCharIsHex(str[i]))
            return 0;
    }

    return 1;
}

int isHashRRGGBB(const char *str)
{
    return (strlen(str) == 7) && (str[0] == '#') && isRRGGBB(&str[1]);
}

DFDocument *DFParseHTMLString(const char *str, int removeSpecial, DFError **error)
{
    DFHTDocument *htdoc = DFHTDocumentNew();
//    printf("tidy option nl = %d\n",tidyOptGetInt(htdoc.doc,TidyNewline));
//    tidyOptSetInt(htdoc.doc,TidyNewline,1);
    if (!DFHTDocumentParseCString(htdoc,str,error)) {
        DFHTDocumentFree(htdoc);
        return NULL;
    }
    if (removeSpecial)
        DFHTDocumentRemoveUXWriteSpecial(htdoc);;
    DFDocument *doc = DFDocumentNew();
    DFNode *root = fromTidyNode(doc,htdoc->doc,tidyGetHtml(htdoc->doc));
    if (root == NULL) {
        DFErrorFormat(error,"No root element");
        DFDocumentRelease(doc);
        DFHTDocumentFree(htdoc);
        return NULL;
    }
    DFAppendChild(doc->docNode,root);
    doc->root = root;

    // Remove empty title element that is automatically added by HTMLTidy's parser, since a lot of
    // tests currently depend on it not being present
    DFNode *head = DFChildWithTag(doc->root,HTML_HEAD);
    if (head != NULL) {
        DFNode *title = DFChildWithTag(head,HTML_TITLE);
        if (title != NULL) {
            char *titleText = DFNodeTextToString(title);
            if (strlen(titleText) == 0)
                DFRemoveNode(title);
            free(titleText);
        }
    }

    DFHTDocumentFree(htdoc);
    return doc;
}

DFDocument *DFParseHTMLFile(const char *filename, int removeSpecial, DFError **error)
{
    DFBuffer *buf = DFBufferReadFromFile(filename,error);
    if (buf == NULL)
        return NULL;;
    DFDocument *doc = DFParseHTMLString(buf->data,removeSpecial,error);
    DFBufferRelease(buf);
    return doc;
}

static void DFHTMLGetImagesRecursive(DFNode *node, DFHashTable *images)
{
    if (node->tag == HTML_IMG) {
        const char *src = DFGetAttribute(node,HTML_SRC);
        if (src != NULL) {
            char *decoded = DFRemovePercentEncoding(src);
            DFHashTableAdd(images,decoded,"");
            free(decoded);
        }
    }
    else {
        for (DFNode *child = node->first; child != NULL; child = child->next)
            DFHTMLGetImagesRecursive(child,images);
    }
}

const char **DFHTMLGetImages(DFDocument *htmlDoc)
{
    DFHashTable *images = DFHashTableNew(NULL,NULL);
    DFHTMLGetImagesRecursive(htmlDoc->root,images);
    const char **filenames = DFHashTableCopyKeys(images);
    DFSortStringsCaseInsensitive(filenames);
    DFHashTableRelease(images);
    return filenames;
}
