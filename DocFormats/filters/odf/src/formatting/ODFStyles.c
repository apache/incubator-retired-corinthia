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

#include "ODFSheet.h"
#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include "DFPlatform.h"
#include "DFDOM.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <stdio.h>
#include "text/color.c"
#include "text/gbg_test.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ODFStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * There can be text properties and paragraph properties under a style definition
 * and a bunch of others that we are ignoring for the moment
 *
 * A style can have a parent definition. If a parent style is not specified,
 * the default style which has the same family as the current style is used.
 *
 * we need to make sure the correct CSSStyle is used for both
 *
 */
static void styleTextPropsGet(DFNode *textProps, CSSSheet *styleSheet)
{
    const char* styleName = DFGetAttribute(textProps->parent, STYLE_NAME);
    printf(CYAN "Create CSS Properties for %s\n", styleName, RESET);
    //if this is a heading? look for the TEXT_OUTLINE_LEVEL
    const char* outlevel = DFGetAttribute(textProps->parent, STYLE_DEFAULT_OUTLINE_LEVEL);
    CSSStyle* cssStyle = NULL;
    if(outlevel != NULL && outlevel[0] < 55) { //'7'
        char hlevel[4] = "h";
        hlevel[1] = outlevel[0];
        hlevel[2] = 0;

        cssStyle = CSSSheetLookupElement(styleSheet,
                                         hlevel,
                                         styleName,
                                         1,
                                         0);
    } else {
        if(outlevel == NULL) {
            printf("Outlevel was NULL\n");
            cssStyle = CSSSheetLookupElement(styleSheet,
                                             "",
                                             styleName,
                                             1,
                                             0);
        }
        else {
            printf("Outlevel was %x hex\n", outlevel[0]);
            cssStyle = CSSSheetLookupElement(styleSheet,
                                             "div",
                                             styleName,
                                             1,
                                             0);
        }
    }
    if(cssStyle != NULL) {
        //just looking for bolds as a first cut
        for (unsigned int i = 0; i < textProps->attrsCount; i++)
        {
            Tag t = textProps->attrs[i].tag;
            switch(t) {
                case FO_FONT_WEIGHT: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"font-weight",textProps->attrs[i].value);
                    break;
                }
                case FO_FONT_SIZE: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"font-size",textProps->attrs[i].value);
                    break;
                }
                case STYLE_FONT_NAME: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"font-family",textProps->attrs[i].value);
                    break;
                }
                case FO_FONT_STYLE: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"font-style",textProps->attrs[i].value);
                    break;
                }
                case FO_COLOR: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"color",textProps->attrs[i].value);
                    break;
                }
                case STYLE_TEXT_UNDERLINE_STYLE: {
                    CSSProperties * localproperties = CSSStyleRule(cssStyle);
                    CSSPut(localproperties,"text-decoration", "underline");
                    //This becomes browser specific - default to firefox
                    //Chrome need an inner outer and span arund the element
                    CSSPut(localproperties,"text-decoration-style", textProps->attrs[i].value);
                }
                default: {
                    printf(RED "Ignored text properties attrbute %s:%s\n", translateXMLEnumName[t], textProps->attrs[i].value, RESET);
                    break;
                }
            }
        }
    }
}

/**
 * The Style:Style element is the main node for text style inof
 * There are many attributes that do no map into CSSProperties
 * Log/Record those we do map
 * and perhaps more importantly those we do not
 */
static void styleStyleGet(DFNode *styleStyleNode, CSSSheet *styleSheet)
{
    for (DFNode *styleStylChildNode = styleStyleNode->first; styleStylChildNode != NULL; styleStylChildNode = styleStylChildNode->next)
    {
        //switch on its tag
        switch(styleStylChildNode->tag) {
            case STYLE_TEXT_PROPERTIES: {
                //figure out what we have..
                styleTextPropsGet(styleStylChildNode, styleSheet);
                break;
            }
            default: {
                printf(RED "Ignoring Style:Style element %s\n", translateXMLEnumName[styleStylChildNode->tag], RESET);
            }
        }
    }
}

static void OfficeStylesGet(DFNode *officStyles, CSSSheet *styleSheet)
{
    printf(CYAN "Processing office styles\n" RESET);
    for (DFNode *officStylesNode = officStyles->first; officStylesNode != NULL; officStylesNode = officStylesNode->next)
    {
        //switch on its tag
        switch(officStylesNode->tag){
            case STYLE_STYLE: {
                //figure out what we have..
                styleStyleGet(officStylesNode, styleSheet);
            }
            break;
            case STYLE_DEFAULT_STYLE: {
                //not supported yet
            }
            default:
            {
                printf(RED "Ignoring Office:Style element %s\n", translateXMLEnumName[officStylesNode->tag], RESET);
            }
        }
    }
}

ODFStyle *ODFStyleNew()
{
    ODFStyle *style = (ODFStyle *)xcalloc(1,sizeof(ODFStyle));
    style->retainCount = 1;
    return style;
}

ODFStyle *ODFStyleRetain(ODFStyle *style)
{
    if (style != NULL)
        style->retainCount++;
    return style;
}

void ODFStyleRelease(ODFStyle *style)
{
    if ((style == NULL) || (--style->retainCount > 0))
        return;

    free(style->selector);
    free(style);
}

CSSSheet *ODFStylesGet(ODFConverter *converter)
{
    CSSSheet *styleSheet = CSSSheetNew();
    CSSStyle *bodyStyle = CSSSheetLookupElement(styleSheet,"body",NULL,1,0);
    CSSPut(CSSStyleRule(bodyStyle),"counter-reset","h1 h2 h3 h4 h5 h6 figure table");

    printf(RED
    "============================================================\n"
    "Process ODF style nodes prior to the traverseContent function\n"
    "============================================================\n"
    RESET);

    printf(GREEN "Number of style nodes: %lu\n" RESET, (unsigned long)converter->package->stylesDoc->nodesCount);
    //show_nodes(converter->package->stylesDoc->root, 0);

    //walk through the nodes
    // go to the office:styles can we find it?
    //iterate each style:style
    // make a css
    // dip down to get its attributes
    DFNode *odfNode = converter->package->stylesDoc->root;
    printf("buildCSS_Styles\n");
    printf("name = %s\n", translateXMLEnumName[odfNode->tag]);

    DFNode *officStyles = DFChildWithTag(converter->package->stylesDoc->root,OFFICE_STYLES);
    OfficeStylesGet(officStyles, styleSheet);
    return styleSheet;
}
