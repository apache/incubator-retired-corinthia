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
#include "DFZipFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void addStrippedSerializedDoc(DFBuffer *result, DFDocument *doc, const char *filename)
{
    if (doc != NULL) {
        DFStripWhitespace(doc->docNode);
        char *str = DFSerializeXMLString(doc,0,1);
        DFBufferFormat(result,"#item %s\n",filename);
        DFBufferFormat(result,"%s",str);
        free(str);
    }
}

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

static char *findDocumentPath(DFPackage *package, DFError **error)
{
    int ok = 0;
    DFDocument *relsDoc = NULL;
    char *result = NULL;

    relsDoc = DFParseXMLPackage(package,"/_rels/.rels",error);
    if (relsDoc == NULL) {
        DFErrorFormat(error,"_rels/.rels: %s",DFErrorMessage(error));
        goto end;
    }

    for (DFNode *child = relsDoc->root->first; child != NULL; child = child->next) {
        if (child->tag != REL_RELATIONSHIP)
            continue;

        const char *type = DFGetAttribute(child,NULL_Type);
        const char *target = DFGetAttribute(child,NULL_TARGET);
        if ((type == NULL) || (target == NULL))
            continue;

        if (strcmp(type,"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"))
            continue;

        result = strdup(target);
        ok = 1;
        break;
    }

end:
    DFDocumentRelease(relsDoc);
    if (ok)
        return result;
    free(result);
    return NULL;
}

static char *computeDocumentRelsPath(const char *documentPath)
{
    char *documentParent = DFPathDirName(documentPath);
    char *documentFilename = DFPathBaseName(documentPath);
    char *documentRelsPath = DFFormatString("%s/_rels/%s.rels",documentParent,documentFilename);
    free(documentParent);
    free(documentFilename);
    return documentRelsPath;
}

static void parseDocumentRels(DFDocument *relsDoc, DFHashTable *rels, DFError **error)
{
    if (relsDoc == NULL)
        return;
    for (DFNode *child = relsDoc->root->first; child != NULL; child = child->next) {
        if (child->tag != REL_RELATIONSHIP)
            continue;
        const char *type = DFGetAttribute(child,NULL_Type);
        const char *target = DFGetAttribute(child,NULL_TARGET);
        if ((type == NULL) || (target == NULL))
            continue;

        DFHashTableAdd(rels,type,target);
    }
}

static int addRelatedDoc(DFHashTable *parts, DFHashTable *documentRels, const char *relName, const char *filename,
                         DFBuffer *output, DFHashTable *includeTypes, DFPackage *package, DFError **error)
{
    const char *relPath = DFHashTableLookup(documentRels,relName);
    if (relPath == NULL)
        return 1;;

    DFDocument *doc = DFParseXMLPackage(package,relPath,error);
    if (doc == NULL) {
        DFErrorFormat(error,"%s: %s",relPath,DFErrorMessage(error));
        return 0;
    }

    if (doc->root->first != NULL) {
        addStrippedSerializedDoc(output,doc,filename);
        DFHashTableAdd(includeTypes,relName,"");
    }

    DFDocumentRelease(doc);
    return 1;
}

static int processParts(DFHashTable *parts, const char *documentPath, DFDocument *relsDoc,
                        DFHashTable *documentRels,
                        DFBuffer *output, DFPackage *package, DFError **error)
{
    int ok = 0;
    DFHashTable *includeTypes = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(includeTypes,WORDREL_HYPERLINK,"");
    DFHashTableAdd(includeTypes,WORDREL_IMAGE,"");

    if ((parts == NULL) || (DFHashTableLookup(parts,"document") != NULL)) {
        DFDocument *doc = DFParseXMLPackage(package,documentPath,error);
        if (doc == NULL)
            goto end;
        addStrippedSerializedDoc(output,doc,"document.xml");
        DFDocumentRelease(doc);
    }

    if ((parts == NULL) || (DFHashTableLookup(parts,"styles") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_STYLES,"styles.xml",output,includeTypes,package,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"numbering") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_NUMBERING,"numbering.xml",output,includeTypes,package,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"footnotes") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_FOOTNOTES,"footnotes.xml",output,includeTypes,package,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"endnotes") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_ENDNOTES,"endnotes.xml",output,includeTypes,package,error))
            goto end;
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"settings") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_SETTINGS,"settings.xml",output,includeTypes,package,error))
            goto end;
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"theme") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_THEME,"theme.xml",output,includeTypes,package,error))
            goto end;
    }

    if ((DFHashTableLookup(documentRels,WORDREL_HYPERLINK) != NULL) ||
        (DFHashTableLookup(documentRels,WORDREL_IMAGE) != NULL) ||
        ((parts != NULL) && (DFHashTableLookup(parts,"documentRels") != NULL))) {
        if (relsDoc == NULL) {
            DFErrorFormat(error,"document.xml.rels does not exist");
            goto end;
        }
        DFNode *next;
        for (DFNode *child = relsDoc->root->first; child != NULL; child = next) {
            next = child->next;
            if (child->tag != REL_RELATIONSHIP)
                continue;
            const char *type = DFGetAttribute(child,NULL_Type);
            if ((type != NULL) && (DFHashTableLookup(includeTypes,type) == NULL)) {
                DFRemoveNode(child);
            }
        }
        addSerializedDoc(output,relsDoc,"document.xml.rels");
    }

    const char **entries = DFPackageList(package,NULL);
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
                DFBuffer *data = DFBufferReadFromPackage(package,absFilename,NULL);
                addSerializedBinary(output,data,absFilename);
                DFBufferRelease(data);
                free(absFilename);
            }
            free(extension);
        }
    }
    free(entries);
    DFHashTableRelease(includeTypes);

    ok = 1;

end:
    return ok;
}

static char *Word_toPlainFromDir(DFPackage *package, DFHashTable *parts, DFError **error)
{
    char *documentPath = NULL;
    DFHashTable *rels = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFBuffer *output = DFBufferNew();
    char *relsPathRel = NULL;
    DFDocument *relsDoc = NULL;
    int ok = 0;


    documentPath = findDocumentPath(package,error);
    if (documentPath == NULL) {
        DFErrorFormat(error,"findDocumentPath: %s",DFErrorMessage(error));
        goto end;
    }

    relsPathRel = computeDocumentRelsPath(documentPath);
    if (DFPackageExists(package,relsPathRel) && ((relsDoc = DFParseXMLPackage(package,relsPathRel,error)) == NULL)) {
        DFErrorFormat(error,"%s: %s",relsPathRel,DFErrorMessage(error));
        goto end;
    }

    parseDocumentRels(relsDoc,rels,error);

    if (!processParts(parts,documentPath,relsDoc,rels,output,package,error))
        goto end;

    ok = 1;

end:
    free(relsPathRel);
    free(documentPath);
    DFHashTableRelease(rels);
    DFDocumentRelease(relsDoc);
    if (!ok) {
        DFBufferRelease(output);
        return NULL;
    }
    else {
        char *result = strdup(output->data);
        DFBufferRelease(output);
        return result;
    }
}

static char *Word_toPlainOrError(WordPackage *wordPackage, DFPackage *rawPackage,
                                 DFHashTable *parts, const char *tempPath, DFError **error)
{
    char *docxPath = DFAppendPathComponent(tempPath,"file.docx");
    char *result = NULL;
    int ok = 0;

    if (!DFEmptyDirectory(tempPath,error)) {
        DFErrorFormat(error,"%s: %s",tempPath,DFErrorMessage(error));
        goto end;
    }

    if (!WordPackageSaveTo(wordPackage,docxPath,error)) {
        DFErrorFormat(error,"WordPackageSaveTo: %s",DFErrorMessage(error));
        goto end;
    }

    result = Word_toPlainFromDir(rawPackage,parts,error);
    ok = 1;

end:
    free(docxPath);
    if (ok)
        return result;
    free(result);
    return 0;
}

char *Word_toPlain(WordPackage *wordPackage, DFPackage *rawPackage, DFHashTable *parts, const char *tempPath)
{
    DFError *error = NULL;
    char *result = Word_toPlainOrError(wordPackage,rawPackage,parts,tempPath,&error);
    if (result == NULL) {
        result = DFFormatString("%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
    }
    return result;
}

static int saveXMLDocument(DFPackage *package, const char *filename, DFDocument *doc, NamespaceID defaultNS, DFError **error)
{
    char *parentPath = DFPathDirName(filename);
    int ok = 0;

    if (!DFSerializeXMLPackage(doc,defaultNS,0,package,filename,error)) {
        DFErrorFormat(error,"serialize %s: %s",filename,DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    free(parentPath);
    return ok;
}

static int saveStrippedXMLText(DFPackage *package, const char *filename,
                               const char *input, NamespaceID defaultNS, DFError **error)
{
    DFDocument *doc = DFParseXMLString(input,error);
    if (doc == NULL)
        return 0;
    DFStripWhitespace(doc->docNode);
    int ok = saveXMLDocument(package,filename,doc,defaultNS,error);
    DFDocumentRelease(doc);
    return ok;
}

typedef struct PartInfo {
    const char *filename;
    const char *path;
    const char *rel;
    const char *type;
} PartInfo;

static int saveContentTypes(DFPackage *package, DFHashTable *ctDefaults, DFHashTable *ctOverrides, DFError **error)
{
    DFDocument *doc = DFDocumentNewWithRoot(CT_TYPES);

    const char **keys = DFHashTableCopyKeys(ctDefaults);
    DFSortStringsCaseInsensitive(keys);
    for (int i = 0; keys[i]; i++) {
        const char *extension = keys[i];
        const char *contentType = DFHashTableLookup(ctDefaults,extension);
        DFNode *deflt = DFCreateChildElement(doc->root,CT_DEFAULT);
        DFSetAttribute(deflt,NULL_EXTENSION,extension);
        DFSetAttribute(deflt,NULL_CONTENTTYPE,contentType);
    }
    free(keys);
    keys = DFHashTableCopyKeys(ctOverrides);
    DFSortStringsCaseInsensitive(keys);
    for (int i = 0; keys[i]; i++) {
        const char *partName = keys[i];
        const char *contentType = DFHashTableLookup(ctOverrides,partName);
        DFNode *override = DFCreateChildElement(doc->root,CT_OVERRIDE);
        DFSetAttribute(override,NULL_PARTNAME,partName);
        DFSetAttribute(override,NULL_CONTENTTYPE,contentType);
    }
    free(keys);

    int ok = saveXMLDocument(package,"[Content_Types].xml",doc,NAMESPACE_CT,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveDocRels(DFPackage *package,
                       DFHashTable *docRelURIs,
                       DFHashTable *docRelTypes,
                       DFHashTable *docRelModes,
                       DFError **error)
{
    if (DFHashTableCount(docRelURIs) == 0)
        return 1;;

    DFDocument *doc = DFDocumentNewWithRoot(REL_RELATIONSHIPS);

    const char **sortedIds = DFHashTableCopyKeys(docRelURIs);
    DFSortStringsCaseInsensitive(sortedIds);
    for (int i = 0; sortedIds[i]; i++) {
        const char *rId = sortedIds[i];
        const char *URI = DFHashTableLookup(docRelURIs,rId);
        const char *type = DFHashTableLookup(docRelTypes,rId);
        const char *mode = DFHashTableLookup(docRelModes,rId); // may be NULL
        DFNode *child = DFCreateChildElement(doc->root,REL_RELATIONSHIP);
        DFSetAttribute(child,NULL_Id,rId);
        DFSetAttribute(child,NULL_Type,type);
        DFSetAttribute(child,NULL_TARGET,URI);
        DFSetAttribute(child,NULL_TARGETMODE,mode);
    }
    free(sortedIds);

    int ok = saveXMLDocument(package,"/word/_rels/document.xml.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveRootRels(DFPackage *package, DFError **error)
{
    DFDocument *doc = DFDocumentNewWithRoot(REL_RELATIONSHIPS);
    DFNode *rel = DFCreateChildElement(doc->root,REL_RELATIONSHIP);
    DFSetAttribute(rel,NULL_Id,"rId1");
    DFSetAttribute(rel,NULL_Type,"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
    DFSetAttribute(rel,NULL_TARGET,"/word/document.xml");
    int ok = saveXMLDocument(package,"/_rels/.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int Word_fromPackage(TextPackage *tp, DFPackage *store, DFError **error)
{
    PartInfo parts[7] = {
        { "numbering.xml", "/word/numbering.xml", WORDREL_NUMBERING, WORDTYPE_NUMBERING },
        { "styles.xml", "/word/styles.xml", WORDREL_STYLES, WORDTYPE_STYLES },
        { "settings.xml", "/word/settings.xml", WORDREL_SETTINGS, WORDTYPE_SETTINGS },
        { "theme.xml", "/word/theme.xml", WORDREL_THEME, WORDTYPE_THEME },
        { "footnotes.xml", "/word/footnotes.xml", WORDREL_FOOTNOTES, WORDTYPE_FOOTNOTES },
        { "endnotes.xml", "/word/endnotes.xml", WORDREL_ENDNOTES, WORDTYPE_ENDNOTES },
        { NULL, NULL, NULL, NULL },
    };

    int ok = 0;

    const char *documentStr = DFHashTableLookup(tp->items,"document.xml");
    const char **allFilenames = NULL;
    DFHashTable *ctDefaults = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFHashTable *ctOverrides = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFHashTable *docRelURIs = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFHashTable *docRelTypes = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFHashTable *docRelModes = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);


    if (documentStr == NULL) {
        DFErrorFormat(error,"No document.xml");
        goto end;
    }

    DFHashTableAdd(ctDefaults,"rels","application/vnd.openxmlformats-package.relationships+xml");
    DFHashTableAdd(ctOverrides,"/word/document.xml",
                   "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");


    if (documentStr != NULL) {
        if (!saveStrippedXMLText(store,"/word/document.xml",documentStr,NAMESPACE_NULL,error))
            goto end;
    }

    int rIdNext = 1;
    for (int i = 0; parts[i].filename; i++) {
        const char *content = DFHashTableLookup(tp->items,parts[i].filename);
        if (content == NULL)
            continue;

        if (!saveStrippedXMLText(store,parts[i].path,content,NAMESPACE_NULL,error))
            goto end;

        char rIdStr[100];
        snprintf(rIdStr,100,"rId%d",rIdNext++);
        DFHashTableAdd(docRelURIs,rIdStr,parts[i].path);
        DFHashTableAdd(docRelTypes,rIdStr,parts[i].rel);
        DFHashTableAdd(ctOverrides,parts[i].path,parts[i].type);
    }

    allFilenames = DFHashTableCopyKeys(tp->items);
    for (int i = 0; allFilenames[i]; i++) {
        const char *curFilename = allFilenames[i];
        char *ext = DFPathExtension(curFilename);

        int isImage = 0;

        if (DFStringEqualsCI(ext,"png")) {
            DFHashTableAdd(ctDefaults,"png","image/png");
            isImage = 1;
        }

        if (DFStringEqualsCI(ext,"jpg")) {
            DFHashTableAdd(ctDefaults,"jpg","image/png");
            isImage = 1;
        }

        if (DFStringEqualsCI(ext,"jpeg")) {
            DFHashTableAdd(ctDefaults,"jpeg","image/png");
            isImage = 1;
        }

        free(ext);

        if (isImage) {
            const char *str = DFHashTableLookup(tp->items,curFilename);
            char *parentRel = DFPathDirName(curFilename);
            int fileok = 1;

            DFBuffer *data = stringToBinary(str);
            if (!DFBufferWriteToPackage(data,store,curFilename,error)) {
                DFErrorFormat(error,"%s: %s",curFilename,DFErrorMessage(error));
                fileok = 0;
            }

            DFBufferRelease(data);
            free(parentRel);

            if (!fileok)
                goto end;
        }
    }

    if (!saveContentTypes(store,ctDefaults,ctOverrides,error)) {
        DFErrorFormat(error,"saveContentTypes: %s",DFErrorMessage(error));
        goto end;
    }

    const char *relsStr = DFHashTableLookup(tp->items,"document.xml.rels");
    if (relsStr != NULL) {
        DFDocument *doc = DFParseXMLString(relsStr,error);
        if (doc == NULL)
            goto end;

        DFHashTableRelease(docRelURIs);
        DFHashTableRelease(docRelTypes);
        DFHashTableRelease(docRelModes);
        docRelURIs = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
        docRelTypes = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
        docRelModes = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);

        for (DFNode *child = doc->root->first; child != NULL; child = child->next) {
            if (child->tag == REL_RELATIONSHIP) {
                const char *rId = DFGetAttribute(child,NULL_Id);
                const char *type = DFGetAttribute(child,NULL_Type);
                const char *target = DFGetAttribute(child,NULL_TARGET);
                const char *mode = DFGetAttribute(child,NULL_TARGETMODE);

                if ((rId != NULL) && (type != NULL) && (target != NULL)) {
                    DFHashTableAdd(docRelURIs,rId,target);
                    DFHashTableAdd(docRelTypes,rId,type);
                    if (mode != NULL)
                        DFHashTableAdd(docRelModes,rId,mode);
                }
            }
        }

        DFDocumentRelease(doc);
    }

    if (!saveDocRels(store,docRelURIs,docRelTypes,docRelModes,error)) {
        DFErrorFormat(error,"saveDocRels: %s",DFErrorMessage(error));
        goto end;
    }

    if (!saveRootRels(store,error)) {
        DFErrorFormat(error,"saveRootRels: %s",DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    DFHashTableRelease(ctDefaults);
    DFHashTableRelease(ctOverrides);
    DFHashTableRelease(docRelURIs);
    DFHashTableRelease(docRelTypes);
    DFHashTableRelease(docRelModes);
    free(allFilenames);
    return ok;
}

int Word_fromPlain(const char *plain, const char *plainPath, const char *zipTempPath,
                   WordPackage **outWordPackage, DFPackage **outRawPackage, DFError **error)
{
    int ok = 0;
    char *docxPath = DFAppendPathComponent(zipTempPath,"document.docx");
    DFPackage *firstStore = DFPackageNewMemory();
    DFPackage *secondStore = DFPackageNewMemory();
    WordPackage *wp = NULL;
    TextPackage *tp = NULL;

    tp = TextPackageNewWithString(plain,plainPath,error);
    if (tp == NULL)
        goto end;

    if (DFFileExists(zipTempPath) && !DFDeleteFile(zipTempPath,error)) {
        DFErrorFormat(error,"delete %s: %s",zipTempPath,DFErrorMessage(error));
        goto end;
    }

    if (!DFCreateDirectory(zipTempPath,1,error)) {
        DFErrorFormat(error,"create %s: %s",zipTempPath,DFErrorMessage(error));
        goto end;
    }

    if (!Word_fromPackage(tp,firstStore,error)) {
        DFErrorFormat(error,"Word_fromPackageNew: %s",DFErrorMessage(error));
        printf("%s\n",DFErrorMessage(error));
        goto end;
    }

    if (!DFZip(docxPath,firstStore,error)) {
        DFErrorFormat(error,"zip %s: %s",docxPath,DFErrorMessage(error));
        goto end;
    }

    // Now we have a .docx file; access it using what will be the new way (this API will change so we just say
    // "open a word document from here", without having to separately create the package object first.
    wp = WordPackageOpenFrom(secondStore,docxPath,error);
    if (wp == NULL) {
        DFErrorFormat(error,"WordPackageStartFrom %s: %s",docxPath,DFErrorMessage(error));
        goto end;
    }
    ok = 1;

end:
    free(docxPath);
    DFPackageRelease(firstStore);
    TextPackageRelease(tp);
    if (ok) {
        *outWordPackage = wp;
        *outRawPackage = secondStore;
        return 1;
    }
    else {
        WordPackageRelease(wp);
        DFPackageRelease(secondStore);
        *outWordPackage = NULL;
        *outRawPackage = NULL;
        return 0;
    }
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
