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
#include "DFDOM.h"
#include "DFXMLNames.h"
#include "DFHashTable.h"
#include "DFNameMap.h"
#include "DFAllocator.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           DFDocument                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void DFAssignSeqNo(DFDocument *doc, DFNode *node)
{
    node->seqNo = doc->nextSeqNo++;
    node->doc = doc;
    if (node != doc->docNode) {
        unsigned int hash = node->seqNo % SEQNO_HASH_SIZE;
        assert(node->seqNoHashNext == NULL);
        node->seqNoHashNext = doc->seqNoHashBins[hash];
        doc->seqNoHashBins[hash] = node;
    }
}

static DFNode *DocumentCreateNode(DFDocument *doc, Tag tag)
{
    if (doc->nodesCount == doc->nodesAlloc) {
        doc->nodesAlloc *= 2;
        doc->nodes = (DFNode **)realloc(doc->nodes,doc->nodesAlloc*sizeof(DFNode *));
    }

//    Node *node = NodeNew(tag);
    DFNode *node = DFAllocatorAlloc(doc->allocator,sizeof(DFNode));
    bzero(node,sizeof(DFNode));
    node->tag = tag;
    doc->nodes[doc->nodesCount++] = node;
    DFAssignSeqNo(doc,node);
    return node;
}

DFDocument *DFDocumentNew(void)
{
    DFDocument *doc = (DFDocument *)calloc(1,sizeof(DFDocument));
    doc->retainCount = 1;
    doc->allocator = DFAllocatorNew();
    doc->map = DFNameMapNew();
    doc->nodesByIdAttr = DFHashTableNew2(NULL,NULL,997);

    doc->nodesCount = 0;
    doc->nodesAlloc = 1;
    doc->nodes = (DFNode **)malloc(doc->nodesAlloc*sizeof(DFNode *));
    doc->docNode = DocumentCreateNode(doc,DOM_DOCUMENT);

    return doc;
}

DFDocument *DFDocumentNewWithRoot(Tag rootTag)
{
    DFDocument *doc = DFDocumentNew();
    doc->root = DFCreateElement(doc,rootTag);
    DFAppendChild(doc->docNode,doc->root);
    return doc;
}

static void DFClearSeqNoHash(DFDocument *doc)
{
    for (unsigned int hash = 0; hash < SEQNO_HASH_SIZE; hash++) {
        DFNode *node = doc->seqNoHashBins[hash];
        while (node != NULL) {
            DFNode *nextNode = node->seqNoHashNext;
            node->seqNoHashNext = NULL;
            node = nextNode;
        }
        doc->seqNoHashBins[hash] = NULL;
    }
}

DFDocument *DFDocumentRetain(DFDocument *doc)
{
    if (doc == NULL)
        return NULL;
    doc->retainCount++;
    return doc;
}

void DFDocumentRelease(DFDocument *doc)
{
    if (doc == NULL)
        return;
    assert(doc->retainCount > 0);
    doc->retainCount--;
    if (doc->retainCount == 0) {
        DFHashTableRelease(doc->nodesByIdAttr);
        DFClearSeqNoHash(doc);
        for (size_t i = 0; i < doc->nodesCount; i++)
            free(doc->nodes[i]->attrs);
        free(doc->nodes);
        DFNameMapFree(doc->map);
        DFAllocatorFree(doc->allocator);
        free(doc);
    }
}

DFNode *DFNodeForSeqNo(DFDocument *doc, unsigned int seqNo)
{
    unsigned int hash = seqNo % SEQNO_HASH_SIZE;
    for (DFNode *node = doc->seqNoHashBins[hash]; node != NULL; node = node->seqNoHashNext) {
        if (node->seqNo == seqNo)
            return node;
    }
    return NULL;
}

DFNode *DFElementForIdAttr(DFDocument *doc, const char *idAttr)
{
    return (DFNode *)DFHashTableLookup(doc->nodesByIdAttr,idAttr);
}

void DFDocumentReassignSeqNosRecursive(DFDocument *doc, DFNode *node)
{
    DFAssignSeqNo(doc,node);
    for (DFNode *child = node->first; child != NULL; child = child->next)
        DFDocumentReassignSeqNosRecursive(doc,child);
}

void DFDocumentReassignSeqNos(DFDocument *doc)
{
    DFClearSeqNoHash(doc);
    doc->nextSeqNo = 0;
    DFDocumentReassignSeqNosRecursive(doc,doc->docNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                               DOM                                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

char *DFCopyStringLen(DFDocument *doc, const char *str, size_t len)
{
    char *copy = (char *)DFAllocatorAlloc(doc->allocator,len+1);
    memcpy(copy,str,len);
    copy[len] = '\0';
    return copy;
}

char *DFCopyString(DFDocument *doc, const char *str)
{
    return DFCopyStringLen(doc,str,strlen(str));
}

// Document methods

DFNode *DFCreateElement(DFDocument *doc, Tag tag)
{
    DFNode *node = DocumentCreateNode(doc,tag);
    return node;
}

DFNode *DFCreateTextNode(DFDocument *doc, const char *data)
{
    DFNode *node = DocumentCreateNode(doc,DOM_TEXT);
    node->value = DFCopyString(doc,data);
    return node;
}

DFNode *DFCreateComment(DFDocument *doc, const char *data)
{
    DFNode *node = DocumentCreateNode(doc,DOM_COMMENT);
    node->value = DFCopyString(doc,data);
    return node;
}

DFNode *DFCreateCDATASection(DFDocument *doc, char *data)
{
    DFNode *node = DocumentCreateNode(doc,DOM_CDATA);
    node->value = DFCopyString(doc,data);
    return node;
}

DFNode *DFCreateProcessingInstruction(DFDocument *doc, const char *target, const char *content)
{
    DFNode *node = DocumentCreateNode(doc,DOM_PROCESSING_INSTRUCTION);
    node->target = DFCopyString(doc,target);
    node->value = DFCopyString(doc,content);
    return node;
}

// Node methods

void DFInsertBefore(DFNode *parent, DFNode *newChild, DFNode *refChild)
{
    assert(newChild->doc == parent->doc);
    assert((refChild == NULL) || (refChild->doc == parent->doc));

    if (newChild == refChild)
        return;

    if (newChild->parent != NULL)
        DFRemoveNode(newChild);

    assert(newChild->prev == NULL);
    assert(newChild->next == NULL);
    assert((refChild == NULL) || (refChild->parent == parent));
    assert(((parent->first == NULL) && (parent->last == NULL)) ||
           ((parent->first != NULL) && (parent->last != NULL)));
    assert((parent->first != NULL) || (refChild == NULL));

    newChild->prev = (refChild != NULL) ? refChild->prev : parent->last;
    newChild->next = refChild;

    if (newChild->prev != NULL)
        newChild->prev->next = newChild;
    if (newChild->next != NULL)
        newChild->next->prev = newChild;

    if (refChild == parent->first)
        parent->first = newChild;
    if (refChild == NULL)
        parent->last = newChild;

    newChild->parent = parent;
}

void DFAppendChild(DFNode *parent, DFNode *newChild)
{
    DFInsertBefore(parent,newChild,NULL);
}

DFNode *DFCreateChildElement(DFNode *parent, Tag tag)
{
    DFNode *child = DFCreateElement(parent->doc,tag);
    DFAppendChild(parent,child);
    return child;
}

DFNode *DFCreateChildTextNode(DFNode *parent, const char *data)
{
    DFNode *text = DFCreateTextNode(parent->doc,data);
    DFAppendChild(parent,text);
    return text;
}

void DFRemoveNode(DFNode *node)
{
    DFNode *parent = node->parent;
    if (parent == NULL)
        return;
    if (parent->first == node)
        parent->first = node->next;
    if (parent->last == node)
        parent->last = node->prev;
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
    node->parent = NULL;
}

void DFRemoveNodeButKeepChildren(DFNode *node)
{
    DFNode *parent = node->parent;
    assert(parent != NULL);
    while (node->first != NULL)
        DFInsertBefore(parent,node->first,node);
    DFRemoveNode(node);
}

void DFSetNodeValue(DFNode *node, const char *value)
{
    node->value = DFCopyString(node->doc,value);
}

// Element methods

const char *DFGetAttribute(DFNode *node, Tag tag)
{
    if ((node == NULL) || (node->tag < MIN_ELEMENT_TAG))
        return NULL;

    for (unsigned int i = 0; i < node->attrsCount; i++) {
        if (node->attrs[i].tag == tag)
            return node->attrs[i].value;
    }

    return NULL;
}

const char *DFGetChildAttribute(DFNode *parent, Tag childTag, Tag attrTag)
{
    DFNode *child = DFChildWithTag(parent,childTag);
    return DFGetAttribute(child,attrTag);
}

void DFSetAttribute(DFNode *element, Tag tag, const char *value)
{
    if (value == NULL) {
        DFRemoveAttribute(element,tag);
        return;
    }

    if (tag == HTML_ID) {
        const char *oldIdAttr = DFGetAttribute(element,HTML_ID);
        if (oldIdAttr != NULL)
            DFHashTableRemove(element->doc->nodesByIdAttr,oldIdAttr);
        DFHashTableAdd(element->doc->nodesByIdAttr,value,element);
    }

    // Is there an existing attribute with this tag? If so, replace it
    for (unsigned int i = 0; i < element->attrsCount; i++) {
        if (element->attrs[i].tag == tag) {
            element->attrs[i].value = DFCopyString(element->doc,value);
            return;
        }
    }

    // No existing attribute with this tag - add it
    if (element->attrsCount == element->attrsAlloc) {
        element->attrsAlloc = (element->attrsAlloc == 0) ? 8 : (2*element->attrsAlloc);
        element->attrs = (DFAttribute *)realloc(element->attrs,element->attrsAlloc*sizeof(DFAttribute));
    }

    element->attrs[element->attrsCount].tag = tag;
    element->attrs[element->attrsCount].value = DFCopyString(element->doc,value);
    element->attrsCount++;
}

void DFVFormatAttribute(DFNode *element, Tag tag, const char *format, va_list ap)
{
    char *value = DFVFormatString(format,ap);
    DFSetAttribute(element,tag,value);
    free(value);
}

void DFFormatAttribute(DFNode *element, Tag tag, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFVFormatAttribute(element,tag,format,ap);
    va_end(ap);
}

void DFRemoveAttribute(DFNode *element, Tag tag)
{
    if (tag == HTML_ID) {
        const char *oldIdAttr = DFGetAttribute(element,HTML_ID);
        if (oldIdAttr != NULL)
            DFHashTableRemove(element->doc->nodesByIdAttr,oldIdAttr);
    }

    for (unsigned int i = 0; i < element->attrsCount; i++) {
        if (element->attrs[i].tag == tag) {
            if (i+1 < element->attrsCount) {
                // Move the last attribute into this slot
                element->attrs[i].tag = element->attrs[element->attrsCount-1].tag;
                element->attrs[i].value = element->attrs[element->attrsCount-1].value;
            }
            element->attrsCount--;
            return;
        }
    }
}

void DFRemoveAllAttributes(DFNode *element)
{
    element->attrsCount = 0;
}

DFNode *DFPrevNode(DFNode *node)
{
    if (node->prev != NULL) {
        node = node->prev;
        while (node->last != NULL)
            node = node->last;
        return node;
    }
    else {
        return node->parent;
    }
}

DFNode *DFNextNodeAfter(DFNode *node)
{
    while (node != NULL) {
        if (node->next != NULL) {
            node = node->next;
            break;
        }

        node = node->parent;
    }
    return node;
}

DFNode *DFNextNode(DFNode *node)
{
    if (node->first) {
        node = node->first;
        return node;
    }
    else {
        return DFNextNodeAfter(node);
    }
}

Tag DFLookupTag(DFDocument *doc, const char *URI, const char *name)
{
    return DFNameMapTagForName(doc->map,URI,name);
}

const char *DFTagName(DFDocument *doc, Tag tag)
{
    const TagDecl *tagDecl = DFNameMapNameForTag(doc->map,tag);
    return (tagDecl != NULL) ? tagDecl->localName : NULL;
}

const char *DFTagURI(DFDocument *doc, Tag tag)
{
    const TagDecl *tagDecl = DFNameMapNameForTag(doc->map,tag);
    const NamespaceDecl *nsDecl = (tagDecl == NULL) ? NULL : DFNameMapNamespaceForID(doc->map,tagDecl->namespaceID);
    return (nsDecl != NULL) ? nsDecl->namespaceURI : NULL;
}

const char *DFNodeName(DFNode *node)
{
    if (node == NULL)
        return NULL;
    switch (node->tag) {
        case DOM_DOCUMENT:
            return "#document";
        case DOM_TEXT:
            return "#text";
        case DOM_COMMENT:
            return "#comment";
        case DOM_CDATA:
            return "#cdata-section";
        case DOM_PROCESSING_INSTRUCTION:
            return node->target;
        default:
            return DFTagName(node->doc,node->tag);
    }
}

const char *DFNodeURI(DFNode *node)
{
    return (node->tag < MIN_ELEMENT_TAG) ? NULL : DFTagURI(node->doc,node->tag);
}

void DFNodeTextToBuffer(DFNode *node, DFBuffer *buf)
{
    switch (node->tag) {
        case DOM_TEXT:
        case DOM_CDATA: {
            DFBufferAppendString(buf,node->value);
            break;
        }
    }
    for (DFNode *child = node->first; child != NULL; child = child->next)
        DFNodeTextToBuffer(child,buf);
}

char *DFNodeTextToString(DFNode *node)
{
    DFBuffer *buf = DFBufferNew();
    DFNodeTextToBuffer(node,buf);
    char *result = strdup(buf->data);
    DFBufferRelease(buf);
    return result;
}

void DFStripIds(DFNode *node)
{
    if (node->tag >= MIN_ELEMENT_TAG)
        DFRemoveAttribute(node,HTML_ID);
    for (DFNode *child = node->first; child != NULL; child = child->next)
        DFStripIds(child);
}

DFNode *DFChildWithTag(DFNode *parent, Tag tag)
{
    if ((parent == NULL) || (tag < MIN_ELEMENT_TAG))
        return NULL;
    for (DFNode *child = parent->first; child != NULL; child = child->next) {
        if (child->tag == tag)
            return child;
    }
    return NULL;
}

void DFRemoveWhitespaceNodes(DFNode *node)
{
    DFNode *next;
    for (DFNode *child = node->first; child != NULL; child = next) {
        next  = child->next;
        switch (child->tag) {
            case DOM_TEXT:
            case DOM_CDATA: {
                if (DFStringIsWhitespace(child->value))
                    DFRemoveNode(child);
                break;
            }
            case DOM_COMMENT:
                DFRemoveNode(child);
                break;
            default:
                DFRemoveWhitespaceNodes(child);
                break;
        }
    }
}

int DFIsWhitespaceNode(DFNode *node)
{
    return ((node->tag == DOM_TEXT) && DFStringIsWhitespace(node->value));
}

int identicalAttributesExcept(DFNode *first, DFNode *second, Tag except)
{
    // FIXME: except paremeter is ignored

    // This is O(n^2), but it's not really a problem as we generally only have a very small
    // number of attributes

    for (unsigned int i = 0; i < first->attrsCount; i++) {
        Tag tag = first->attrs[i].tag;
        if (tag == HTML_ID)
            continue;
        const char *firstValue = first->attrs[i].value;
        const char *secondValue = DFGetAttribute(second,tag);
        if (!DFStringEquals(firstValue,secondValue))
            return 0;
    }

    for (unsigned int i = 0; i < second->attrsCount; i++) {
        Tag tag = second->attrs[i].tag;
        if (tag == HTML_ID)
            continue;
        const char *secondValue = second->attrs[i].value;
        const char *firstValue = DFGetAttribute(first,tag);
        if (!DFStringEquals(secondValue,firstValue))
            return 0;
    }

    return 1;
}

void DFStripWhitespace(DFNode *node)
{
    if (node->tag == DOM_TEXT) {
        char *trimmed = DFStringTrimWhitespace(node->value);
        if ((strlen(trimmed) == 0) && (node->parent != NULL))
            DFRemoveNode(node);
        else
            DFSetNodeValue(node,trimmed);
        free(trimmed);
    }
    else {
        if (node->tag >= MIN_ELEMENT_TAG) {
            const char *space = DFGetAttribute(node,XML_SPACE);
            if ((space != NULL) && !strcmp(space,"preserve"))
                return;
        }
        DFNode *next;
        for (DFNode *child = node->first; child != NULL; child = next) {
            next = child->next;
            DFStripWhitespace(child);
        }
    }
}
