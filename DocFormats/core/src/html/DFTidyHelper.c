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
#include "DFTidyHelper.h"
#include "buffio.h"
#include "streamio.h"
#include "DFNameMap.h"
#include "DFCommon.h"

char *copyTidyNodeValue(TidyNode tnode, TidyDoc tdoc)
{
    TidyBuffer buf;
    tidyBufInit(&buf);
    tidyNodeGetValue(tdoc,tnode,&buf);

    char *str = (char *)malloc(buf.size+1);
    memcpy(str,buf.bp,buf.size);
    str[buf.size] = '\0';

    tidyBufFree(&buf);
    return str;
}

DFNode *fromTidyNode(DFDocument *htmlDoc, TidyDoc tdoc, TidyNode tnode)
{
    switch (tidyNodeGetType(tnode)) {
        case TidyNode_Text: {
            char *value = copyTidyNodeValue(tnode,tdoc);
            DFNode *result = DFCreateTextNode(htmlDoc,value);
            free(value);
            return result;
        }
        case TidyNode_CDATA:
            break;
        case TidyNode_Comment:
            break;
        case TidyNode_Root:
            printf("Have root\n");
            break;
        default: {
            const char *name = tidyNodeGetName(tnode);
            if (name == NULL) {
                printf("NULL name for %p, type %d\n",tnode,tidyNodeGetType(tnode));
                return NULL;
            }
            const NamespaceDecl *namespaceDecl = DFNameMapNamespaceForID(htmlDoc->map,NAMESPACE_HTML);
            Tag tag = DFNameMapTagForName(htmlDoc->map,namespaceDecl->namespaceURI,name);
            DFNode *element = DFCreateElement(htmlDoc,tag);

            for (TidyAttr tattr = tidyAttrFirst(tnode); tattr != NULL; tattr = tidyAttrNext(tattr)) {
                const char *name = tidyAttrName(tattr);
                const char *value = tidyAttrValue(tattr);
                if (value == NULL) // Can happen in case of the empty string
                    value = "";;
                Tag attrTag = DFNameMapTagForName(htmlDoc->map,namespaceDecl->namespaceURI,name);
                DFSetAttribute(element,attrTag,value);
            }

            for (TidyNode tchild = tidyGetChild(tnode); tchild != NULL; tchild = tidyGetNext(tchild)) {
                DFNode *child = fromTidyNode(htmlDoc,tdoc,tchild);
                if (child != NULL)
                    DFAppendChild(element,child);
            }
            return element;
        }
    }
    return NULL;
}
