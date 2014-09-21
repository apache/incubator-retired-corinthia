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

#include "Plain.h"
#include "TextPackage.h"
#include "Plain.h"
#include "Commands.h"
#include "OPC.h"
#include "WordConverter.h"
#include "DFXML.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFHTML.h"
#include "DFCommon.h"

static void addSerializedDoc(DFBuffer *result, DFDocument *doc, const char *filename)
{
    if (doc != NULL) {
        char *str = DFSerializeXMLString(doc,0,1);
        DFBufferFormat(result,"#item %s\n",filename);
        DFBufferFormat(result,"%s",str);
        free(str);
    }
}

static void addSerializedBinary(DFBuffer *result, DFBuffer *data, const char *filename)
{
    if (data != NULL) {
        char *str = binaryToString(data);
        DFBufferFormat(result,"#item %s\n",filename);
        DFBufferFormat(result,"%s",str);
        free(str);
    }
}

char *Word_toPlain(WordPackage *package, DFHashTable *parts)
{
    DFHashTable *includeTypes = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(includeTypes,WORDREL_HYPERLINK,"");
    DFHashTableAdd(includeTypes,WORDREL_IMAGE,"");
    DFBuffer *result = DFBufferNew();
    if ((parts == NULL) || (DFHashTableLookup(parts,"document") != NULL)) {
        addSerializedDoc(result,package->document,"document.xml");
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"styles") != NULL)) {
        addSerializedDoc(result,package->styles,"styles.xml");
        DFHashTableAdd(includeTypes,WORDREL_STYLES,"");
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"numbering") != NULL)) {
        if ((package->numbering != NULL) && (package->numbering->root->first != NULL)) {
            // Only include the file if we have one or more numbering definitions
            addSerializedDoc(result,package->numbering,"numbering.xml");
            DFHashTableAdd(includeTypes,WORDREL_NUMBERING,"");
        }
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"footnotes") != NULL)) {
        if (package->footnotes->root->first != NULL) {
            // Only include the file if we have one or more footnotes
            addSerializedDoc(result,package->footnotes,"footnotes.xml");
            DFHashTableAdd(includeTypes,WORDREL_FOOTNOTES,"");
        }
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"endnotes") != NULL)) {
        if (package->endnotes->root->first != NULL) {
            // Only include the file if we have one or more endnotes
            addSerializedDoc(result,package->endnotes,"endnotes.xml");
            DFHashTableAdd(includeTypes,WORDREL_ENDNOTES,"");
        }
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"settings") != NULL)) {
        addSerializedDoc(result,package->settings,"settings.xml");
        DFHashTableAdd(includeTypes,WORDREL_SETTINGS,"");
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"theme") != NULL)) {
        addSerializedDoc(result,package->theme,"theme.xml");
        DFHashTableAdd(includeTypes,WORDREL_THEME,"");
    }

    int haveLinks = 0;
    int haveImages = 0;

    OPCRelationshipSet *oldRels = package->documentPart->relationships;
    OPCRelationshipSet *newRels = OPCRelationshipSetNew();
    const char **oldRelIds = OPCRelationshipSetAllIds(oldRels);
    for (int i = 0; oldRelIds[i]; i++) {
        const char *rId = oldRelIds[i];
        OPCRelationship *rel = OPCRelationshipSetLookupById(oldRels,rId);
        if (DFHashTableLookup(includeTypes,rel->type) != NULL)
            OPCRelationshipSetAddId(newRels,rel->rId,rel->type,rel->target,rel->external);
        if (!strcmp(rel->type,WORDREL_HYPERLINK))
            haveLinks = 1;
        if (!strcmp(rel->type,WORDREL_IMAGE))
            haveImages = 1;
    }
    free(oldRelIds);

    int includeRels = 0;
    if (parts == NULL)
        includeRels = (haveLinks || haveImages);
    else
        includeRels = (DFHashTableLookup(parts,"documentRels") != NULL);

    if (includeRels) {
        DFDocument *relsDoc = OPCRelationshipSetToDocument(newRels);
        addSerializedDoc(result,relsDoc,"document.xml.rels");
        DFDocumentRelease(relsDoc);
    }

    OPCRelationshipSetFree(newRels);

    const char **entries = DFContentsOfDirectory(package->tempPath,1,NULL);
    if (entries != NULL) { // FIXME: Should really report an error if this is not the case
        for (int i = 0; entries[i]; i++) {
            const char *filename = entries[i];
            char *extension = DFPathExtension(filename);
            if (DFStringEqualsCI(extension,"png") || DFStringEqualsCI(extension,"jpg")) {
                char *absFilename;
                if (!DFStringHasSuffix(filename,"/"))
                    absFilename = DFFormatString("/%s",filename);
                else
                    absFilename = strdup(filename);
                char *absPath = DFAppendPathComponent(package->tempPath,absFilename);
                DFBuffer *data = DFBufferReadFromFile(absPath,NULL);
                addSerializedBinary(result,data,absFilename);
                DFBufferRelease(data);
                free(absFilename);
                free(absPath);
            }
            free(extension);
        }
    }
    free(entries);
    DFHashTableRelease(includeTypes);

    char *str = strdup(result->data);
    DFBufferRelease(result);
    return str;
}

static DFDocument *xmlFromString(const char *str, DFError **error)
{
    DFError *err = NULL;
    DFDocument *doc = DFParseXMLString(str,&err);
    if (doc == NULL)
        return NULL;
    DFStripWhitespace(doc->docNode);
    DFDocumentReassignSeqNos(doc);
    return doc;
}

static int Word_fromPackage(WordPackage *package, TextPackage *tp, DFError **error)
{
    const char *documentStr = DFHashTableLookup(tp->items,"document.xml");
    const char *stylesStr = DFHashTableLookup(tp->items,"styles.xml");
    const char *numberingStr = DFHashTableLookup(tp->items,"numbering.xml");
    const char *footnotesStr = DFHashTableLookup(tp->items,"footnotes.xml");
    const char *endnotesStr = DFHashTableLookup(tp->items,"endnotes.xml");
    const char *settingsStr = DFHashTableLookup(tp->items,"settings.xml");
    const char *themeStr = DFHashTableLookup(tp->items,"theme.xml");

    if (documentStr != NULL) {
        DFDocument *document = xmlFromString(documentStr,error);
        if (document == NULL) {
            DFErrorFormat(error,"document.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetDocument(package,document);
        DFDocumentRelease(document);
    }

    if (stylesStr != NULL) {
        DFDocument *styles = xmlFromString(stylesStr,error);
        if (styles == NULL) {
            DFErrorFormat(error,"styles.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetStyles(package,styles);
        DFDocumentRelease(styles);
    }

    if (numberingStr != NULL) {
        DFDocument *numbering = xmlFromString(numberingStr,error);
        if (numbering == NULL) {
            DFErrorFormat(error,"numbering.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetNumbering(package,numbering);
        DFDocumentRelease(numbering);
    }

    if (footnotesStr != NULL) {
        DFDocument *footnotes = xmlFromString(footnotesStr,error);
        if (footnotes == NULL) {
            DFErrorFormat(error,"footnotes.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetFootnotes(package,footnotes);
        DFDocumentRelease(footnotes);
    }

    if (endnotesStr != NULL) {
        DFDocument *endnotes = xmlFromString(endnotesStr,error);
        if (endnotes == NULL) {
            DFErrorFormat(error,"endnotes.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetEndnotes(package,endnotes);
        DFDocumentRelease(endnotes);
    }

    if (settingsStr != NULL) {
        DFDocument *settings = xmlFromString(settingsStr,error);
        if (settings == NULL) {
            DFErrorFormat(error,"settings.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetSettings(package,settings);
        DFDocumentRelease(settings);
    }

    if (themeStr != NULL) {
        DFDocument *theme = xmlFromString(themeStr,error);
        if (theme == NULL) {
            DFErrorFormat(error,"theme.xml: %s",DFErrorMessage(error));
            return 0;
        }
        WordPackageSetTheme(package,theme);
        DFDocumentRelease(theme);
    }

    const char *relsStr = DFHashTableLookup(tp->items,"document.xml.rels");
    if (relsStr != NULL) {
        DFDocument *relsDoc = xmlFromString(relsStr,error);
        if (relsDoc == NULL) {
            DFErrorFormat(error,"document.xml.rels: %s",DFErrorMessage(error));
            return 0;
        }
        OPCPackageReadRelationships(package->opc,package->documentPart->relationships,"/word/document.xml",relsDoc);
        DFDocumentRelease(relsDoc);
    }
    WordPackageSetPartsFromRels(package);

    if (package->stylesPart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->stylesPart->URI,WORDTYPE_STYLES);
    if (package->numberingPart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->numberingPart->URI,WORDTYPE_NUMBERING);
    if (package->settingsPart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->settingsPart->URI,WORDTYPE_SETTINGS);
    if (package->themePart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->themePart->URI,WORDTYPE_THEME);;
    if (package->footnotesPart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->footnotesPart->URI,WORDTYPE_FOOTNOTES);;
    if (package->endnotesPart != NULL)
        OPCContentTypesSetOverride(package->opc->contentTypes,package->endnotesPart->URI,WORDTYPE_ENDNOTES);;

    OPCRelationshipSet *rels = package->documentPart->relationships;
    const char **allIds = OPCRelationshipSetAllIds(rels);
    for (int i = 0; allIds[i]; i++) {
        const char *rId = allIds[i];
        OPCRelationship *rel = OPCRelationshipSetLookupById(rels,rId);
        if (!strcmp(rel->type,WORDREL_IMAGE)) {
            char *ext = DFPathExtension(rel->target);
            if (DFStringEqualsCI(ext,"png")) {
                OPCContentTypesSetDefault(package->opc->contentTypes,"png","image/png");
            }
            else if (DFStringEqualsCI(ext,"jpeg")) {
                OPCContentTypesSetDefault(package->opc->contentTypes,"jpeg","image/jpeg");
            }
            else if (DFStringEqualsCI(ext,"jpg")) {
                OPCContentTypesSetDefault(package->opc->contentTypes,"jpg","image/jpg");
            }
            else {
                DFErrorFormat(error,"Unsupported image type: %s",ext);
                free(ext);
                return 0;
            }
            free(ext);

            const char *str = DFHashTableLookup(tp->items,rel->target);
            char *path = DFAppendPathComponent(package->tempPath,rel->target);
            char *parent = DFPathDirName(path);
            int ok = 1;

            if (!DFFileExists(parent) && !DFCreateDirectory(parent,1,error)) {
                DFErrorFormat(error,"%s: %s",parent,DFErrorMessage(error));
                ok = 0;
            }

            DFBuffer *data = stringToBinary(str);
            if (!DFBufferWriteToFile(data,path,error)) {
                DFErrorFormat(error,"%s: %s",path,DFErrorMessage(error));
                ok = 0;
            }

            DFBufferRelease(data);
            free(parent);
            free(path);

            if (!ok)
                return 0;
        }
    }
    free(allIds);

    return 1;
}

WordPackage *Word_fromPlain(const char *plain, const char *plainPath, const char *packagePath, DFError **error)
{
    WordPackage *wp = WordPackageNew(packagePath);
    if (!WordPackageOpenNew(wp,error)) {
        WordPackageRelease(wp);
        return NULL;
    }

    TextPackage *tp = TextPackageNewWithString(plain,plainPath,error);
    if (tp == NULL) {
        WordPackageRelease(wp);
        return NULL;
    }

    if (!Word_fromPackage(wp,tp,error)) {
        WordPackageRelease(wp);
        TextPackageRelease(tp);
        return NULL;
    }

    TextPackageRelease(tp);
    return wp;
}

static void HTML_getImageSourcesRecursive(DFNode *node, DFHashTable *result)
{
    if (node->tag == HTML_IMG) {
        const char *src = DFGetAttribute(node,HTML_SRC);
        if (src != NULL)
            DFHashTableAdd(result,src,"");
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        HTML_getImageSourcesRecursive(child,result);
}

static const char **HTML_getImageSources(DFDocument *doc)
{
    DFHashTable *set = DFHashTableNew(NULL,NULL);
    HTML_getImageSourcesRecursive(doc->docNode,set);
    const char **result = DFHashTableCopyKeys(set);
    DFHashTableRelease(set);
    return result;
}

char *HTML_toPlain(DFDocument *doc, const char *imagePath, DFError **error)
{
    DFBuffer *output = DFBufferNew();
    char *docStr = DFSerializeXMLString(doc,0,0);
    DFBufferFormat(output,"%s",docStr);
    free(docStr);
    const char **imageSources = HTML_getImageSources(doc);
    DFSortStringsCaseInsensitive(imageSources);
    for (size_t i = 0; imageSources[i]; i++) {
        const char *src = imageSources[i];
        if (DFStringHasPrefix(src,"images/"))
            DFBufferFormat(output,"#item %s\n",src);
        else
            DFBufferFormat(output,"#item images/%s\n",src);
        char *srcPath = DFAppendPathComponent(imagePath,src);
        DFBuffer *imageData = DFBufferReadFromFile(srcPath,error);
        if (imageData == NULL) {
            DFErrorFormat(error,"%s: %s",srcPath,DFErrorMessage(error));
            free(srcPath);
            return NULL;
        }
        char *imageStr = binaryToString(imageData);
        DFBufferFormat(output,"%s",imageStr);
        free(srcPath);
        free(imageStr);
        DFBufferRelease(imageData);
    }
    free(imageSources);

    char *str = strdup(output->data);
    DFBufferRelease(output);
    return str;
}

static DFDocument *HTML_fromPackage(TextPackage *tp, const char *path, const char *htmlPath, DFError **error)
{
    const char *html = DFHashTableLookup(tp->items,"");
    if (html == NULL) {
        DFErrorFormat(error,"No HTML data");
        return NULL;
    }

    DFDocument *doc = DFParseHTMLString(html,0,error);
    if (doc == NULL)
        return NULL;

    for (size_t ki = 0; ki < tp->nkeys; ki++) {
        const char *key = tp->keys[ki];
        if (strlen(key) == 0)
            continue;

        char *thisFullPath = DFAppendPathComponent(htmlPath,key);
        char *thisFullParentPath = DFPathDirName(thisFullPath);

        int ok = 1;

        if (!DFFileExists(thisFullParentPath) && !DFCreateDirectory(thisFullParentPath,1,error)) {
            DFErrorFormat(error,"%s: %s",thisFullParentPath,DFErrorMessage(error));
            DFDocumentRelease(doc);
            ok = 0;
        }

        const char *str = DFHashTableLookup(tp->items,key);
        DFBuffer *data = stringToBinary(str);
        if (!DFBufferWriteToFile(data,thisFullPath,error)) {
            DFErrorFormat(error,"%s: %s",thisFullPath,DFErrorMessage(error));
            DFDocumentRelease(doc);
            ok = 0;
        }

        DFBufferRelease(data);
        free(thisFullPath);
        free(thisFullParentPath);

        if (!ok)
            return NULL;
    }

    return doc;
}

DFDocument *HTML_fromPlain(const char *plain, const char *path, const char *htmlPath, DFError **error)
{
    TextPackage *tp = TextPackageNewWithString(plain,path,error);
    if (tp == NULL)
        return NULL;;
    DFDocument *result = HTML_fromPackage(tp,path,htmlPath,error);
    TextPackageRelease(tp);
    return result;
}
