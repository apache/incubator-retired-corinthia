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
#include "WordPackage.h"
#include "WordStyles.h"
#include "WordSettings.h"
#include "DFDOM.h"
#include "DFFilesystem.h"
#include "DFXML.h"
#include "OPC.h"
#include "DFHTML.h"
#include "WordField.h"
#include "WordBookmark.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static DFDocument *parsePart(WordPackage *package, OPCPart *part, DFError **error);

// RSIDs are used to associated particular pargraphs, runs, and other components of the document
// with distinct editing sessions to facilitate merging. Keeping them up to date in an appropriate
// manner would for us be more work than necessary, and there's better ways to do merging anyway
// (see: any version control system). It's safe to remove these from the document entirely.
// See: http://blogs.msdn.com/b/brian_jones/archive/2006/12/11/what-s-up-with-all-those-rsids.aspx

static void Word_stripRSIDsRecursive(DFNode *node)
{
    if (node->tag >= MIN_ELEMENT_TAG) {
        Tag remove[20];
        unsigned int removeCount = 0;

        for (unsigned int i = 0; i < node->attrsCount; i++) {
            switch (node->attrs[i].tag) {
                case WORD_RSID:
                case WORD_RSIDDEL:
                case WORD_RSIDP:
                case WORD_RSIDR:
                case WORD_RSIDRDEFAULT:
                case WORD_RSIDROOT:
                case WORD_RSIDRPR:
                case WORD_RSIDS:
                case WORD_RSIDSECT:
                case WORD_RSIDTR:
                    assert(removeCount+1 < 20);
                    remove[removeCount++] = node->attrs[i].tag;
                    break;
            }
        }

        for (unsigned int i = 0; i < removeCount; i++)
            DFRemoveAttribute(node,remove[i]);
    }
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        Word_stripRSIDsRecursive(child);
    }
}

static void Word_stripRSIDs(WordPackage *package)
{
    if (package->document != NULL)
        Word_stripRSIDsRecursive(package->document->docNode);
    if (package->numbering != NULL)
        Word_stripRSIDsRecursive(package->numbering->docNode);
    if (package->styles != NULL)
        Word_stripRSIDsRecursive(package->styles->docNode);
    if (package->settings != NULL)
        Word_stripRSIDsRecursive(package->settings->docNode);

    if (package->settings != NULL) {
        DFNode *rsids = DFChildWithTag(package->settings->root,WORD_RSIDS);
        if (rsids != NULL)
            DFRemoveNode(rsids);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordPackage                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordPackage *WordPackageRetain(WordPackage *package)
{
    if (package != NULL)
        package->retainCount++;

    return package;
}

void WordPackageRelease(WordPackage *package)
{
    if ((package == NULL) || (--package->retainCount > 0))
        return;

    DFDocumentRelease(package->document);
    DFDocumentRelease(package->numbering);
    DFDocumentRelease(package->styles);
    DFDocumentRelease(package->settings);
    DFDocumentRelease(package->theme);
    DFDocumentRelease(package->footnotes);
    DFDocumentRelease(package->endnotes);
    OPCPackageFree(package->opc);
    free(package);
}

void WordPackageSetDocument(WordPackage *package, DFDocument *document)
{
    DFDocumentRetain(document);
    DFDocumentRelease(package->document);
    package->document = document;
}

void WordPackageSetNumbering(WordPackage *package, DFDocument *numbering)
{
    DFDocumentRetain(numbering);
    DFDocumentRelease(package->numbering);
    package->numbering = numbering;
}

void WordPackageSetStyles(WordPackage *package, DFDocument *styles)
{
    DFDocumentRetain(styles);
    DFDocumentRelease(package->styles);
    package->styles = styles;
}

void WordPackageSetSettings(WordPackage *package, DFDocument *settings)
{
    DFDocumentRetain(settings);
    DFDocumentRelease(package->settings);
    package->settings = settings;
}

void WordPackageSetTheme(WordPackage *package, DFDocument *theme)
{
    DFDocumentRetain(theme);
    DFDocumentRelease(package->theme);
    package->theme = theme;
}

void WordPackageSetFootnotes(WordPackage *package, DFDocument *footnotes)
{
    DFDocumentRetain(footnotes);
    DFDocumentRelease(package->footnotes);
    package->footnotes = footnotes;
}

void WordPackageSetEndnotes(WordPackage *package, DFDocument *endnotes)
{
    DFDocumentRetain(endnotes);
    DFDocumentRelease(package->endnotes);
    package->endnotes = endnotes;
}

static void addMissingParts(WordPackage *package)
{
    if (package->styles == NULL)
        package->styles = DFDocumentNewWithRoot(WORD_STYLES);
    if (package->settings == NULL)
        package->settings = DFDocumentNewWithRoot(WORD_SETTINGS);
    if (package->footnotes == NULL)
        package->footnotes = DFDocumentNewWithRoot(WORD_FOOTNOTES);
    if (package->endnotes == NULL)
        package->endnotes = DFDocumentNewWithRoot(WORD_ENDNOTES);
}

WordPackage *WordPackageOpenNew(DFStorage *storage, DFError **error)
{
    if (DFStorageFormat(storage) != DFFileFormatDocx) {
        DFErrorFormat(error,"Incorrect format: Expected %s, got %s",
                      DFFileFormatToExtension(DFFileFormatDocx),DFFileFormatToExtension(DFStorageFormat(storage)));
        return NULL;
    }

    OPCPackage *opc = OPCPackageOpenNew(storage,error);
    if (opc == NULL)
        return NULL;

    int ok = 0;
    WordPackage *package = (WordPackage *)calloc(1,sizeof(WordPackage));
    package->retainCount = 1;
    package->opc = opc;
    package->documentPart = OPCPackagePartWithURI(package->opc,"/word/document.xml");
    package->document = DFDocumentNewWithRoot(WORD_DOCUMENT);
    DFAppendChild(package->document->root,DFCreateElement(package->document,WORD_BODY));

    OPCRelationshipSetAddId(package->opc->relationships,"rId1",WORDREL_OFFICE_DOCUMENT,package->documentPart->URI,0);
    OPCContentTypesSetOverride(package->opc->contentTypes,package->documentPart->URI,WORDTYPE_OFFICE_DOCUMENT);

//    OPCContentTypesSetDefault(package->_opc->contentTypes,"xml","application/xml");
    OPCContentTypesSetDefault(package->opc->contentTypes,"rels","application/vnd.openxmlformats-package.relationships+xml");
//    OPCContentTypesSetDefault(package->_opc->contentTypes,"jpeg","image/jpeg");

    addMissingParts(package);

    ok = 1;

    if (ok)
        return package;
    WordPackageRelease(package);
    return NULL;
}

WordPackage *WordPackageOpenFrom(DFStorage *storage, DFError **error)
{
    if (DFStorageFormat(storage) != DFFileFormatDocx) {
        DFErrorFormat(error,"Incorrect format: Expected %s, got %s",
                      DFFileFormatToExtension(DFFileFormatDocx),DFFileFormatToExtension(DFStorageFormat(storage)));
        return NULL;
    }

    OPCPackage *opc = OPCPackageOpenFrom(storage,error);
    if (opc == NULL)
        return NULL;

    int ok = 0;
    WordPackage *package = (WordPackage *)calloc(1,sizeof(WordPackage));
    package->retainCount = 1;
    package->opc = opc;

    OPCRelationship *rel;

    rel = OPCRelationshipSetLookupByType(package->opc->relationships,WORDREL_OFFICE_DOCUMENT);
    if (rel != NULL)
        package->documentPart = OPCPackagePartWithURI(package->opc,rel->target);

    if (package->documentPart == NULL) {
        DFErrorFormat(error,"Document part not found");
        goto end;
    }

    assert(package->document == NULL);
    assert(package->numbering == NULL);
    assert(package->styles == NULL);
    assert(package->settings == NULL);
    assert(package->theme == NULL);
    assert(package->footnotes == NULL);
    assert(package->endnotes == NULL);

    if ((package->document = parsePart(package,package->documentPart,error)) == NULL)
        goto end;

    // Numbering
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_NUMBERING);
    OPCPart *numberingPart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (numberingPart != NULL) {
        if ((package->numbering = parsePart(package,numberingPart,error)) == NULL)
            goto end;
    }

    // Styles
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_STYLES);
    OPCPart *stylesPart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (stylesPart != NULL) {
        if ((package->styles = parsePart(package,stylesPart,error)) == NULL)
            goto end;
    }

    // Settings
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_SETTINGS);
    OPCPart *settingsPart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (settingsPart != NULL) {
        if ((package->settings = parsePart(package,settingsPart,error)) == NULL)
            goto end;
    }

    // Theme
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_THEME);
    OPCPart *themePart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (themePart != NULL) {
        if ((package->theme = parsePart(package,themePart,error)) == NULL)
            goto end;
    }

    // Footnotes
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_FOOTNOTES);
    OPCPart *footnotesPart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (footnotesPart != NULL) {
        if ((package->footnotes = parsePart(package,footnotesPart,error)) == NULL)
            goto end;
    }

    // Endnotes
    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_ENDNOTES);
    OPCPart *endnotesPart = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if (endnotesPart != NULL) {
        if ((package->endnotes = parsePart(package,endnotesPart,error)) == NULL)
            goto end;
    }

    addMissingParts(package);

    Word_stripRSIDs(package);

    ok = 1;

end:
    if (ok)
        return package;
    WordPackageRelease(package);
    return NULL;
}

static int serializePart(WordPackage *package, DFDocument *doc, OPCPart *part, DFError **error)
{
    char *str = DFSerializeXMLString(doc,0,0);
    int ok = OPCPackageWritePart(package->opc,str,strlen(str),part,error);
    free(str);
    return ok;
}

static int savePart(WordPackage *package, DFDocument *document,
                    const char *relName, const char *typeName,
                    const char *path, DFError **error)
{
    OPCRelationship *rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,relName);
    OPCPart *part = (rel != NULL) ? OPCPackagePartWithURI(package->opc,rel->target) : NULL;
    if ((document != NULL) && (document->root->first != NULL)) {
        if (part == NULL)
            part = OPCPackageAddRelatedPart(package->opc,path,typeName,relName,package->documentPart);
        if (!serializePart(package,document,part,error))
            return 0;
    }
    else {
        if (part != NULL) {
            OPCPackageDeletePart(package->opc,part,error);
            OPCPackageRemoveRelatedPart(package->opc,part->URI,relName,package->documentPart);
        }
    }
    return 1;
}

int WordPackageSave(WordPackage *package, DFError **error)
{
    // Document
    assert(package->document != NULL);
    assert(package->documentPart != NULL);
    if ((package->document != NULL) && !serializePart(package,package->document,package->documentPart,error))
        return 0;

    // Numbering
    if (!savePart(package,package->numbering,WORDREL_NUMBERING,WORDTYPE_NUMBERING,"/word/numbering.xml",error))
        return 0;

    // Styles
    if (!savePart(package,package->styles,WORDREL_STYLES,WORDTYPE_STYLES,"/word/styles.xml",error))
        return 0;

    // Settings
    if (!savePart(package,package->settings,WORDREL_SETTINGS,WORDTYPE_SETTINGS,"/word/settings.xml",error))
        return 0;

    // Theme
    if (!savePart(package,package->theme,WORDREL_THEME,WORDTYPE_THEME,"/word/theme.xml",error))
        return 0;

    // Footnotes
    if (!savePart(package,package->footnotes,WORDREL_FOOTNOTES,WORDTYPE_FOOTNOTES,"/word/footnotes.xml",error))
        return 0;

    // Endnotes
    if (!savePart(package,package->endnotes,WORDREL_ENDNOTES,WORDTYPE_ENDNOTES,"/word/endnotes.xml",error))
        return 0;

    // Build .docx zip archive, if requested
    if (!OPCPackageSave(package->opc,error))
        return 0;

    return 1;
}

static DFDocument *parsePart(WordPackage *package, OPCPart *part, DFError **error)
{
    DFBuffer *content = OPCPackageReadPart(package->opc,part,error);
    if (content == NULL)
        return NULL;;
    DFDocument *doc = DFParseXMLString(content->data,error);
    DFBufferRelease(content);
    return doc;
}

const char *WordPackageTargetForDocumentRel(WordPackage *package, const char *relId)
{
    OPCRelationship *rel = OPCRelationshipSetLookupById(package->documentPart->relationships,relId);
    if (rel != NULL)
        return rel->target;
    else
        return NULL;
}

int WordPackageSimplifyFields(WordPackage *package)
{
    return Word_simplifyFields(package);
}

void WordPackageCollapseBookmarks(WordPackage *package)
{
    WordBookmarks_collapseNew(package->document);
}

void WordPackageExpandBookmarks(WordPackage *package)
{
    WordBookmarks_expandNew(package->document);
}
