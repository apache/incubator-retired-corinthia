#include "DFPlatform.h"
#include "ODFText.h"
#include "ODFPackage.h"
#include "ODFTextConverter.h"
#include "DFDOM.h"

#include "CSS.h"
#include "CSSSheet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DFXMLNames.h"
#include "DFNameMap.h"
#include "color.h"
#include "gbg_test.h"

/*
  G: Advice I have still to cover:
  ================================

  printNode:

  The attrs field of the DFNode struct is an array, not a single
  object. It contains attrsCount elements. Accessing it in the way that
  printNode does is incorrect for two reasons:

  1) The fact that attrs is non-NULL doesn’t necessarily mean that the
  node has attributes. It’s possible that it did at one point have
  attributes, but those were all removed. In such a case, attrsCount
  will be zero, but attrs will still be non-NULL. Have a look at the
  implementation of DFGetAttribute, DFSetAttribute, DFRemoveAttribute in
  DFDOM.c to see how attrs, attrsCount, and attrsAlloc are used.

  2) If there are multiple attributes, this will only print the first.

  To go through all the attributes and print out their tags & values,
  you would use a loop:


  You can also use DFGetAttribute if you want to already know the tag
  you are looking for (e.g. TEXT_OUTLINE_LEVEL). This function will
  return NULL if the attribute does not exist, or the attribute value if
  it does.


  > Tag text_h(DFNode *node)
  > {
  >    char *s = node->attrs->value;

Referencing node->attrs->value is incorrect, as you don’t know
whether the node will have any attributes, and it if does, whether the
style name (which is what I assume you’re looking for here) will
happen to be the first one. DFGetAttribute(node,TEXT_STYLE_NAME) is
how you would get this reliably.

G: Does not always happen so :( see TEXT_H entry for a failure
(probably on my part :-)

    // TODO: Once this basic traversal is implemented and is capable of producing paragraphs,
    // tables, lists, and spans, add ids to the HTML elements as they are created. That is, set
    // the id attribute of each new HTML element to a string containing the idPrefix followed by

Q: what is that idPrefix thing and how do I find/generate this?

    // the seqNo field of the node in contentDoc from which the HTML element was generated.

Q: As in: adjust the automatic counter that works currently?  Ok, but
it'll leave holes in the count :(

Note: I still found the translateXMLEnumName array informative, so I
put it back again as a dev aid.

*/

/**
 * Temp function: Collect all the possible translations from ODF to
 * HTML.  Will probably still change shape a lot.
 *
 * Input:    DFNode from ODF document that is currently being examined.
 *
 * Returns: Tag with HTMl value or TAG_NOT_MATCHED, or, writes code to
 * screen that can be added to the switch function.  (see default: )
 */
Tag find_HTML(DFNode *odfNode, DFNode *htmlNode)
{
    // I added a couple of #defines for dev purposes.  
    //
    // TAG_NOT_MATCHED shows me stuff I know about but haven't managed
    // to match up.
    //
    // default sends back TAG_NOT_FOUND and so catches new tags and
    // helpfully prints out code for adding to the switch statement
    // ;-)
    //
    // for some weird reason, c-mode in emacs gives me a funky
    // indentation in switch functions.  I'll fix it once this
    // function settled.

    printf(MAGENTA "find_HTML(): TAG NAME: %s  Value: %d \n" RESET,
           translateXMLEnumName[odfNode->tag],
           odfNode->tag
           );

    switch(odfNode->tag) {

    case TEXT_H : { //headers h1 - h6, h7+ returns as p
        
        const char* s = DFGetAttribute(odfNode,TEXT_STYLE_NAME);

        // Grab the very last digit.  If it's a 0-5 we have a proper
        // header, otherwise, we have heading 6-10.
        int s_val = atoi(&s[strlen(s)-1]) - 1;
        if (s_val >= 0 && s_val < 6)
            return HTML_H1 + s_val;
        else
            return HTML_P;
    }
        // paragraph
    case TEXT_P : { return HTML_P; }

        // lists
    case TEXT_LIST :      { return HTML_LIST; }
    case TEXT_LIST_ITEM : { return HTML_LI; }

        // tables
    case TABLE_TABLE :        { return HTML_TABLE; }
    case TABLE_TABLE_COLUMN : { return HTML_COL; }
    case TABLE_TABLE_ROW :    { return HTML_ROWS; }

    case  TABLE_TABLE_CELL :              { return TAG_NOT_MATCHED; }
    case  STYLE_TABLE_CELL_PROPERTIES :   { return TAG_NOT_MATCHED; }
    case  STYLE_TABLE_COLUMN_PROPERTIES : { return TAG_NOT_MATCHED; }
    case  STYLE_TABLE_PROPERTIES :        { return TAG_NOT_MATCHED; }

    case  STYLE_TEXT_PROPERTIES : {

        // no output:
        // const char* s = DFGetAttribute(odfNode,STYLE_TEXT_PROPERTIES);
        const char* s = DFGetAttribute(odfNode,DOM_TEXT);
        
        for (int i = 0; i < odfNode->attrsCount; i++) {

            if (odfNode->attrs[i].tag == FO_FONT_WEIGHT 
                && !strcmp("bold",odfNode->attrs[i].value)) {          
                DFCreateChildElement(htmlNode, HTML_B);
                return 0;
            }

            if (odfNode->attrs[i].tag == FO_FONT_STYLE 
                && !strcmp("italic",odfNode->attrs[i].value)) {
                DFCreateChildElement(htmlNode, HTML_I);
                return 0; 
            }

            if (odfNode->attrs[i].tag == STYLE_TEXT_UNDERLINE_STYLE) {
                DFCreateChildElement(htmlNode, HTML_U);
                return 0; 
            }

            printf(RED "Failure: " RESET
                   "tag %u tag name %s value %s\n",
                   odfNode->attrs[i].tag,
                   translateXMLEnumName[odfNode->attrs[i].tag],
                   odfNode->attrs[i].value);
        }

        return TAG_NOT_MATCHED;
    }
        // document
    case  OFFICE_AUTOMATIC_STYLES : { return TAG_NOT_MATCHED; }
    case  OFFICE_BODY :             { return TAG_NOT_MATCHED; }
    case  OFFICE_FONT_FACE_DECLS :  { return TAG_NOT_MATCHED; }
    case  OFFICE_SCRIPTS :          { return TAG_NOT_MATCHED; }
    case  OFFICE_TEXT :             { return TAG_NOT_MATCHED; }
        
        // fonts, styles'n'things.
    case  STYLE_FONT_FACE :     { return TAG_NOT_MATCHED; }
    case  STYLE_STYLE :         { return TAG_NOT_MATCHED; }
    case  TEXT_SEQUENCE_DECL :  { return TAG_NOT_MATCHED; }
    case  TEXT_SEQUENCE_DECLS : { return TAG_NOT_MATCHED; }
    
    default: {
        // rm foo.html; make ; ./bin/dfconvert get bin/lists.odt foo.html | sort -u
        // ... because we are too lazy to type.
        printf("case  %s : { return TAG_NOT_MATCHED; }\n",
               translateXMLEnumName[odfNode->tag]);     
        return TAG_NOT_FOUND;
    }
    }
}

/**
 * Dev tool: List all the nodes following the given one.
 */
void show_nodes(DFNode *odfNode, int level)
{
    printf("Level: %d\n",level);
    level++;
    print_node_info(odfNode);
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next) {
	walkChildren(odfChild, level);
    }
}

/**
 * Dev tool: List all the nodes following the given one.
 */
void walkChildren(DFNode *odfNode, int level)
{
    printf("Level: %d\n",level);
    level++;
    print_node_info(odfNode);
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next) {
        walkChildren(odfChild, level);
    }
}

/**
 * Dev tool: List all the nodes below the given one.
 */
/*void show_nodes(DFNode *odfNode, int level)
{
    DFNode *nextNode = odfNode;
    do {
        print_node_info(nextNode);
        nextNode = DFNextNode(nextNode);
    }while(nextNode != NULL);
}*/

//give me the styles document
void buildCSS_Styles(CSSSheet * cssSheet, DFNode *odfNode) 
{
  //walk through the nodes
  // go to the office:styles can we find it?
  //iterate each style:style
  // make a css 
  // dip down to get its attributes
    printf("buildCSS_Styles\n");
    printf("name = %s\n", translateXMLEnumName[odfNode->tag]);
    
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next) {
        if(odfChild->tag == OFFICE_STYLES){
            printf("Processing office styles\n");
            for (DFNode *styleNode = odfChild->first; styleNode != NULL; styleNode = styleNode->next) {
                if(styleNode->tag == STYLE_STYLE) {
                    for (unsigned int i = 0; i < styleNode->attrsCount; i++) {
                        Tag t = styleNode->attrs[i].tag;
                        if(t == STYLE_NAME) {
                            printf("Create CSS Properties for %s\n", styleNode->attrs[i].value);

                            //if this is a heading look for the TEXT_OUTLINE_LEVEL

                            // use the attrbute fetch thing...
                            const char* outlevel = DFGetAttribute(styleNode, STYLE_DEFAULT_OUTLINE_LEVEL);
                            CSSStyle* cssStyle = NULL;
                            if(outlevel != NULL) {
                                char hlevel[4] = "h";
                                hlevel[1] = outlevel[0];
                                hlevel[2] = 0;

                                cssStyle = CSSSheetLookupElement(cssSheet,
                                                                           hlevel,
                                                                           styleNode->attrs[i].value,
                                                                           1,
                                                                           0);
                            } else {
                                cssStyle = CSSSheetLookupElement(cssSheet,
                                                            "div",
                                                            styleNode->attrs[i].value,
                                                            1,
                                                            0);
                            }
                            for (DFNode *styleInfo = styleNode->first; styleInfo != NULL; styleInfo = styleInfo->next) {
                                if(styleInfo->tag == STYLE_TEXT_PROPERTIES) {
                                    //just looking for bolds as a first cut
                                    for (unsigned int i = 0; i < styleInfo->attrsCount; i++) {
                                        Tag t = styleInfo->attrs[i].tag;
                                        switch(t) {
                                            case FO_FONT_WEIGHT: {
                                                CSSProperties * localproperties = CSSStyleRule(cssStyle);
                                                CSSPut(localproperties,"font-weight",styleInfo->attrs[i].value);
                                                break;
                                            }
                                            case FO_FONT_SIZE: {
                                                CSSProperties * localproperties = CSSStyleRule(cssStyle);
                                                CSSPut(localproperties,"font-size",styleInfo->attrs[i].value);
                                                break;
                                            }
                                            case STYLE_FONT_NAME:
                                            {
                                                CSSProperties * localproperties = CSSStyleRule(cssStyle);
                                                CSSPut(localproperties,"font-family",styleInfo->attrs[i].value);
                                                break;
                                            }
                                        }
                                    }
                                }
                                else if(styleInfo->tag == STYLE_PARAGRAPH_PROPERTIES)
                                {
                                    //TBD
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * Dev tool: List everything about a Node that's fit to print..  
 */
void print_node_info(DFNode *n)
{
    if (n == NULL) return;

    if (n->tag) printf("Tag tag: %zu name = %s\n",n->tag, translateXMLEnumName[n->tag]);
    printf("unsigned int seqNo: %d\n",n->seqNo);
    // printf("struct DFDocument *doc: %p\n",n->doc);
    if (n->js)      printf("void *js: %p\n",n->js);
    if (n->changed) printf("int changed: %d\n",n->changed);
    if (n->childrenChanged) printf("int childrenChanged %d\n",n->childrenChanged);
    if (n->seqNoHashNext) printf("DFNode *seqNoHashNext %p\n", n->seqNoHashNext);

    DFNameMap *map = n->doc->map; 
    const TagDecl *td = DFNameMapNameForTag(map,n->tag);
    const NamespaceDecl *ns = DFNameMapNamespaceForID(map,td->namespaceID);

    // Attributes
    if (n->tag >= MIN_ELEMENT_TAG) {
        for (unsigned int i = 0; i < n->attrsCount; i++) {
            Tag t = n->attrs[i].tag;
            const TagDecl *attrDecl = DFNameMapNameForTag(map,t);
            printf("iteration %d === tag: %d  name: %s namespaceID = %d localName = %s\n",
                   i, t, translateXMLEnumName[t], 
                   td->namespaceID, 
                   td->localName);
        }
    }
    if (ns->namespaceURI) printf("namespace URI = %s\n",ns->namespaceURI);
    if (td->localName) printf("local name = %s\n",td->localName);
    if (n->attrsCount) printf(RED "unsigned int attrsCount: %d  \n" RESET,n->attrsCount);
    if (n->attrs) {
        // printf("DFAttribute *attrs: %p ",n->attrs);
        if (n->attrs->tag) printf(", Tag tags: %zu ",n->attrs->tag);
        if (n->attrs->value) printf(", char *value: %s ",n->attrs->value);
        printf(" Tag Name = %s\n", translateXMLEnumName[n->attrs->tag]);
    }

    for (int i = 0; i < n->attrsCount; i++) {
        printf(BLUE "attrsCount %d tag %u value %s\n" RESET,
               i, n->attrs[i].tag,n->attrs[i].value);
    }

    if (n->attrsAlloc) printf("unsigned int attrsAlloc: %d\n", n->attrsAlloc);
    if (n->target) printf("char *target: %s\n", n->target);
    if (n->value) printf("char *value: %s\n", n->value);
}

/**
 * Dev tool: List less info about a node than print_node_info().
 */
char *node_id_info(DFNode *node)
{
    const char *local_name = DFNodeName(node);
    //const char *namespace_URI = DFNodeURI(node);
    const char *enum_name = translateXMLEnumName[node->tag];
    int len = strlen (local_name) + strlen(enum_name) + 30; //strlen( namespace_URI)+30;
    char *r = malloc(len);
    snprintf(r, len, "Tag name = %s DFNodeName: %s",
             // " DFNodeURI: %s", 
             enum_name,
             local_name
             //namespace_URI
             );
    return r;
}

/**
 * Dev tool: List some info about a missing tag, alone with some high
 * lighting for easy viewing spotting.
 */
char *missing_tag_info(DFNode *node)
{
    char *inform = red("Tag not matched: ");
    char *facts  = node_id_info(node);
    int len = strlen(inform)+strlen(facts)+2;
    char *r = malloc(len);
    snprintf(r, len, "%s%s", inform, facts);
    return r;
}

/**
 * Dev tool: A selection of snazzy lines because I watched too many serger vids.
 */
void print_line(int style)
{ 
    if (!style)
        printf("=============================================================\n");
    else if (style == 1)
        printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    else if (style == 2)
        printf("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=\n");
}

