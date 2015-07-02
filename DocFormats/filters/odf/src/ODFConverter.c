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
#include "ODFConverter.h"
#include "DFString.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ODFSheet.h"
#include "text/ODFText.h"
#include "lenses/ODFLenses.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"

#include "text/gbg_test.h"

#include <stdio.h>


static ODFConverter *ODFConverterNew(DFDocument *html, DFStorage *abstractStorage, ODFPackage *package, const char *idPrefix)
{
    ODFConverter *converter = (ODFConverter *)xcalloc(1,sizeof(ODFConverter));
    converter->html = DFDocumentRetain(html);
    converter->abstractStorage = DFStorageRetain(abstractStorage);
    assert(DFStorageFormat(converter->abstractStorage) == DFFileFormatHTML);
    converter->idPrefix = (idPrefix != NULL) ? xstrdup(idPrefix) : xstrdup("odf");
    converter->package = ODFPackageRetain(package);
    converter->styles = ODFSheetNew(converter->package->stylesDoc, converter->package->contentDoc);
    converter->warnings = DFBufferNew();
    return converter;
}

static void ODFConverterFree(ODFConverter *converter)
{
    DFDocumentRelease(converter->html);
    DFStorageRelease(converter->abstractStorage);
    free(converter->idPrefix);
    ODFSheetRelease(converter->styles);
    DFBufferRelease(converter->warnings);
    CSSSheetRelease(converter->styleSheet);
    ODFPackageRelease(converter->package);
    free(converter);
}


int ODFConverterGet(DFDocument *html, DFStorage *abstractStorage, ODFPackage *package, const char *idPrefix, DFError **error)
{
    printf("ODFConverterGet\n");

    if (package->contentDoc == NULL) {
        DFErrorFormat(error,"document.xml not found");
        return 0;
    }
    printf("doc node %s\n", translateXMLEnumName[package->contentDoc->root->tag]);
    if (package->contentDoc->root->tag != OFFICE_DOCUMENT_CONTENT) {
        DFErrorFormat(error,"odf:document content not found");
        return 0;
    }

    DFNode *odfDocument = package->contentDoc->root;

    ODFConverter *converter = ODFConverterNew(html,abstractStorage,package,idPrefix);

    //Get the styles data
    //CSSSheetRelease(converter->styleSheet);
    converter->styleSheet = ODFStylesGet(converter);

    //Try a lenses approach
    ODFGetData get;
    get.conv = converter;
    DFNode *abstract = ODFDocumentLens.get(&get,odfDocument);
    DFAppendChild(converter->html->docNode,abstract);
    converter->html->root = abstract;
    //ODF_postProcessHTMLDoc(converter);

    //Convert the content.xml to an html beastie
    //ODFTextGet(converter);

    char *cssText = CSSSheetCopyCSSText(converter->styleSheet);
    HTMLAddInternalStyleSheet(converter->html, cssText);
    HTML_safeIndent(converter->html->docNode,0);

    int ok = 1;
    if (converter->warnings->len > 0) {
        DFErrorFormat(error,"%s",converter->warnings->data);
        ok = 0;
    }

    ODFConverterFree(converter);
    return ok;
}

DFNode *ODFConverterCreateAbstract(ODFGetData *get, Tag tag, DFNode *concrete)
{
    DFNode *element = DFCreateElement(get->conv->html,tag);

   if (concrete != NULL) {
        char *idStr;
        if (concrete->doc == get->conv->package->contentDoc)
            idStr = DFFormatString("%s%u",get->conv->idPrefix,concrete->seqNo);
        else
            idStr = DFFormatString("%s%u-%s",get->conv->idPrefix,concrete->seqNo,DFNodeName(concrete->doc->root));
        DFSetAttribute(element,HTML_ID,idStr);
        free(idStr);
    }
    return element;
}

DFNode *ODFConverterGetConcrete(ODFPutData *put, DFNode *abstract)
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

/*    DFDocument *doc = NULL;
    if (docName == NULL)
        doc = put->conv->package->document;
    else if (!strcmp(docName,"footnotes"))
        doc = put->conv->package->footnotes;
    else if (!strcmp(docName,"endnotes"))
        doc = put->conv->package->endnotes;
    else
        return NULL; */

    // Check to see if we have a node in the concrete document matching that sequence number
    DFNode *node = DFNodeForSeqNo(put->conv->package->contentDoc,seqNo);

    // Only return the node if it's actually an element
    if ((node == NULL) || (node->tag < MIN_ELEMENT_TAG))
        return NULL;
    return node;
}

