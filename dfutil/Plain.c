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
        if ((package->styles != NULL) && (package->styles->root->first != NULL)) {
            addSerializedDoc(result,package->styles,"styles.xml");
            DFHashTableAdd(includeTypes,WORDREL_STYLES,"");
        }
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"numbering") != NULL)) {
        if ((package->numbering != NULL) && (package->numbering->root->first != NULL)) {
            // Only include the file if we have one or more numbering definitions
            addSerializedDoc(result,package->numbering,"numbering.xml");
            DFHashTableAdd(includeTypes,WORDREL_NUMBERING,"");
        }
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"footnotes") != NULL)) {
        if ((package->footnotes != NULL) && (package->footnotes->root->first != NULL)) {
            // Only include the file if we have one or more footnotes
            addSerializedDoc(result,package->footnotes,"footnotes.xml");
            DFHashTableAdd(includeTypes,WORDREL_FOOTNOTES,"");
        }
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"endnotes") != NULL)) {
        if ((package->endnotes != NULL) && (package->endnotes->root->first != NULL)) {
            // Only include the file if we have one or more endnotes
            addSerializedDoc(result,package->endnotes,"endnotes.xml");
            DFHashTableAdd(includeTypes,WORDREL_ENDNOTES,"");
        }
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"settings") != NULL)) {
        if ((package->settings != NULL) && (package->settings->root->first != NULL)) {
            addSerializedDoc(result,package->settings,"settings.xml");
            DFHashTableAdd(includeTypes,WORDREL_SETTINGS,"");
        }
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"theme") != NULL)) {
        if ((package->theme != NULL) && (package->theme->root->first != NULL)) {
            addSerializedDoc(result,package->theme,"theme.xml");
            DFHashTableAdd(includeTypes,WORDREL_THEME,"");
        }
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

static int saveXMLDocument(const char *zipDir, const char *filename,
                           DFDocument *doc, NamespaceID defaultNS, DFError **error)
{
    char *fullPath = DFAppendPathComponent(zipDir,filename);
    char *parentPath = DFPathDirName(fullPath);

    int ok = 0;

    if (!DFFileExists(parentPath) && !DFCreateDirectory(parentPath,1,error)) {
        DFErrorFormat(error,"create %s: %s",parentPath,DFErrorMessage(error));
        goto end;
    }

    if (!DFSerializeXMLFile(doc,defaultNS,0,fullPath,error)) {
        DFErrorFormat(error,"serialize %s: %s",fullPath,DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    free(fullPath);
    free(parentPath);
    return ok;
}

static int saveStrippedXMLText(const char *zipDir, const char *filename,
                               const char *input, NamespaceID defaultNS, DFError **error)
{
    DFDocument *doc = DFParseXMLString(input,error);
    if (doc == NULL)
        return 0;
    DFStripWhitespace(doc->docNode);
    int ok = saveXMLDocument(zipDir,filename,doc,defaultNS,error);
    DFDocumentRelease(doc);
    return ok;
}

typedef struct PartInfo {
    const char *filename;
    const char *path;
    const char *rel;
    const char *type;
} PartInfo;

static int saveContentTypes(const char *zipDir, DFHashTable *ctDefaults, DFHashTable *ctOverrides, DFError **error)
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

    int ok = saveXMLDocument(zipDir,"[Content_Types].xml",doc,NAMESPACE_CT,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveDocRels(const char *zipDir,
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

    int ok = saveXMLDocument(zipDir,"/word/_rels/document.xml.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveRootRels(const char *zipDir, DFError **error)
{
    DFDocument *doc = DFDocumentNewWithRoot(REL_RELATIONSHIPS);
    DFNode *rel = DFCreateChildElement(doc->root,REL_RELATIONSHIP);
    DFSetAttribute(rel,NULL_Id,"rId1");
    DFSetAttribute(rel,NULL_Type,"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
    DFSetAttribute(rel,NULL_TARGET,"/word/document.xml");
    int ok = saveXMLDocument(zipDir,"/_rels/.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int Word_fromPackage(TextPackage *tp, const char *zipDir, DFError **error)
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
    if (DFFileExists(zipDir) && !DFDeleteFile(zipDir,error)) {
        DFErrorFormat(error,"delete %s: %s",zipDir,DFErrorMessage(error));
        return 0;
    }

    if (!DFCreateDirectory(zipDir,1,error)) {
        DFErrorFormat(error,"create %s: %s",zipDir,DFErrorMessage(error));
        return 0;
    }

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
        if (!saveStrippedXMLText(zipDir,"/word/document.xml",documentStr,NAMESPACE_NULL,error))
            goto end;
    }

    int rIdNext = 1;
    for (int i = 0; parts[i].filename; i++) {
        const char *content = DFHashTableLookup(tp->items,parts[i].filename);
        if (content == NULL)
            continue;

        if (!saveStrippedXMLText(zipDir,parts[i].path,content,NAMESPACE_NULL,error))
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
            char *path = DFAppendPathComponent(zipDir,curFilename);
            char *parent = DFPathDirName(path);
            int fileok = 1;

            if (!DFFileExists(parent) && !DFCreateDirectory(parent,1,error)) {
                DFErrorFormat(error,"%s: %s",parent,DFErrorMessage(error));
                fileok = 0;
            }

            DFBuffer *data = stringToBinary(str);
            if (!DFBufferWriteToFile(data,path,error)) {
                DFErrorFormat(error,"%s: %s",path,DFErrorMessage(error));
                fileok = 0;
            }

            DFBufferRelease(data);
            free(parent);
            free(path);

            if (!fileok)
                goto end;
        }
    }

    if (!saveContentTypes(zipDir,ctDefaults,ctOverrides,error)) {
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

    if (!saveDocRels(zipDir,docRelURIs,docRelTypes,docRelModes,error)) {
        DFErrorFormat(error,"saveDocRels: %s",DFErrorMessage(error));
        goto end;
    }

    if (!saveRootRels(zipDir,error)) {
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

WordPackage *Word_fromPlain(const char *plain, const char *plainPath,
                            const char *packagePath, const char *zipTempPath,
                            DFError **error)
{
    int ok = 0;
    char *unzippedPath = DFAppendPathComponent(zipTempPath,"unzipped");
    char *contentsPath = DFAppendPathComponent(zipTempPath,"contents2");
    char *docxPath = DFAppendPathComponent(zipTempPath,"document.docx");
    WordPackage *wp = NULL;
    TextPackage *tp = NULL;

    tp = TextPackageNewWithString(plain,plainPath,error);
    if (tp == NULL)
        goto end;

    if (!Word_fromPackage(tp,contentsPath,error)) {
        DFErrorFormat(error,"Word_fromPackageNew: %s",DFErrorMessage(error));
        printf("%s\n",DFErrorMessage(error));
        goto end;
    }

    if (!DFZip(docxPath,contentsPath,error)) {
        DFErrorFormat(error,"zip %s: %s",docxPath,DFErrorMessage(error));
        goto end;
    }

    // Now we have a .docx file; access it using what will be the new way (this API will change so we just say
    // "open a word document from here", without having to separately create the package object first.
    wp = WordPackageNew(unzippedPath);
    if (!WordPackageOpenFrom(wp,docxPath,error)) {
        DFErrorFormat(error,"WordPackageOpenFrom %s: %s",docxPath,DFErrorMessage(error));
        goto end;
    }
    ok = 1;

end:
    free(unzippedPath);
    free(contentsPath);
    free(docxPath);
    TextPackageRelease(tp);
    if (ok) {
        return wp;
    }
    else {
        WordPackageRelease(wp);
        return NULL;
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
