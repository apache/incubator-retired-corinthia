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
#include "DFHTDocument.h"
#include "DFString.h"
#include "streamio.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                               HTML document processing functions                               //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static ctmbstr getAttrValue(TidyNode node, TidyAttrId attrId)
{
    TidyAttr attr = tidyAttrGetById(node,attrId);
    if (attr == NULL)
        return NULL;
    else
        return tidyAttrValue(attr);
}

static void removeSpecial(TidyDoc doc, TidyNode node)
{
    // We process the children first, so that if there are any nested removable elements (e.g.
    // a selection span inside of an autocorrect span), all levels of nesting are taken care of
    TidyNode next;
    for (TidyNode child = tidyGetChild(node); child != NULL; child = next) {
        next = tidyGetNext(child);
        removeSpecial(doc,child);
    }

    if (!tidyNodeIsText(node)) {
        ctmbstr cssClass = getAttrValue(node,TidyAttr_CLASS);
        if (cssClass != NULL) {
            if (!strcmp(cssClass,"uxwrite-heading-number") ||
                !strcmp(cssClass,"uxwrite-figure-number") ||
                !strcmp(cssClass,"uxwrite-table-number") ||
                !strcmp(cssClass,"uxwrite-autocorrect") ||
                !strcmp(cssClass,"uxwrite-selection") ||
                !strcmp(cssClass,"uxwrite-selection-highlight") ||
                !strcmp(cssClass,"uxwrite-spelling") ||
                !strcmp(cssClass,"uxwrite-match")) {
                tidyDiscardContainer(doc,node);
                return;
            }
        }

        switch (tidyNodeGetId(node)) {
            case TidyTag_META: {
                ctmbstr name = getAttrValue(node,TidyAttr_NAME);
                if ((name != NULL) && !strcasecmp(name,"viewport")) {
                    tidyDiscardElement(doc,node);
                    return;
                }
                break;
            }
            case TidyTag_LINK: {
                // This code removes the special "built-in" stylesheet that UX Write uses for displaying documents.
                // FIXME: change the filename so it uniquely identifies it as being UX Write's own CSS file, not
                // some other one that happens to have the same filename.
                // This code really belongs in UX Write itself, rather than DocFormats
                ctmbstr rel = getAttrValue(node,TidyAttr_REL);
                ctmbstr href = getAttrValue(node,TidyAttr_HREF);
                if ((rel != NULL) && (href != NULL)) {
                    if (DFStringEquals(rel,"stylesheet") && DFStringHasSuffix(href,"/builtin.css")) {
                        tidyDiscardElement(doc,node);
                        return;
                    }
                }
                break;
            }
            case TidyTag_HTML: {
                TidyAttr attr = tidyAttrGetById(node,TidyAttr_STYLE);
                if (attr != NULL)
                    tidyRemoveAttribute(doc,node,attr);
                break;
            }
            default:
                break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          DFHTDocument                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFHTDocument *DFHTDocumentNew()
{
    DFHTDocument *htd = (DFHTDocument *)calloc(1,sizeof(DFHTDocument));
    htd->doc = tidyCreate();
    tidyBufInit(&htd->errbuf);
    tidySetErrorBuffer(htd->doc,&htd->errbuf);
    tidyOptSetInt(htd->doc,TidyIndentContent,TidyAutoState);
    tidyOptSetInt(htd->doc,TidyWrapLen,80);
    tidyOptSetBool(htd->doc,TidyDropEmptyElems,no);
    tidyOptSetBool(htd->doc,TidyMark,no);
    tidyOptSetInt(htd->doc,TidyInCharEncoding,UTF8);
    tidyOptSetInt(htd->doc,TidyOutCharEncoding,UTF8);
    return htd;
}

void DFHTDocumentFree(DFHTDocument *htd)
{
    tidyRelease(htd->doc);
    tidyBufFree(&htd->errbuf);
    free(htd);
}

int DFHTDocumentParseCString(DFHTDocument *htd, const char *str, DFError **error)
{
    TidyBuffer inbuf;
    tidyBufInit(&inbuf);
    tidyBufAttach(&inbuf,(byte*)str,(unsigned int)strlen(str));
    int rc = tidyParseBuffer(htd->doc,&inbuf);
    tidyBufDetach(&inbuf);
    tidyBufFree(&inbuf);

    if (rc >= 0) {
        return 1;
    }
    else {
        DFErrorFormat(error,"Operation failed: error %d\n",-rc);
        return 0;
    }
}

void DFHTDocumentRemoveUXWriteSpecial(DFHTDocument *htd)
{
    removeSpecial(htd->doc,tidyGetRoot(htd->doc));
}
