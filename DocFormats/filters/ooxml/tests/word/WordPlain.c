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
#include "WordPlain.h"
#include "TextPackage.h"
#include "OPC.h"
#include "WordConverter.h"
#include "DFXML.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFHTML.h"
#include "DFCommon.h"
#include "DFZipFile.h"
#include "DFUnitTest.h"
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

static char *findDocumentPath(DFStorage *storage, DFError **error)
{
    int ok = 0;
    DFDocument *relsDoc = NULL;
    char *result = NULL;

    relsDoc = DFParseXMLStorage(storage,"/_rels/.rels",error);
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

static void parseDocumentRels(const char *documentPath, DFDocument *relsDoc, DFHashTable *rels, DFError **error)
{
    if (relsDoc == NULL)
        return;;
    const char *basePrefix = (documentPath[0] == '/') ? "" : "/";
    char *basePath = DFFormatString("%s%s",basePrefix,documentPath);
    for (DFNode *child = relsDoc->root->first; child != NULL; child = child->next) {
        if (child->tag != REL_RELATIONSHIP)
            continue;
        const char *type = DFGetAttribute(child,NULL_Type);
        const char *target = DFGetAttribute(child,NULL_TARGET);
        if ((type == NULL) || (target == NULL))
            continue;

        char *absTarget = DFPathResolveAbsolute(basePath,target);
        DFHashTableAdd(rels,type,absTarget);
        free(absTarget);
    }
    free(basePath);
}

static int addRelatedDoc(DFHashTable *parts, DFHashTable *documentRels, const char *relName, const char *filename,
                         DFBuffer *output, DFHashTable *includeTypes, DFStorage *storage, DFError **error)
{
    const char *relPath = DFHashTableLookup(documentRels,relName);
    if (relPath == NULL)
        return 1;;

    DFDocument *doc = DFParseXMLStorage(storage,relPath,error);
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
                        DFBuffer *output, DFStorage *storage, DFError **error)
{
    int ok = 0;
    DFHashTable *includeTypes = DFHashTableNew((DFCopyFunction)strdup,free);
    DFHashTableAdd(includeTypes,WORDREL_HYPERLINK,"");
    DFHashTableAdd(includeTypes,WORDREL_IMAGE,"");

    if ((parts == NULL) || (DFHashTableLookup(parts,"document") != NULL)) {
        DFDocument *doc = DFParseXMLStorage(storage,documentPath,error);
        if (doc == NULL)
            goto end;
        addStrippedSerializedDoc(output,doc,"document.xml");
        DFDocumentRelease(doc);
    }

    if ((parts == NULL) || (DFHashTableLookup(parts,"styles") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_STYLES,"styles.xml",output,includeTypes,storage,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"numbering") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_NUMBERING,"numbering.xml",output,includeTypes,storage,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"footnotes") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_FOOTNOTES,"footnotes.xml",output,includeTypes,storage,error))
            goto end;
    }
    if ((parts == NULL) || (DFHashTableLookup(parts,"endnotes") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_ENDNOTES,"endnotes.xml",output,includeTypes,storage,error))
            goto end;
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"settings") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_SETTINGS,"settings.xml",output,includeTypes,storage,error))
            goto end;
    }
    if ((parts != NULL) && (DFHashTableLookup(parts,"theme") != NULL)) {
        if (!addRelatedDoc(parts,documentRels,WORDREL_THEME,"theme.xml",output,includeTypes,storage,error))
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

    const char **entries = DFStorageList(storage,NULL);
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
                DFBuffer *data = DFBufferReadFromStorage(storage,absFilename,NULL);
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

static char *Word_toPlainFromDir(DFStorage *storage, DFHashTable *parts, DFError **error)
{
    char *documentPath = NULL;
    DFHashTable *rels = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    DFBuffer *output = DFBufferNew();
    char *relsPathRel = NULL;
    DFDocument *relsDoc = NULL;
    int ok = 0;


    documentPath = findDocumentPath(storage,error);
    if (documentPath == NULL) {
        DFErrorFormat(error,"findDocumentPath: %s",DFErrorMessage(error));
        goto end;
    }

    relsPathRel = computeDocumentRelsPath(documentPath);
    if (DFStorageExists(storage,relsPathRel) && ((relsDoc = DFParseXMLStorage(storage,relsPathRel,error)) == NULL)) {
        DFErrorFormat(error,"%s: %s",relsPathRel,DFErrorMessage(error));
        goto end;
    }

    parseDocumentRels(documentPath,relsDoc,rels,error);

    if (!processParts(parts,documentPath,relsDoc,rels,output,storage,error))
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

char *Word_toPlain(DFStorage *rawStorage, DFHashTable *parts)
{
    DFError *error = NULL;
    char *result = Word_toPlainFromDir(rawStorage,parts,&error);
    if (result == NULL) {
        result = DFFormatString("%s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
    }
    return result;
}

static int saveXMLDocument(DFStorage *storage, const char *filename, DFDocument *doc, NamespaceID defaultNS, DFError **error)
{
    char *parentPath = DFPathDirName(filename);
    int ok = 0;

    if (!DFSerializeXMLStorage(doc,defaultNS,0,storage,filename,error)) {
        DFErrorFormat(error,"serialize %s: %s",filename,DFErrorMessage(error));
        goto end;
    }

    ok = 1;

end:
    free(parentPath);
    return ok;
}

static int saveStrippedXMLText(DFStorage *storage, const char *filename,
                               const char *input, NamespaceID defaultNS, DFError **error)
{
    DFDocument *doc = DFParseXMLString(input,error);
    if (doc == NULL)
        return 0;
    DFStripWhitespace(doc->docNode);
    int ok = saveXMLDocument(storage,filename,doc,defaultNS,error);
    DFDocumentRelease(doc);
    return ok;
}

typedef struct PartInfo {
    const char *filename;
    const char *path;
    const char *rel;
    const char *type;
} PartInfo;

static int saveContentTypes(DFStorage *storage, DFHashTable *ctDefaults, DFHashTable *ctOverrides, DFError **error)
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

    int ok = saveXMLDocument(storage,"[Content_Types].xml",doc,NAMESPACE_CT,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveDocRels(DFStorage *storage,
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

    int ok = saveXMLDocument(storage,"/word/_rels/document.xml.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int saveRootRels(DFStorage *storage, DFError **error)
{
    DFDocument *doc = DFDocumentNewWithRoot(REL_RELATIONSHIPS);
    DFNode *rel = DFCreateChildElement(doc->root,REL_RELATIONSHIP);
    DFSetAttribute(rel,NULL_Id,"rId1");
    DFSetAttribute(rel,NULL_Type,"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
    DFSetAttribute(rel,NULL_TARGET,"/word/document.xml");
    int ok = saveXMLDocument(storage,"/_rels/.rels",doc,NAMESPACE_REL,error);
    DFDocumentRelease(doc);
    return ok;
}

static int Word_fromStorage(TextPackage *tp, DFStorage *storage, DFError **error)
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
        if (!saveStrippedXMLText(storage,"/word/document.xml",documentStr,NAMESPACE_NULL,error))
            goto end;
    }

    int rIdNext = 1;
    for (int i = 0; parts[i].filename; i++) {
        const char *content = DFHashTableLookup(tp->items,parts[i].filename);
        if (content == NULL)
            continue;

        if (!saveStrippedXMLText(storage,parts[i].path,content,NAMESPACE_NULL,error))
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
            if (!DFBufferWriteToStorage(data,storage,curFilename,error)) {
                DFErrorFormat(error,"%s: %s",curFilename,DFErrorMessage(error));
                fileok = 0;
            }

            DFBufferRelease(data);
            free(parentRel);

            if (!fileok)
                goto end;
        }
    }

    if (!saveContentTypes(storage,ctDefaults,ctOverrides,error)) {
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

    if (!saveDocRels(storage,docRelURIs,docRelTypes,docRelModes,error)) {
        DFErrorFormat(error,"saveDocRels: %s",DFErrorMessage(error));
        goto end;
    }

    if (!saveRootRels(storage,error)) {
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

DFStorage *Word_fromPlain(const char *plain, const char *plainPath, DFError **error)
{
    int ok = 0;
    DFStorage *concreteStorage = NULL;
    TextPackage *textPackage = NULL;

    textPackage = TextPackageNewWithString(plain,plainPath,error);
    if (textPackage == NULL)
        goto end;

    concreteStorage = DFStorageNewMemory(DFFileFormatDocx);

    if (!Word_fromStorage(textPackage,concreteStorage,error)) {
        DFErrorFormat(error,"Word_fromStorage: %s",DFErrorMessage(error));
        printf("%s\n",DFErrorMessage(error));
        goto end;
    }
    
    if (!DFStorageSave(concreteStorage,error))
        goto end;
    
    ok = 1;
    
end:
    TextPackageRelease(textPackage);
    if (ok) {
        return concreteStorage;
    }
    else {
        DFStorageRelease(concreteStorage);
        return NULL;
    }
}

DFStorage *TestCaseOpenPackage(DFError **error)
{
    const char *inputDocx = DFHashTableLookup(utgetdata(),"input.docx");
    if (inputDocx == NULL) {
        DFErrorFormat(error,"input.docx not defined");
        return NULL;
    }

    return Word_fromPlain(inputDocx,utgetpath(),error);
}
