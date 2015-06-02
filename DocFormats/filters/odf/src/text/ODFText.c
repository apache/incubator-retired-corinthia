
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
#include "ODFText.h"
#include "ODFPackage.h"
#include "ODFTextConverter.h"
#include "DFDOM.h"
#include "DFHTML.h"
#include "DFHTMLNormalization.h"
#include "CSS.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DFXMLNames.h"
#include "gbg_test.h"
#include "color.h"

typedef struct {
    ODFTextConverter *conv;
    DFDocument *contentDoc;
    DFHashTable *numIdByHtmlId;
    DFHashTable *htmlIdByNumId;
} ODFPutData;

// I'm not sure what ODFTextConverter ise used here for.  
static void traverseContent(ODFTextConverter *conv, DFNode *odfNode, DFNode *htmlNode)
{
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next) {

        if (odfChild->tag == DOM_TEXT) { // we have some text or a text modfier here.
            // DFNode *check = 
//            DFCreateChildTextNode(htmlNode, odfChild->value);
            printf(YELLOW "DOM_TEXT: %s \n" RESET,
                   odfChild->value
                   );

            // print_node_info(check);
            // print_line(1);          
        }
        else {
            Tag newTag = find_HTML(odfChild, htmlNode);
            if (newTag == TAG_NOT_FOUND) {
                // We found a new tag that we need to add to
                // find_HTML(), which reports this to stdout
                // currently.
                ; 
            }
            else if (newTag == TAG_NOT_MATCHED) {  
                // we find tag that we have not managed to match, but
                // that is in find_HTML() already.

                DFCreateChildTextNode(htmlNode, missing_tag_info(odfChild));
                ;
            }
            else if (!newTag) {
                ;  // we added an attribute node already in find_HTML (for now)
                // DFNode *newChild =  DFCreateChildElement(htmlNode, newTag);
            }
            else {
		
                DFNode *node = DFCreateChildElement(htmlNode, HTML_DIV);
		const char * styleName = DFGetAttribute(odfChild,TEXT_STYLE_NAME);
		printf("Found style name %s\n", styleName);
		DFSetAttribute(node, HTML_CLASS, styleName); //DFGetAttribute(odfNode,TEXT_STYLE_NAME));
		for (DFNode *domChild = odfChild->first; domChild != NULL; domChild = domChild->next) 
		{
		  if (domChild->tag == DOM_TEXT) { // we have some text or a text modfier here.
		    // DFNode *check = 
		    DFCreateChildTextNode(node, domChild->value);
		    printf(YELLOW "DOM_TEXT: %s \n" RESET,
		    domChild->value
		    );
		  }
		}
           }
        }
        traverseContent(conv,odfChild,htmlNode);
    }
    // TODO: Add a switch statement here to check the type of ODF element, and use
    // DFCreateChildElement to create a new element in the HTML document as a child of htmlNode
    // based on the type. As this function gets more complicated, it will likely be useful to
    // split it up into several functions
}

DFDocument *ODFTextGet(DFStorage *concreteStorage, DFStorage *abstractStorage, const char *idPrefix, DFError **error)
{
    int ok = 0;
    DFDocument *html = NULL;
    ODFPackage *package = NULL;
    ODFTextConverter *conv = NULL;
    DFNode *body = NULL;

    package = ODFPackageOpenFrom(concreteStorage, error);
    if (package == NULL)
        goto end;

    html = DFDocumentNewWithRoot(HTML_HTML);
    body = DFCreateChildElement(html->root, HTML_BODY);
    DFNode *head = DFChildWithTag(html->root,HTML_HEAD);
    if (head == NULL) {
        head = DFCreateElement(html,HTML_HEAD);
        DFNode *body = DFChildWithTag(html->root,HTML_BODY);
        DFInsertBefore(html->root,head,body);
    }
    conv = ODFTextConverterNew(html, abstractStorage, package, idPrefix);

    printf(RED
           "============================================================\n"
           "Process ODF style nodes prior to the traverseContent function\n"
           "============================================================\n"
           RESET);

    printf(GREEN "Number of style nodes: %lu\n" RESET, (unsigned long)package->stylesDoc->nodesCount);
    show_nodes(package->stylesDoc->root, 0);
    //we want to build up the CSS Stylesheet
    CSSSheet * cssSheet = CSSSheetNew();
    buildCSS_Styles(cssSheet, package->stylesDoc->root);
    
    printf(GREEN "CSS: %s\n" RESET, CSSSheetCopyCSSText(cssSheet));

    
    print_line(2);
    print_line(2);
    print_line(2);

    printf(YELLOW
           "============================================================\n"
           "Showing ODF content nodes prior to the traverseContent function\n"
           "============================================================\n"
           RESET);

    show_nodes(package->contentDoc->root, 0);
    print_line(2);
    print_line(2);
    print_line(2);
    

    // TODO: Traverse the DOM tree of package->contentDoc, adding elements to the HTML document.
    // contentDoc is loaded from content.xml, and represents the most important information in
    // the document, i.e. the text, tables, lists, etc.

    traverseContent(conv, package->contentDoc->root, body);
    char *cssText = CSSSheetCopyCSSText(cssSheet);
    HTMLAddInternalStyleSheet(conv->html, cssText);
    HTML_safeIndent(conv->html->docNode,0);
    // uncomment to see the result. (spammy!)
    printf(GREEN
           "============================================================\n"
           "Showing HTML nodes after the traverseContent function\n"
           "============================================================\n"
           RESET);

    show_nodes(body, 0);


    // TODO: Once this basic traversal is implemented and is capable of producing paragraphs,
    // tables, lists, and spans, add ids to the HTML elements as they are created. That is, set
    // the id attribute of each new HTML element to a string containing the idPrefix followed by
    // the seqNo field of the node in contentDoc from which the HTML element was generated.
    //
    // These id attributes can then later be used in ODFTextPut to figure out which elements in an
    // updated HTML file correspond to existing elements in the ODF content document, which lets us
    // retain information in the ODF file that could not be translated to HTML during the get.
    //
    // See WordConverterCreateAbstract and WordConverterGetConcrete for how this is done in the
    // Word filter.

    ok = 1;

 end:
    ODFPackageRelease(package);
    ODFTextConverterRelease(conv);
    if (!ok) {
        DFDocumentRelease(html);
        return NULL;
    }
    return html;
}

int ODFTextPut(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, const char *idPrefix, DFError **error)
{
    DFErrorFormat(error,"ODFTextPut: Not yet implemented");
    return 0;
}

int ODFTextCreate(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, DFError **error)
{
    DFErrorFormat(error,"ODFTextCreate: Not yet implemented");
    return 0;
}


