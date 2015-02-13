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
#include "BDTTests.h"
#include "DFDOM.h"
#include "DFBDT.h"
#include "DFString.h"
#include "DFXML.h"
#include "DFCommon.h"
#include "DFUnitTest.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ItemLens                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ItemLens ItemLens;

struct ItemLens {
    DFDocument *abstractDoc;
    DFDocument *concreteDoc;
};

static ItemLens *ItemLensNew(DFDocument *abstractDoc, DFDocument *concreteDoc)
{
    ItemLens *lens = (ItemLens *)calloc(1,sizeof(ItemLens));
    lens->abstractDoc = DFDocumentRetain(abstractDoc);
    lens->concreteDoc = DFDocumentRetain(concreteDoc);
    return lens;
}

static void ItemLensFree(ItemLens *lens)
{
    DFDocumentRelease(lens->abstractDoc);
    DFDocumentRelease(lens->concreteDoc);
    free(lens);
}

static DFNode *ItemGet(ItemLens *lens, DFNode *concrete)
{
    switch (concrete->tag) {
        case HTML_P: {
            DFNode *li = DFCreateElement(lens->abstractDoc,HTML_H1);
            DFFormatAttribute(li,HTML_ID,"x%u", concrete->seqNo);
            char *value = DFNodeTextToString(concrete);
            DFNode *text = DFCreateTextNode(lens->abstractDoc,value);
            free(value);
            DFAppendChild(li,text);
            return li;
        }
        default:
            return NULL;
    }
}

static int ItemIsVisible(ItemLens *lens, DFNode *concrete)
{
    switch (concrete->tag) {
        case HTML_P:
            return 1;
        default:
            return 0;
    }
}

static DFNode *ItemToConcrete(ItemLens *lens, DFNode *abstract)
{
    DFNode *result = NULL;
    const char *idval = DFGetAttribute(abstract,HTML_ID);
    if ((idval != NULL) && DFStringHasPrefix(idval,"x")) {
        unsigned int seqNo = atoi(&idval[1]);
        result = DFNodeForSeqNo(lens->concreteDoc,seqNo);
        assert(result != NULL);
    }
    if (result == NULL) {
        // FIXME: Not covered by tests
        DFNode *p = DFCreateElement(lens->concreteDoc,HTML_P);
        char *value = DFNodeTextToString(abstract);
        DFNode *text = DFCreateTextNode(lens->concreteDoc,value);
        free(value);
        DFFormatAttribute(abstract,HTML_ID,"x%u",p->seqNo);
        DFAppendChild(p,text);
        result = p;
    }
    return result;
}

DFLens lensItem = {
    .isVisible = (void *)ItemIsVisible,
    .get = (void *)ItemGet,
    .put = NULL,
    .create = NULL,
    .remove = NULL,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            Test code                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TestContainerLens TestContainerLens;

struct TestContainerLens {
    DFDocument *abstractDoc;
    DFDocument *concreteDoc;
    ItemLens *itemLens;
};

static TestContainerLens *TestContainerLensNew(DFDocument *abstractDoc, DFDocument *concreteDoc)
{
    TestContainerLens *lens = (TestContainerLens *)calloc(1,sizeof(TestContainerLens));
    lens->abstractDoc = DFDocumentRetain(abstractDoc);
    lens->concreteDoc = DFDocumentRetain(concreteDoc);
    lens->itemLens = ItemLensNew(abstractDoc,concreteDoc);
    return lens;
}

static void TestContainerLensFree(TestContainerLens *lens)
{
    DFDocumentRelease(lens->abstractDoc);
    DFDocumentRelease(lens->concreteDoc);
    ItemLensFree(lens->itemLens);
    free(lens);
}

DFNode *TestContainerLensToAbstract(TestContainerLens *lens, DFNode *concrete)
{
    DFNode *abstract = DFCreateElement(lens->abstractDoc,HTML_UL);
    BDTContainerGet(lens->itemLens,&lensItem,abstract,concrete);
    return abstract;
}

void TestContainerLensPut(TestContainerLens *lens, DFNode *abstract, DFNode *concrete)
{
    BDTContainerPut(lens->itemLens,&lensItem,abstract,concrete,
                    (DFLookupConcreteFunction)ItemToConcrete);
}

void TestContainerLensRemove(TestContainerLens *lens, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        lensItem.remove(lens->itemLens,child);
}

static DFDocument *createHTMLDoc()
{
    DFDocument *doc = DFDocumentNew();
    DFNode *html = DFCreateElement(doc,HTML_HTML);
    DFNode *body = DFCreateElement(doc,HTML_BODY);
    DFAppendChild(doc->docNode,html);
    DFAppendChild(html,body);
    return doc;
}

static DFDocument *createConcrete()
{
    const char *strings[] = {
        "start1",
        "start2",
        "a",
        "b",
        "mid1",
        "mid2",
        "c",
        "d",
        "mid3",
        "mid4",
        "e",
        "f",
        "end1",
        "end2",
        NULL };

    DFDocument *doc = createHTMLDoc();
    for (int i = 0; strings[i]; i++) {
        const char *str = strings[i];
        DFNode *item;
        if (strlen(str) == 1)
            item = DFCreateElement(doc,HTML_P);
        else
            item = DFCreateElement(doc,HTML_DIV);
        DFNode *body = doc->docNode->last->last;
        DFNode *text = DFCreateTextNode(doc,str);
        DFAppendChild(body,item);
        DFAppendChild(item,text);
    }
    return doc;
}

static void move(DFNode *container, int count, int from, int to)
{
    if ((to >= from) && (to <= from+count))
        return;;

    DFNode *nextSibling = NULL;
    DFArray *move = DFArrayNew(NULL,NULL);
    int index = 0;
    for (DFNode *child = container->first; child != NULL; child = child->next) {
        if (index == to)
            nextSibling = child;
        if ((index >= from) && (index < from + count))
            DFArrayAppend(move,child);
        index++;
    }

    for (size_t i = 0; i < DFArrayCount(move); i++) {
        DFNode *child = DFArrayItemAt(move,i);
        DFInsertBefore(container,child,nextSibling);
    }
    DFArrayRelease(move);
}

static void removeChildren(DFNode *container, int *indices, int count)
{
    int index = 0;
    DFNode *next;
    for (DFNode *child = container->first; child != NULL; child = next) {
        next = child->next;

        int removeThisNode = 0;
        for (int i = 0; i < count; i++) {
            if (indices[i] == index)
                removeThisNode = 1;
        }

        if (removeThisNode)
            DFRemoveNode(child);

        index++;
    }
}

static DFDocument *getAbstractDoc(DFDocument *concreteDoc)
{
    DFDocument *abstractDoc = DFDocumentNew();
    DFNode *html = DFCreateElement(abstractDoc,HTML_HTML);
    DFAppendChild(abstractDoc->docNode,html);

    DFNode *concrete = concreteDoc->docNode->last->last;
    TestContainerLens *containerLens = TestContainerLensNew(abstractDoc,concreteDoc);
    DFNode *abstract = TestContainerLensToAbstract(containerLens,concrete);
    DFAppendChild(html,abstract);
    TestContainerLensFree(containerLens);
    return abstractDoc;
}

static int charIsNewline(int c)
{
    return (c == '\n');
}

static void combinedListing(DFDocument *abstractDoc, DFDocument *concreteDoc, DFBuffer *output)
{
    DFInsertBefore(abstractDoc->docNode,DFCreateTextNode(abstractDoc,""),abstractDoc->docNode->first);
    DFInsertBefore(concreteDoc->docNode,DFCreateTextNode(concreteDoc,""),concreteDoc->docNode->first);
    char *abstractStr = DFSerializeXMLString(abstractDoc,0,1);
    char *concreteStr = DFSerializeXMLString(concreteDoc,0,1);

    const char **abstractLines = DFStringTokenize(abstractStr,charIsNewline);
    const char **concreteLines = DFStringTokenize(concreteStr,charIsNewline);

    size_t abstractCount = DFStringArrayCount(abstractLines);
    size_t concreteCount = DFStringArrayCount(concreteLines);

    size_t absIndex = 0;
    size_t conIndex = 0;
    while ((absIndex < abstractCount) && (conIndex < concreteCount)) {
        const char *absLine = abstractLines[absIndex];
        const char *conLine = concreteLines[conIndex];

        if (DFStringHasSuffix(conLine,"</div>")) {
            DFBufferFormat(output,"%-30s %-40s\n","",conLine);
            conIndex++;
        }
        else {
            DFBufferFormat(output,"%-30s %-40s\n",absLine,conLine);
            absIndex++;
            conIndex++;
        }

    }
    while (absIndex < abstractCount) {
        DFBufferFormat(output,"%s\n",abstractLines[absIndex]);
        absIndex++;
    }
    while (conIndex < concreteCount) {
        DFBufferFormat(output,"%-30s %s\n","",concreteLines[conIndex]);
        conIndex++;
    }
    free(abstractStr);
    free(concreteStr);
    free(abstractLines);
    free(concreteLines);

    DFBufferFormat(output,"%-30s %-40s\n","",""); // to satisfy tests
}

static void BDT_testResult(DFDocument *concreteDoc, DFDocument *abstractDoc, DFBuffer *output)
{
    char *oldAbstractStr = DFSerializeXMLString(abstractDoc,0,1);
    TestContainerLens *containerLens = TestContainerLensNew(abstractDoc,concreteDoc);
    TestContainerLensPut(containerLens,abstractDoc->docNode->first->first,concreteDoc->docNode->first->first);
    TestContainerLensFree(containerLens);

    DFDocument *newAbstractDoc = getAbstractDoc(concreteDoc);
    char *newAbstractStr = DFSerializeXMLString(newAbstractDoc,0,1);
    DFDocumentRelease(newAbstractDoc);

    DFStripIds(abstractDoc->docNode);
    combinedListing(abstractDoc,concreteDoc,output);
    int match = !strcmp(newAbstractStr,oldAbstractStr);
    DFBufferFormat(output,"Match? %s\n",match ? "true" : "false");
    free(oldAbstractStr);
    free(newAbstractStr);
}

void BDT_testMove(int count, int from, int to, DFBuffer *output)
{
    DFDocument *concreteDoc = createConcrete();
    DFDocument *abstractDoc = getAbstractDoc(concreteDoc);
    move(abstractDoc->docNode->first->first,count,from,to);
    BDT_testResult(concreteDoc,abstractDoc,output);
    DFDocumentRelease(concreteDoc);
    DFDocumentRelease(abstractDoc);
}

void BDT_testRemove(int *indices, int count, DFBuffer *output)
{
    DFDocument *concreteDoc = createConcrete();
    DFDocument *abstractDoc = getAbstractDoc(concreteDoc);
    removeChildren(abstractDoc->docNode->first->first,indices,count);
    BDT_testResult(concreteDoc,abstractDoc,output);
    DFDocumentRelease(concreteDoc);
    DFDocumentRelease(abstractDoc);
}

int BDT_Test(int argc, const char **argv)
{
    if (argc < 3) {
        printf("Usage: dfutil -bdt count from to\n");
        return 1;
    }

    int count = atoi(argv[0]);
    int from = atoi(argv[1]);
    int to = atoi(argv[2]);

    DFBuffer *output = DFBufferNew();
    BDT_testMove(count,from,to,output);
    printf("%s",output->data);
    DFBufferRelease(output);
    return 0;
}

static void test_move(void)
{
    if (utgetargc() < 3) {
        DFBufferFormat(utgetoutput(),"move: insufficient arguments");
        return;
    }

    int count = atoi(utgetargv()[0]);
    int from = atoi(utgetargv()[1]);
    int to = atoi(utgetargv()[2]);

    DFBuffer *output = DFBufferNew();
    BDT_testMove(count,from,to,output);
    DFBufferFormat(utgetoutput(),"%s",output->data);
    DFBufferRelease(output);
}

static void test_remove(void)
{
    int *indices = (int *)malloc(utgetargc()*sizeof(int));

    for (int i = 0; i < utgetargc(); i++) {
        int index = atoi(utgetargv()[i]);
        indices[i] = index;
    }

    DFBuffer *output = DFBufferNew();
    BDT_testRemove(indices,utgetargc(),output);
    DFBufferFormat(utgetoutput(),"%s",output->data);
    DFBufferRelease(output);

    free(indices);
}

TestGroup BDTTests = {
    "core.bdt", {
        { "move", DataTest, test_move },
        { "remove", DataTest, test_remove },
        { NULL, PlainTest, NULL }
    }
};
