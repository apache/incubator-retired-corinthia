
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

/*typedef struct {
    ODFTextConverter *conv;
    DFDocument *contentDoc;
    DFHashTable *numIdByHtmlId;
    DFHashTable *htmlIdByNumId;
} ODFPutData;*/

// I'm not sure what ODFTextConverter ise used here for.  
static void traverseContent(ODFConverter *conv, DFNode *odfNode, DFNode *htmlNode)
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
            }
            else if (!newTag) {
                ;  // we added an attribute node already in find_HTML (for now)
                // DFNode *newChild =  DFCreateChildElement(htmlNode, newTag);
            }
            else {
                //what do we have here a header or a paragraph
                DFNode *node = NULL;
                const char * styleName = DFGetAttribute(odfChild,TEXT_STYLE_NAME);
                const char * outlevel = DFGetAttribute(odfChild,TEXT_OUTLINE_LEVEL);
                if(outlevel != NULL) {
                    int s_val = atoi(&outlevel[strlen(outlevel)-1]) - 1;
                    node = DFCreateChildElement(htmlNode, HTML_H1 + s_val);
                } else {
                    node = DFCreateChildElement(htmlNode, HTML_P);
                }
                printf("Found style name %s\n", styleName);
                DFSetAttribute(node, HTML_CLASS, styleName); //DFGetAttribute(odfNode,TEXT_STYLE_NAME));
                for (DFNode *domChild = odfChild->first; domChild != NULL; domChild = domChild->next) {
                    if (domChild->tag == DOM_TEXT) { // we have some text or a text modfier here.
                        DFCreateChildTextNode(node, domChild->value);
                        printf(YELLOW "DOM_TEXT: %s \n" RESET, domChild->value);
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

DFDocument *ODFTextGet(ODFConverter *converter)
{
/*    print_line(2);
    print_line(2);
    print_line(2);

    printf(RED
           "============================================================\n"
           "Showing ODF content nodes prior to the traverseContent function\n"
           "============================================================\n"
           RESET);

    show_nodes(converter->package->contentDoc->root, 0);
    print_line(2);
    print_line(2);
    print_line(2);


    printf(YELLOW
           "============================================================\n"
           "Showing ODF content nodes prior to the traverseContent function\n"
           "============================================================\n"
           RESET);

    show_nodes(converter->package->contentDoc->root, 0);
    print_line(2);
    print_line(2);
    print_line(2);
*/

    // TODO: Traverse the DOM tree of package->contentDoc, adding elements to the HTML document.
    // contentDoc is loaded from content.xml, and represents the most important information in
    // the document, i.e. the text, tables, lists, etc.

    traverseContent(converter, converter->package->contentDoc->root, converter->body);
    // uncomment to see the result. (spammy!)
/*    printf(GREEN
           "============================================================\n"
           "Showing HTML nodes after the traverseContent function\n"
           "============================================================\n"
           RESET);

    show_nodes(converter->body, 0);
*/
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


