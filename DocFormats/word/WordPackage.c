// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "WordPackage.h"
#include "WordStyles.h"
#include "WordSettings.h"
#include "DFDOM.h"
#include "DFFilesystem.h"
#include "DFXML.h"
#include "OPC.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "WordField.h"
#include "WordBookmark.h"
#include "DFCommon.h"

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

WordPackage *WordPackageNew(const char *tempPath)
{
    WordPackage *package = (WordPackage *)calloc(1,sizeof(WordPackage));
    package->retainCount = 1;
    package->tempPath = strdup(tempPath);
    package->opc = OPCPackageNew(tempPath);
    return package;
}

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

    free(package->tempPath);
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

static int WordPackageSetupTempPath(WordPackage *package, DFError **error)
{
    if (DFFileExists(package->tempPath) && !DFDeleteFile(package->tempPath,error))
        return 0;
    if (!DFCreateDirectory(package->tempPath,1,error))
        return 0;
    return 1;
}

int WordPackageOpenNew(WordPackage *package, DFError **error)
{
    if (!WordPackageSetupTempPath(package,error))
        return 0;
    if (!OPCPackageOpenNew(package->opc,error))
        return 0;

    package->documentPart = OPCPackagePartWithURI(package->opc,"/word/document.xml");
    package->document = DFDocumentNewWithRoot(WORD_DOCUMENT);
    DFAppendChild(package->document->root,DFCreateElement(package->document,WORD_BODY));

    OPCRelationshipSetAddId(package->opc->relationships,"rId1",WORDREL_OFFICE_DOCUMENT,package->documentPart->URI,0);
    OPCContentTypesSetOverride(package->opc->contentTypes,package->documentPart->URI,WORDTYPE_OFFICE_DOCUMENT);

//    OPCContentTypesSetDefault(package->_opc->contentTypes,"xml","application/xml");
    OPCContentTypesSetDefault(package->opc->contentTypes,"rels","application/vnd.openxmlformats-package.relationships+xml");
//    OPCContentTypesSetDefault(package->_opc->contentTypes,"jpeg","image/jpeg");

    addMissingParts(package);

    return 1;
}

void WordPackageSetPartsFromRels(WordPackage *package)
{
    OPCRelationship *rel;

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_NUMBERING);
    if (rel != NULL)
        package->numberingPart = OPCPackagePartWithURI(package->opc,rel->target);

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_STYLES);
    if (rel != NULL)
        package->stylesPart = OPCPackagePartWithURI(package->opc,rel->target);

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_SETTINGS);
    if (rel != NULL)
        package->settingsPart = OPCPackagePartWithURI(package->opc,rel->target);

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_THEME);
    if (rel != NULL)
        package->themePart = OPCPackagePartWithURI(package->opc,rel->target);

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_FOOTNOTES);
    if (rel != NULL)
        package->footnotesPart = OPCPackagePartWithURI(package->opc,rel->target);

    rel = OPCRelationshipSetLookupByType(package->documentPart->relationships,WORDREL_ENDNOTES);
    if (rel != NULL)
        package->endnotesPart = OPCPackagePartWithURI(package->opc,rel->target);
}

int WordPackageOpenFrom(WordPackage *package, const char *filename, DFError **error)
{
    if (!WordPackageSetupTempPath(package,error))
        return 0;
    if (!OPCPackageOpenFrom(package->opc,filename)) {
        DFErrorFormat(error,"%s",package->opc->errors->data);
        return 0;
    }

    OPCRelationship *rel;

    rel = OPCRelationshipSetLookupByType(package->opc->relationships,WORDREL_OFFICE_DOCUMENT);
    if (rel != NULL)
        package->documentPart = OPCPackagePartWithURI(package->opc,rel->target);

    if (package->documentPart == NULL) {
        DFErrorFormat(error,"Document part not found");
        return 0;
    }

    assert(package->document == NULL);
    assert(package->numbering == NULL);
    assert(package->styles == NULL);
    assert(package->settings == NULL);
    assert(package->theme == NULL);
    assert(package->footnotes == NULL);
    assert(package->endnotes == NULL);

    if ((package->document = parsePart(package,package->documentPart,error)) == NULL)
        return 0;

    WordPackageSetPartsFromRels(package);

    if (package->numberingPart != NULL) {
        if ((package->numbering = parsePart(package,package->numberingPart,error)) == NULL)
            return 0;
    }

    if (package->stylesPart != NULL) {
        if ((package->styles = parsePart(package,package->stylesPart,error)) == NULL)
            return 0;
    }

    if (package->settingsPart != NULL) {
        if ((package->settings = parsePart(package,package->settingsPart,error)) == NULL)
            return 0;
    }

    if (package->themePart != NULL) {
        if ((package->theme = parsePart(package,package->themePart,error)) == NULL)
            return 0;
    }

    if (package->footnotesPart != NULL) {
        if ((package->footnotes = parsePart(package,package->footnotesPart,error)) == NULL)
            return 0;
    }

    if (package->endnotesPart != NULL) {
        if ((package->endnotes = parsePart(package,package->endnotesPart,error)) == NULL)
            return 0;
    }

    addMissingParts(package);

    Word_stripRSIDs(package);

    return 1;
}

static int serializePart(WordPackage *package, DFDocument *doc, OPCPart *part, DFError **error)
{
    char *str = DFSerializeXMLString(doc,0,0);
    int ok = OPCPackageWritePart(package->opc,str,strlen(str),part,error);
    free(str);
    return ok;
}

int WordPackageSaveTo(WordPackage *package, const char *filename, DFError **error)
{
    // Document
    assert(package->document != NULL);
    assert(package->documentPart != NULL);
    if ((package->document != NULL) && !serializePart(package,package->document,package->documentPart,error))
        return 0;

    // Numbering
    if (package->numbering != NULL) {
        if (package->numberingPart == NULL) {
            package->numberingPart = OPCPackageAddRelatedPart(package->opc,"/word/numbering.xml",
                                                              WORDTYPE_NUMBERING,
                                                              WORDREL_NUMBERING,
                                                              package->documentPart);
        }
        if (!serializePart(package,package->numbering,package->numberingPart,error))
            return 0;
    }

    // Styles
    if (package->styles != NULL) {
        if (package->stylesPart == NULL) {
            package->stylesPart = OPCPackageAddRelatedPart(package->opc,"/word/styles.xml",
                                                           WORDTYPE_STYLES,
                                                           WORDREL_STYLES,
                                                           package->documentPart);
        }
        if (!serializePart(package,package->styles,package->stylesPart,error))
            return 0;
    }

    // Settings
    if (package->settings != NULL) {
        if (package->settingsPart == NULL) {
            package->settingsPart = OPCPackageAddRelatedPart(package->opc,"/word/settings.xml",
                                                             WORDTYPE_SETTINGS,
                                                             WORDREL_SETTINGS,
                                                             package->documentPart);
        }
        if (!serializePart(package,package->settings,package->settingsPart,error))
            return 0;
    }

    // Theme
    if (package->theme != NULL) {
        if (package->themePart == NULL) {
            package->themePart = OPCPackageAddRelatedPart(package->opc,"/word/theme.xml",
                                                          WORDTYPE_THEME,
                                                          WORDREL_THEME,
                                                          package->documentPart);
        }
        if (!serializePart(package,package->theme,package->themePart,error))
            return 0;
    }

    // Footnotes
    if (package->footnotes != NULL) {
        if (package->footnotesPart == NULL) {
            package->footnotesPart = OPCPackageAddRelatedPart(package->opc,"/word/footnotes.xml",
                                                              WORDTYPE_FOOTNOTES,
                                                              WORDREL_FOOTNOTES,
                                                              package->documentPart);
        }
        if (!serializePart(package,package->footnotes,package->footnotesPart,error))
            return 0;
    }

    // Endnotes
    if (package->endnotes != NULL) {
        if (package->endnotesPart == NULL) {
            package->endnotesPart = OPCPackageAddRelatedPart(package->opc,"/word/endnotes.xml",
                                                             WORDTYPE_ENDNOTES,
                                                             WORDREL_ENDNOTES,
                                                             package->documentPart);
        }
        if (!serializePart(package,package->endnotes,package->endnotesPart,error))
            return 0;
    }

    // Build .docx zip archive, if requested
    if ((filename != NULL) && !OPCPackageSaveTo(package->opc,filename)) {
        DFErrorFormat(error,"%s",package->opc->errors->data);
        return 0;
    }

    return 1;
}

DFDocument *WordPackageGenerateHTML(WordPackage *package, const char *path, const char *idPrefix,
                                    DFError **error, DFBuffer *warnings)
{
    DFDocument *html = DFDocumentNew();
    WordConverter *converter = WordConverterNew(html,path,idPrefix,package,warnings);
    int ok = WordConverterConvertToHTML(converter,error);
    WordConverterFree(converter);
    if (!ok)
        return 0;
    return html;
}

int WordPackageUpdateFromHTML(WordPackage *package, DFDocument *input, const char *path,
                              const char *idPrefix, DFError **error, DFBuffer *warnings)
{
    HTML_normalizeDocument(input);
    HTML_pushDownInlineProperties(input->docNode);
    WordConverter *converter = WordConverterNew(input,path,idPrefix,package,warnings);
    int ok = WordConverterUpdateFromHTML(converter,error);
    WordConverterFree(converter);
    if (!ok)
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

void WordPackageRemovePointlessElements(WordPackage *package)
{
    if (package->styles != NULL) {
        DFNode *latentStyles = DFChildWithTag(package->styles->root,WORD_LATENTSTYLES);
        if (latentStyles != NULL)
            DFRemoveNode(latentStyles);
    }
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
