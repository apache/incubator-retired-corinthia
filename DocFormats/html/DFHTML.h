//
//  DFHTML.h
//  DocFormats
//
//  Created by Peter Kelly on 26/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFHTML_h
#define DocFormats_DFHTML_h

#include "DFXMLNames.h"
#include "CSSSheet.h"
#include "DFXMLForward.h"
#include "DFError.h"
#include "CSSLength.h"

typedef struct CSSSize CSSSize;

struct CSSSize {
    CSSLength width;
    CSSLength height;
};

void HTMLAddExternalStyleSheet(DFDocument *doc, const char *href);
void HTMLAddInternalStyleSheet(DFDocument *doc, const char *cssText);
char *HTMLCopyCSSText(DFDocument *doc);

int HTML_isContainerTag(Tag tag);
int HTML_isBlockLevelTag(Tag tag);
int HTML_isParagraphTag(Tag tag);
int HTML_isListTag(Tag tag);
int HTML_requiresCloseTag(Tag tag);
int HTML_isSpecialSpan(DFNode *node);
int HTML_isContentNode(DFNode *node);
int HTML_nodeHasContent(DFNode *node);
int HTML_nodeIsHyperlink(DFNode *node);
void HTML_safeIndent(DFNode *node, int depth);
void HTMLBreakBDTRefs(DFNode *node, const char *idPrefix);
CSSSize HTML_getImageDimensions(DFNode *img);

int isHexChar(unsigned short c);
int isRRGGBB(const char *str);
int isHashRRGGBB(const char *str);

DFDocument *DFParseHTMLString(const char *str, int removeSpecial, DFError **error);
DFDocument *DFParseHTMLFile(const char *filename, int removeSpecial, DFError **error);
const char **DFHTMLGetImages(DFDocument *htmlDoc);

#endif
