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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DFXMLNames.h"

typedef struct {
    int ODF_EnumDFXMLNameKey;
    int HTML_EnumDFXMLNameKey;
} ODF_to_HTML_key;

ODF_to_HTML_key ODF_to_HTML_keys [] = {
    { 0,0 },
};

// tags we already identified
char *tagSeen = " ";;

// probably use a hash table here.  But I've not gotten to that in the
// Skiena book yet... ;-)
//
// Is this worse than switch()?
//
// Not really, if it's irregular(which it is, otherwise I could do a
// straight mapping), I only have to beat the O(binary search) of
// switch() and this list is so short it will be negible anyway.
//
// Also: upside: we get a nice error and omission report out of it.
//
static int find_HTML_from_ODF_key(DFNode *odfNode)
{
    // subtract the offset of 10 in the enum defined in DFXMLNames.h
    int index = (int)odfNode->tag - 10;
    
    int missing_tag = 0;
    char *name;

    if (index > -1) {
        for (int i = 0; ODF_to_HTML_keys[i].ODF_EnumDFXMLNameKey; i++)
            if (ODF_to_HTML_keys[i].ODF_EnumDFXMLNameKey == index)
                return ODF_to_HTML_keys[i].HTML_EnumDFXMLNameKey;

        // we didn't find anything, so it's addition or error alert time.
        name = translateXMLEnumName[index];
        missing_tag = 1;
    }
    else {
        name = xstrdup(DFNodeName(odfNode));
        missing_tag = 2;
    }
    if (!tagSeen || !strstr(tagSeen, name)) {
        int len = strlen(tagSeen)+strlen(name)+2;
        char *newTagSeen = malloc(len);
        snprintf(newTagSeen, len,"%s%s",tagSeen,name);
        tagSeen = xstrdup(newTagSeen);
        free(newTagSeen);
        
        if (missing_tag == 1) {
            printf("Missing: Please add this to struct ODF_to_HTML_keys in ODFTextC.c: ");
            printf("{ %s,\"Add HTML key here\" },\n",name);
        }
        else  {
            printf("Error: No entry found in DFXMLNames: DFNodeName = %s Tag = %d\n",
                   name, odfNode->tag);
        }
    }
}

static void traverseContent(ODFTextConverter *conv, DFNode *odfNode, DFNode *htmlNode)
{
    /* printf("ODF element: %s  tag: %d  emun name = %s\n", */
    /*        DFNodeName(odfNode), odfNode->tag, translateXMLEnumName[odfNode->tag-10]); */
    find_HTML_from_ODF_key(odfNode);
    //    printf("\ntag missing = %s\n",tagSeen);
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next)
        {
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

    package = ODFPackageOpenFrom(concreteStorage,error);
    if (package == NULL)
        goto end;

    html = DFDocumentNewWithRoot(HTML_HTML);
    body = DFCreateChildElement(html->root,HTML_BODY);
    conv = ODFTextConverterNew(html,abstractStorage,package,idPrefix);

    // TODO: Traverse the DOM tree of package->contentDoc, adding elements to the HTML document.
    // contentDoc is loaded from content.xml, and represents the most important information in
    // the document, i.e. the text, tables, lists, etc.
    traverseContent(conv,package->contentDoc->root,body);
    
    free(tagSeen);
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

