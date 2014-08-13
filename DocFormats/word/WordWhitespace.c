//
//  WordWhitespace.c
//  DocFormats
//
//  Created by Peter Kelly on 5/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordWhitespace.h"
#include "DFDOM.h"
#include "DFFilesystem.h"
#include "DFClassNames.h"
#include "DFString.h"
#include "DFCommon.h"

static void addNbsps(DFNode *node, int inParagraph, int *havePrecedingSpace)
{
    switch (node->tag) {
        case WORD_P:
            inParagraph = 1;
            *havePrecedingSpace = 1;
            break;
        case DOM_TEXT: {
            if (node->parent->tag != WORD_T)
                break;
            uint32_t *chars = DFUTF8To32(node->value);
            size_t length = DFUTF32Length(chars);
            for (size_t i = 0; i < length; i++) {
                if (chars[i] == ' ') {
                    if (*havePrecedingSpace)
                        chars[i] = DFNbspChar;
                    else
                        *havePrecedingSpace = 1;
                }
                else {
                    *havePrecedingSpace = 0;
                }
            }
            char *value = DFUTF32to8(chars);
            DFSetNodeValue(node,value);
            free(value);
            free(chars);
            break;
        }
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        addNbsps(child,inParagraph,havePrecedingSpace);
}

void WordAddNbsps(DFDocument *doc)
{
    int havePrecedingSpace = 0;
    addNbsps(doc->root,1,&havePrecedingSpace);
}

static void removeNbsps(DFNode *node)
{
    if (node->tag == DOM_TEXT) {
        uint32_t *chars = DFUTF8To32(node->value);
        size_t length = DFUTF32Length(chars);
        for (size_t i = 0; i < length; i++) {
            if (chars[i] == DFNbspChar)
                chars[i] = ' ';
        }
        char *value = DFUTF32to8(chars);
        DFSetNodeValue(node,value);
        free(value);
        free(chars);
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        removeNbsps(child);
}

void WordRemoveNbsps(DFDocument *doc)
{
    removeNbsps(doc->docNode);
}
