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
#include "DFXML.h"
#include "DFDOM.h"
#include "DFMarkupCompatibility.h"
#include "DFHTML.h"
#include "DFBuffer.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <string.h>

const xmlChar *INDENT =
(const xmlChar *)"\n                                                                                "\
"                                                                                "\
"                                                                                "\
"                                                                                ";

static int HTML_requiresCloseTag(Tag tag)
{
    // FIXME: Check for any other such tags
    switch (tag) {
        case HTML_IMG:
        case HTML_BR:
        case HTML_META:
        case HTML_LINK:
        case HTML_HR:
        case HTML_COL:
            return 0;
        default:
            return 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           DFSAXParser                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void DFSAXSetup(xmlSAXHandler *handler);

typedef struct DFSAXParser DFSAXParser;

struct DFSAXParser {
    DFDocument *document;
    DFBuffer *warnings;
    DFBuffer *errors;
    DFBuffer *fatalErrors;
    DFMarkupCompatibility *compatibility;
    unsigned int ignoreDepth;

    DFNode *parent; // not explicitly retained
};

DFSAXParser *DFSAXParserNew(void)
{
    DFSAXParser *parser = (DFSAXParser *)calloc(1,sizeof(DFSAXParser));
    parser->document = DFDocumentNew();
    parser->parent = parser->document->docNode;
    parser->warnings = DFBufferNew();
    parser->errors = DFBufferNew();
    parser->fatalErrors = DFBufferNew();
    parser->compatibility = DFMarkupCompatibilityNew();
    return parser;
}

void DFSAXParserFree(DFSAXParser *parser)
{
    DFDocumentRelease(parser->document);
    DFBufferRelease(parser->warnings);
    DFBufferRelease(parser->errors);
    DFBufferRelease(parser->fatalErrors);
    DFMarkupCompatibilityFree(parser->compatibility);
    free(parser);
}

void DFSAXParserParse(DFSAXParser *parser, const void *data, size_t len)
{
    xmlSAXHandler handler;
    DFSAXSetup(&handler);
    xmlSAXUserParseMemory(&handler,parser,data,(int)len);
}

static void SAXStartElementNS(void *ctx, const xmlChar *localname,
                              const xmlChar *prefix, const xmlChar *URI,
                              int nb_namespaces, const xmlChar **namespaces,
                              int nb_attributes, int nb_defaulted, const xmlChar **attributes)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;

    if (parser->ignoreDepth > 0) {
        parser->ignoreDepth++;
        return;
    }

    for (int i = 0; i < nb_namespaces; i++) {
        const xmlChar *nsPrefix = namespaces[i*2];
        const xmlChar *nsURI = namespaces[i*2+1];
        DFNameMapFoundNamespace(parser->document->map,(const char *)nsURI,(const char *)nsPrefix);
    }

    Tag tag = DFNameMapTagForName(parser->document->map,(const char *)URI,(const char *)localname);

    if (parser->compatibility != NULL) {
        const TagDecl *tagDecl = DFNameMapNameForTag(parser->document->map,tag);
        MCAction action = DFMarkupCompatibilityLookup(parser->compatibility,tagDecl->namespaceID,tag,1);
        if (action == MCActionIgnore) {
            parser->ignoreDepth++;
            return;
        }
    }

    if (parser->compatibility != NULL) {
        DFMarkupCompatibilityPush(parser->compatibility,nb_namespaces,(const char **)namespaces,parser->document->map);
    }

    DFNode *element = DFCreateElement(parser->document,tag);
    for (int i = 0; i < nb_attributes; i++) {
        const xmlChar *attrLocalName = attributes[i*5+0];
        const xmlChar *attrURI = attributes[i*5+2];
        const xmlChar *attrValueStart = attributes[i*5+3];
        const xmlChar *attrValueEnd = attributes[i*5+4];
        unsigned long attrValueLen = attrValueEnd - attrValueStart;

        Tag attrTag = DFNameMapTagForName(parser->document->map,(const char *)attrURI,(const char *)attrLocalName);
        const TagDecl *attrTagDecl = DFNameMapNameForTag(parser->document->map,attrTag);
        char *attrValue = (char *)malloc(attrValueLen+1);
        memcpy(attrValue,attrValueStart,attrValueLen);
        attrValue[attrValueLen] = '\0';
        if (parser->compatibility != NULL) {
            switch (attrTag) {
                case MC_IGNORABLE:
                case MC_PROCESSCONTENT:
                case MC_MUSTUNDERSTAND:
                    DFMarkupCompatibilityProcessAttr(parser->compatibility,attrTag,attrValue,parser->document->map);
                    break;
                default: {
                    MCAction action = DFMarkupCompatibilityLookup(parser->compatibility,attrTagDecl->namespaceID,0,0);
                    if (action != MCActionIgnore)
                        DFSetAttribute(element,attrTag,attrValue);
                    break;
                }
            }
        }
        else {
            DFSetAttribute(element,attrTag,attrValue);
        }
        free(attrValue);
    }

    DFAppendChild(parser->parent,element);
    parser->parent = element;
    if (parser->document->root == NULL)
        parser->document->root = element;
}

static void SAXEndElementNS(void *ctx, const xmlChar *localname,
                            const xmlChar *prefix, const xmlChar *URI)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;

    if (parser->ignoreDepth > 0) {
        parser->ignoreDepth--;
        return;
    }

    if (parser->compatibility != NULL)
        DFMarkupCompatibilityPop(parser->compatibility);

    assert(parser->parent != NULL);
    parser->parent = parser->parent->parent;
    assert(parser->parent != NULL);
}

// SAXStartElement and SAXEndElement are used only when parsing HTML

static void SAXStartElement(void *ctx, const xmlChar *fullname, const xmlChar **atts)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    const NamespaceDecl *namespaceDecl = DFNameMapNamespaceForID(parser->document->map,NAMESPACE_HTML);
    Tag tag = DFNameMapTagForName(parser->document->map,namespaceDecl->namespaceURI,(const char *)fullname);
    DFNode *element = DFCreateElement(parser->document,tag);
    if (atts != NULL) {
        for (int i = 0; atts[i] != NULL; i += 2) {
            const xmlChar *name = atts[i];
            const xmlChar *value = atts[i+1];
            Tag attrTag = DFNameMapTagForName(parser->document->map,namespaceDecl->namespaceURI,(const char *)name);
            DFSetAttribute(element,attrTag,(const char *)value);
        }
    }
    DFAppendChild(parser->parent,element);
    parser->parent = element;
    if (parser->document->root == NULL)
        parser->document->root = element;
}

static void SAXEndElement(void *ctx, const xmlChar *name)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    assert(parser->parent != NULL);
    parser->parent = parser->parent->parent;
    assert(parser->parent != NULL);
}

static void SAXCharacters(void *ctx, const xmlChar *ch, int len)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    if (parser->ignoreDepth > 0)
        return;
    char *data = (char *)malloc(len+1);
    memcpy(data,ch,len);
    data[len] = '\0';
    DFNode *text = DFCreateTextNode(parser->document,data);
    assert(parser->parent != NULL);
    DFAppendChild(parser->parent,text);
    free(data);
}

static void SAXComment(void *ctx, const xmlChar *value)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    if (parser->ignoreDepth > 0)
        return;;
    DFNode *comment = DFCreateComment(parser->document,(const char *)value);
    assert(parser->parent != NULL);
    DFAppendChild(parser->parent,comment);
}

static void SAXCDATABlock(void *ctx, const xmlChar *value, int len)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    if (parser->ignoreDepth > 0)
        return;
    char *data = (char *)malloc(len+1);
    memcpy(data,value,len);
    data[len] = '\0';
    DFNode *cdata = DFCreateTextNode(parser->document,data);
    assert(parser->parent != NULL);
    DFAppendChild(parser->parent,cdata);
    free(data);
}

static void SAXProcessingInstruction(void *ctx, const xmlChar *target, const xmlChar *data)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    if (parser->ignoreDepth > 0)
        return;;
    DFNode *pi = DFCreateProcessingInstruction(parser->document,(const char *)target,(const char *)data);
    assert(parser->parent != NULL);
    DFAppendChild(parser->parent,pi);
}

static void SAXWarning(void *ctx, const char *msg, ...)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    va_list ap;
    va_start(ap,msg);
    DFBufferFormat(parser->warnings,msg,ap);
    va_end(ap);
}

static void SAXError(void *ctx, const char *msg, ...)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    va_list ap;
    va_start(ap,msg);
    DFBufferFormat(parser->errors,msg,ap);
    va_end(ap);
}

static void SAXFatalError(void *ctx, const char *msg, ...)
{
    DFSAXParser *parser = (DFSAXParser *)ctx;
    va_list ap;
    va_start(ap,msg);
    DFBufferFormat(parser->fatalErrors,msg,ap);
    va_end(ap);
}

static void DFSAXSetup(xmlSAXHandler *handler)
{
    bzero(handler,sizeof(xmlSAXHandler));
    handler->characters = SAXCharacters;
    handler->processingInstruction = SAXProcessingInstruction;
    handler->comment = SAXComment;
    handler->cdataBlock = SAXCDATABlock;
    handler->warning = SAXWarning;
    handler->error = SAXError;
    handler->error = SAXFatalError;
    handler->startElementNs = SAXStartElementNS;
    handler->endElementNs = SAXEndElementNS;
    handler->startElement = SAXStartElement;
    handler->endElement = SAXEndElement;
    handler->initialized = XML_SAX2_MAGIC;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                              DFXML                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void htmlSAXParseDoc(xmlChar * cur, const char * encoding, xmlSAXHandlerPtr sax, void * userData);

typedef struct {
    xmlTextWriterPtr writer;
    DFDocument *doc;
    NamespaceID defaultNS;
    int html;
    int indent;
} Serialization;

static void writeNode(Serialization *serialization, DFNode *node, int depth);

static void findUsedNamespaces(DFDocument *doc, DFNode *node, char *used, NamespaceID count)
{
    if (node->tag < MIN_ELEMENT_TAG)
        return;;
    const TagDecl *tagDecl = DFNameMapNameForTag(doc->map,node->tag);
    assert(tagDecl != NULL);
    assert(tagDecl->namespaceID < count);
    used[tagDecl->namespaceID] = 1;

    // Attributes
    if (node->tag >= MIN_ELEMENT_TAG) {
        for (unsigned int i = 0; i < node->attrsCount; i++) {
            Tag tag = node->attrs[i].tag;
            const TagDecl *attrDecl = DFNameMapNameForTag(doc->map,tag);
            assert(attrDecl != NULL);
            if ((attrDecl->namespaceID != NAMESPACE_NULL) &&
                (attrDecl->namespaceID != NAMESPACE_XML))
                used[attrDecl->namespaceID] = 1;
        }
    }

    // Children
    for (DFNode *child = node->first; child != NULL; child = child->next)
        findUsedNamespaces(doc,child,used,count);
}

static void writeNamespaceDeclarations(Serialization *serialization, DFNode *node)
{
    NamespaceID count = DFNameMapNamespaceCount(serialization->doc->map);
    char *used = (char *)calloc(1,count);
    findUsedNamespaces(serialization->doc,node,used,count);
    for (NamespaceID nsId = 1; nsId < count; nsId++) { // don't write null namespace
        if (used[nsId]) {
            const NamespaceDecl *nsDecl = DFNameMapNamespaceForID(serialization->doc->map,nsId);
            const xmlChar *prefix = (const xmlChar *)nsDecl->prefix;
            const xmlChar *URI = (const xmlChar *)nsDecl->namespaceURI;
            if (nsId == serialization->defaultNS)
                xmlTextWriterWriteAttribute(serialization->writer,(const xmlChar *)"xmlns",URI);
            else
                xmlTextWriterWriteAttributeNS(serialization->writer,(const xmlChar *)"xmlns",prefix,NULL,URI);
        }
    }
    free(used);
}

static int compareAttrs(const void *a, const void *b)
{
    const DFAttribute *attrA = (const DFAttribute *)a;
    const DFAttribute *attrB = (const DFAttribute *)b;
    if (attrA->tag < attrB->tag)
        return -1;
    else if (attrA->tag > attrB->tag)
        return 1;
    else
        return 0;
}

static void writeAttributes(Serialization *serialization, DFNode *element)
{
    // Sort the keys by their tag, to ensure that we always write attributes out in the same order.
    // This is important for automated tests which rely on consistent output for a given XML tree.
    DFAttribute *attrs = (DFAttribute *)malloc(element->attrsCount*sizeof(DFAttribute));
    memcpy(attrs,element->attrs,element->attrsCount*sizeof(DFAttribute));
    qsort(attrs,element->attrsCount,sizeof(DFAttribute),compareAttrs);

    for (unsigned int i = 0; i < element->attrsCount; i++) {
        Tag tag = attrs[i].tag;

        const TagDecl *tagDecl = DFNameMapNameForTag(serialization->doc->map,tag);
        assert(tagDecl != NULL);
        const NamespaceDecl *nsDecl = DFNameMapNamespaceForID(serialization->doc->map,tagDecl->namespaceID);
        assert(nsDecl != NULL);

        const xmlChar *prefix = (const xmlChar *)nsDecl->prefix;
        const xmlChar *localName = (const xmlChar *)tagDecl->localName;
        const xmlChar *value = (const xmlChar *)attrs[i].value;

        if (serialization->html && (tagDecl->namespaceID == NAMESPACE_HTML))
            xmlTextWriterWriteAttribute(serialization->writer,localName,value);
        else
            xmlTextWriterWriteAttributeNS(serialization->writer,prefix,localName,NULL,value);
    }
    free(attrs);
}

static void writeElement(Serialization *serialization, DFNode *element, int depth)
{
    const TagDecl *tagDecl = DFNameMapNameForTag(serialization->doc->map,element->tag);
    assert(tagDecl != NULL);
    const NamespaceDecl *nsDecl = DFNameMapNamespaceForID(serialization->doc->map,tagDecl->namespaceID);
    assert(nsDecl != NULL);

    const xmlChar *prefix = (const xmlChar *)nsDecl->prefix;
    const xmlChar *localName = (const xmlChar *)tagDecl->localName;

    if (serialization->indent && (element->parent != element->doc->docNode))
        xmlTextWriterWriteRawLen(serialization->writer,INDENT,1+depth);

    if (serialization->html || (tagDecl->namespaceID == serialization->defaultNS))
        xmlTextWriterStartElement(serialization->writer,localName);
    else
        xmlTextWriterStartElementNS(serialization->writer,prefix,localName,NULL);

    if ((element->parent == serialization->doc->docNode) && !serialization->html)
        writeNamespaceDeclarations(serialization,element);

    writeAttributes(serialization,element);

    // Check if all children are text nodes. If this is true; we should treat them as if they are a single text
    // node, and not do any indentation.
    int allChildrenText = 1;
    for (DFNode *child = element->first; child != NULL; child = child->next) {
        if (child->tag != DOM_TEXT)
            allChildrenText = 0;
    }

    if (allChildrenText) {
        int oldIndent = serialization->indent;
        serialization->indent = 0;
        for (DFNode *child = element->first; child != NULL; child = child->next)
            writeNode(serialization,child,depth+2);
        serialization->indent = oldIndent;
    }
    else {
        for (DFNode *child = element->first; child != NULL; child = child->next)
            writeNode(serialization,child,depth+2);
    }

    if (serialization->indent && (element->first != NULL) && !allChildrenText) {
        if ((element->first != element->last) ||
            (element->first->tag != DOM_TEXT))
        xmlTextWriterWriteRawLen(serialization->writer,INDENT,1+depth);
    }

    if (serialization->html && (element->first == NULL) && HTML_requiresCloseTag(element->tag)) {
        xmlTextWriterWriteString(serialization->writer,(xmlChar *)"");
    }

    xmlTextWriterEndElement(serialization->writer);
}

static void writeNode(Serialization *serialization, DFNode *node, int depth)
{
    switch (node->tag) {
        case DOM_DOCUMENT: {
            if (!serialization->html)
                xmlTextWriterStartDocument(serialization->writer,"1.0","UTF-8","yes");
            if (serialization->html)
                xmlTextWriterWriteDTD(serialization->writer,(xmlChar *)"html",NULL,NULL,NULL);
//            xmlTextWriterWriteDTD(writer,
//                                  (xmlChar *)"html",
//                                  (xmlChar *)"-//W3C//DTD XHTML 1.0 Strict//EN",
//                                  (xmlChar *)"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd",
//                                  NULL);
            for (DFNode *child = node->first; child != NULL; child = child->next)
                writeNode(serialization,child,0);
            xmlTextWriterEndDocument(serialization->writer);
            break;
        }
        case DOM_TEXT: {
            if (serialization->indent && ((node->prev != NULL) || (node->next != NULL)))
                xmlTextWriterWriteRawLen(serialization->writer,INDENT,1+depth);
            if (serialization->html && (node->parent != NULL) && (node->parent->tag == HTML_STYLE)) {
                xmlTextWriterWriteRaw(serialization->writer,(const xmlChar *)node->value);
            }
            else {
                xmlTextWriterWriteString(serialization->writer,(const xmlChar *)node->value);
            }
            break;
        }
        case DOM_COMMENT: {
            xmlTextWriterWriteComment(serialization->writer,(const xmlChar *)node->value);
            break;
        }
        case DOM_CDATA: {
            xmlTextWriterWriteCDATA(serialization->writer,(const xmlChar *)node->value);
            break;
        }
        case DOM_PROCESSING_INSTRUCTION: {
            xmlTextWriterWritePI(serialization->writer,
                                 (const xmlChar *)node->target,
                                 (const xmlChar *)node->value);
            break;
        }
        default: {
            if (node->parent == serialization->doc->docNode)
                writeElement(serialization,node,0);
            else
                writeElement(serialization,node,depth);
            break;
        }
    }
}

DFDocument *DFParseXMLString(const char *str, DFError **error)
{
    DFSAXParser *parser = DFSAXParserNew();
    DFSAXParserParse(parser,str,strlen(str));
    if (parser->fatalErrors->len > 0) {
        DFErrorFormat(error,"%s",parser->fatalErrors->data);
        DFSAXParserFree(parser);
        return NULL;
    }
    else if (parser->errors->len > 0) {
        DFErrorFormat(error,"%s",parser->errors->data);
        DFSAXParserFree(parser);
        return NULL;
    }
    else if (parser->document->root == NULL) {
        DFErrorFormat(error,"No root element");
        DFSAXParserFree(parser);
        return NULL;
    }

    DFDocument *result = DFDocumentRetain(parser->document);
    DFSAXParserFree(parser);
    return result;
}

DFDocument *DFParseXMLFile(const char *filename, DFError **error)
{
    DFBuffer *buf = DFBufferReadFromFile(filename,error);
    if (buf == NULL)
        return NULL;;
    DFDocument *doc = DFParseXMLString(buf->data,error);
    DFBufferRelease(buf);
    return doc;
}

DFDocument *DFParseXMLStorage(DFStorage *storage, const char *filename, DFError **error)
{
    DFBuffer *content = DFBufferReadFromStorage(storage,filename,error);
    if (content == NULL)
        return NULL;;
    DFDocument *doc = DFParseXMLString(content->data,error);
    DFBufferRelease(content);
    return doc;
}

static int StringBufferWrite(void *context, const char *buffer, int len)
{
    DFBuffer *buf = (DFBuffer *)context;
    DFBufferAppendData(buf,buffer,len);
    return len;
}

static int StringBufferClose(void *context)
{
    return 0;
}

void DFSerializeXMLBuffer(DFDocument *doc, NamespaceID defaultNS, int indent, DFBuffer *buf)
{
    xmlOutputBufferPtr output = xmlOutputBufferCreateIO(StringBufferWrite,
                                                        StringBufferClose,
                                                        buf,
                                                        NULL);
    xmlTextWriterPtr writer = xmlNewTextWriter(output);

    int html = 0;
    for (DFNode *child = doc->docNode->first; child != NULL; child = child->next) {
        if (child->tag == HTML_HTML)
            html = 1;
    }

    Serialization serialization;
    bzero(&serialization,sizeof(serialization));
    serialization.writer = writer;
    serialization.doc = doc;
    serialization.defaultNS = defaultNS;
    serialization.html = html;
    serialization.indent = indent;
    writeNode(&serialization,doc->docNode,0);
    xmlFreeTextWriter(writer);
}

char *DFSerializeXMLString(DFDocument *doc, NamespaceID defaultNS, int indent)
{
    DFBuffer *buf = DFBufferNew();
    DFSerializeXMLBuffer(doc,defaultNS,indent,buf);
    char *result = strdup(buf->data);
    DFBufferRelease(buf);
    return result;
}

int DFSerializeXMLFile(DFDocument *doc, NamespaceID defaultNS, int indent, const char *filename, DFError **error)
{
    DFBuffer *buf = DFBufferNew();
    DFSerializeXMLBuffer(doc,defaultNS,indent,buf);
    int r = DFBufferWriteToFile(buf,filename,error);
    DFBufferRelease(buf);
    return r;
}

int DFSerializeXMLStorage(DFDocument *doc, NamespaceID defaultNS, int indent,
                          DFStorage *storage, const char *filename,
                          DFError **error)
{
    char *str = DFSerializeXMLString(doc,defaultNS,indent);
    DFBuffer *content = DFBufferNew();
    DFBufferAppendString(content,str);
    int r = DFBufferWriteToStorage(content,storage,filename,error);
    DFBufferRelease(content);
    free(str);
    return r;
}
