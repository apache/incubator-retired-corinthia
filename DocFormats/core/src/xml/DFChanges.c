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
#include "DFChanges.h"
#include "DFHashTable.h"
#include "DFString.h"
#include "DFNameMap.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static char *fullNameForTag(DFDocument *doc, Tag tag)
{
    const TagDecl *tagDecl = DFNameMapNameForTag(doc->map,tag);
    const NamespaceDecl *nsDecl = DFNameMapNamespaceForID(doc->map,tagDecl->namespaceID);
    if ((nsDecl->prefix == NULL) || (tagDecl->namespaceID == NAMESPACE_HTML))
        return strdup(tagDecl->localName);
    else
        return DFFormatString("%s:%s",nsDecl->prefix,tagDecl->localName);
}

static void nodeDetailToBuffer(DFNode *node, DFBuffer *output)
{
    if (node == NULL) {
        DFBufferAppendString(output,"null");
        return;
    }
    switch (node->tag) {
        case DOM_DOCUMENT:
            DFBufferAppendString(output,"#document");
            break;
        case DOM_TEXT: {
            char *quotedValue = DFQuote(node->value);
            DFBufferAppendString(output,quotedValue);
            free(quotedValue);
            break;
        }
        case DOM_COMMENT:
            DFBufferAppendString(output,"#comment");
            break;
        case DOM_CDATA:
            DFBufferAppendString(output,"#cdata-section");
            break;
        case DOM_PROCESSING_INSTRUCTION:
            DFBufferAppendString(output,node->target);
            break;
        default: {
            char *elementName = fullNameForTag(node->doc,node->tag);
            DFBufferFormat(output,"<%s",elementName);
            for (unsigned int i = 0; i < node->attrsCount; i++) {
                char *attrName = fullNameForTag(node->doc,node->attrs[i].tag);
                const char *attrValue = node->attrs[i].value;
                char *quotedValue = DFQuote(attrValue);
                DFBufferFormat(output," %s=%s",attrName,quotedValue);
                free(quotedValue);
                free(attrName);
            }
            DFBufferFormat(output,">");
            switch (node->tag) {
                case WORD_T:
                case WORD_DELTEXT: {
                    char *content = DFNodeTextToString(node);
                    char *quoted = DFQuote(content);
                    char *sub;
                    if (strlen(quoted) >= 2)
                        sub = DFSubstring(quoted,1,strlen(quoted)-2);
                    else
                        sub = strdup(quoted);
                    DFBufferFormat(output,"%s</%s>",quoted,elementName);
                    free(sub);
                    free(quoted);
                    free(content);
                }
            }
            free(elementName);
            break;
        }
    }
}

static void DFElementsByAttr(DFNode *node, Tag attr, DFHashTable *result)
{
    if (node->tag >= MIN_ELEMENT_TAG) {
        const char *value = DFGetAttribute(node,attr);
        if ((value != NULL) && (DFHashTableLookup(result,value) == NULL))
            DFHashTableAdd(result,value,node);
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        DFElementsByAttr(child,attr,result);
}

static void DFRecordChanges(DFNode *parent1, Tag idAttr, DFHashTable *map)
{
    assert(parent1->tag >= MIN_ELEMENT_TAG);

    // Check for changes in children
    for (DFNode *child1 = parent1->first; child1 != NULL; child1 = child1->next) {
        if (child1->tag >= MIN_ELEMENT_TAG)
            DFRecordChanges(child1,idAttr,map);
    }

    // Check the parent itself - starting with finding out, first of all, whether there
    // is actually a corresponding element in the new document
    const char *idValue = DFGetAttribute(parent1,idAttr);
    DFNode *parent2 = (idValue != NULL) ? DFHashTableLookup(map,idValue) : NULL;
    if (parent2 == NULL) {
        parent1->changed = 1;
        return;
    }

    if (parent1->tag != parent2->tag)
        parent1->changed = 1; // keep going - still want to check the children

    if (!identicalAttributesExcept(parent1,parent2,0))
        parent1->changed = 1;; // keep going - still want to check the children

    DFNode *child1 = parent1->first;
    DFNode *child2 = parent2->first;

    while ((child1 != NULL) && (child2 != NULL)) {

        // Non-element nodes can't be correlated based on their id attribute. So we judge whether
        // they are the same by inspecting their content, assuming the parent has the same set
        // of children.

        if (child1->tag != child2->tag) {
            parent1->changed = 1;
        }
        else {
            switch (child1->tag) {
                case DOM_TEXT:
                case DOM_COMMENT:
                case DOM_CDATA: {
                    assert(child1->value != NULL);
                    assert(child2->value != NULL);
                    if (strcmp(child1->value,child2->value))
                        child1->changed = 1;
                    break;
                }
                case DOM_PROCESSING_INSTRUCTION: {
                    if (strcmp(child1->target,child2->target) ||
                        strcmp(child1->value,child2->value)) {
                        child1->changed = 1;
                    }
                    break;
                }
            }
        }

        child1 = child1->next;
        child2 = child2->next;
    }

    if ((child1 != NULL) && (child2 == NULL))
        parent1->changed = 1;
    if ((child1 == NULL) && (child2 != NULL))
        parent1->changed = 1;
}

static void DFPropagateChanges(DFNode *node)
{
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        DFPropagateChanges(child);
        if (child->changed || child->childrenChanged)
            node->childrenChanged = 1;
    }
}

static void DFChangesToStringRecursive(DFNode *node, int indent, DFBuffer *output)
{
    if (node->childrenChanged)
        DFBufferAppendString(output,"*");
    else
        DFBufferAppendString(output," ");

    if (node->changed)
        DFBufferAppendString(output,"C");
    else
        DFBufferAppendString(output," ");

    DFBufferAppendString(output,"  ");

    for (int i = 0; i < indent; i++)
        DFBufferAppendString(output,"    ");
    nodeDetailToBuffer(node,output);
    DFBufferAppendString(output,"\n");

    for (DFNode *child = node->first; child != NULL; child = child->next)
        DFChangesToStringRecursive(child,indent+1,output);
}

void DFComputeChanges(DFNode *root1, DFNode *root2, Tag idAttr)
{
    DFHashTable *map = DFHashTableNew(NULL,NULL);
    DFElementsByAttr(root2,idAttr,map);
    DFRecordChanges(root1,HTML_ID,map);
    DFPropagateChanges(root1);
    DFHashTableRelease(map);
}

char *DFChangesToString(DFNode *root)
{
    DFBuffer *output = DFBufferNew();
    DFChangesToStringRecursive(root,0,output);
    char *result = strdup(output->data);
    DFBufferRelease(output);
    return result;
}
