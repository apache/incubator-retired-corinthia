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
#include "HTMLToLaTeX.h"
#include "DFHTML.h"
#include "DFTable.h"
#include "DFHTMLTables.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

typedef struct LaTeXConverter LaTeXConverter;

static void containerChildrenToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *parent);

struct LaTeXConverter {
    DFDocument *htmlDoc;
    CSSSheet *styleSheet;
    DFNode *titleNode;
    DFNode *authorNode;
    DFNode *dateNode;
    DFHashTable *packages;
    int headingNumbering;
    int excludeNotes;
};

LaTeXConverter *LaTeXConverterNew(DFDocument *htmlDoc)
{
    LaTeXConverter *conv = (LaTeXConverter *)calloc(1,sizeof(LaTeXConverter));
    conv->htmlDoc = DFDocumentRetain(htmlDoc);
    conv->packages = DFHashTableNew((DFCopyFunction)strdup,free);
    return conv;
}

void LaTeXConverterFree(LaTeXConverter *conv)
{
    DFDocumentRelease(conv->htmlDoc);
    CSSSheetRelease(conv->styleSheet);
    DFHashTableRelease(conv->packages);
    free(conv);
}

const char *environmentForBlockNode(LaTeXConverter *conv, DFNode *node)
{
    switch (node->tag) {
        case HTML_H1:
            return "section";
        case HTML_H2:
            return "subsection";
        case HTML_H3:
            return "subsubsection";
        default:
            return NULL;
    }
}

static int containsNote(DFNode *node)
{
    if (node->tag == HTML_SPAN) {
        const char *className = DFGetAttribute(node,HTML_CLASS);
        if (DFStringEquals(className,"footnote") || DFStringEquals(className,"endnote"))
            return 1;
    }
    for (DFNode *child = node->first; child != NULL; child = child->next) {
        if (containsNote(child))
            return 1;
    }
    return 0;
}

void nodeTextToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *node)
{
    // This string is in UTF-8. Currently we only support ASCII characters in LaTeX documents. So anything
    // outside this range will appear as a '?'.
    char *text8 = DFNodeTextToString(node);
    uint32_t *text = DFUTF8To32(text8);
    size_t len = 0;
    while (text[len] != 0)
        len++;
    for (size_t pos = 0; pos < len; pos++) {
        uint32_t c = text[pos];
        switch (c) {
            case '#':
                DFBufferAppendString(output,"\\#");
                break;
            case '$':
                DFBufferAppendString(output,"\\$");
                break;
            case '%':
                DFBufferAppendString(output,"\\%");
                break;
            case '^':
                DFBufferAppendString(output,"\\^{}");
                break;
            case '&':
                DFBufferAppendString(output,"\\&");
                break;
            case '_':
                DFBufferAppendString(output,"\\_");
                break;
            case '{':
                DFBufferAppendString(output,"\\{");
                break;
            case '}':
                DFBufferAppendString(output,"\\}");
                break;
            case '~':
                DFBufferAppendString(output,"\\-{}");
                break;
            case '\\':
                DFBufferAppendString(output,"\\textbackslash");
                break;
            case '\n':
            case '\t':
            case ' ':
                DFBufferAppendChar(output,' ');
                break;
            default:
                if (c == 0x2014)
                    DFBufferAppendString(output,"---");
                else if (c == 0x2013)
                    DFBufferAppendString(output,"--");
                else if (c == 0x201C)
                    DFBufferAppendString(output,"``");
                else if (c == 0x201D)
                    DFBufferAppendString(output,"''");
                else if (c == 0xA0)
                    DFBufferAppendString(output," ");
                else if (c >= 128) // Highest order bit is set; this is part of a UTF-8 multibyte character
                    DFBufferAppendChar(output,'?');
                else
                    DFBufferAppendChar(output,c);
                break;
        }

    }
    free(text8);
    free(text);
}

static void inlineChildrenToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *parent);

static void inlineToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *node)
{
    switch (node->tag) {
        case DOM_TEXT:
            nodeTextToLaTeX(conv,output,node);
            break;
        case HTML_SPAN: {
            int closingBraces = 0;
            const char *className = DFGetAttribute(node,HTML_CLASS);
            CSSProperties *properties = CSSPropertiesNewWithString(DFGetAttribute(node,HTML_STYLE));

            if (DFStringEqualsCI(className,"footnote")) {
                if (conv->excludeNotes) {
                    CSSPropertiesRelease(properties);
                    return;
                }
                DFBufferFormat(output,"\\footnote{");
                closingBraces++;
            }
            else if (DFStringEqualsCI(className,"endnote")) {
                if (conv->excludeNotes) {
                    CSSPropertiesRelease(properties);
                    return;
                }
                DFBufferFormat(output,"\\endnote{");
                DFHashTableAdd(conv->packages,"endnotes","");
                closingBraces++;
            }

            if (CSSGetBold(properties)) {
                DFBufferFormat(output,"\\textbf{");
                closingBraces++;
            }

            if (CSSGetItalic(properties)) {
                DFBufferFormat(output,"\\emph{");
                closingBraces++;
            }

            if (CSSGetUnderline(properties)) {
                DFBufferFormat(output,"\\uline{");
                DFHashTableAdd(conv->packages,"ulem","normalem");
                closingBraces++;
            }

            inlineChildrenToLaTeX(conv,output,node);

            for (int i = 0; i < closingBraces; i++)
                DFBufferAppendChar(output,'}');

            CSSPropertiesRelease(properties);
            break;
        }
        case HTML_IMG: {

            char *texWidth = NULL;
            char *htmlWidth = NULL;

            const char *style = DFGetAttribute(node,HTML_STYLE);
            if (style != NULL) {
                CSSProperties *properties = CSSPropertiesNewWithString(style);
                htmlWidth = DFStrDup(CSSGet(properties,"width"));
                CSSPropertiesRelease(properties);
            }

            if (htmlWidth == NULL) {
                htmlWidth = DFStrDup(DFGetAttribute(node,HTML_WIDTH));
            }

            if (htmlWidth != NULL) {
                CSSLength length = CSSLengthFromString(htmlWidth);
                if (CSSLengthIsValid(length) && (length.units == UnitsPct)) {
                    char buf[100];
                    texWidth = DFFormatString(" width%s\\columnwidth",DFFormatDouble(buf,100,length.value/100.0));
                }
            }

            if (texWidth == NULL)
                texWidth = strdup("");;

            const char *src = DFGetAttribute(node,HTML_SRC);
            if (src != NULL) {

                // Special case for logo image in user guide
                if (!strcmp(src,"uxwrite-res:///logo.png"))
                    src = "logo.png";

                // We don't yet have the graphicx package working in TeXKit, so instead we have to use the
                // \XeTeXpicfile or \XeTeXpdffile primitives. The ~ characters (discretionary spaces) are necessary
                // to get the image properly centered; for reasons I'm yet to understand, the image appears as
                // left-aligned without these, even though the image is inside a center environment.
                char *extension = DFPathExtension(src);
                char *noPercents = DFRemovePercentEncoding(src);
                char *quoted = DFQuote(noPercents);
                if (!strcasecmp(extension,"pdf"))
                    DFBufferFormat(output,"~{\\XeTeXpdffile %s%s\\relax}~",quoted,texWidth);
                else
                    DFBufferFormat(output,"~{\\XeTeXpicfile %s%s\\relax}~",quoted,texWidth);
                free(extension);
                free(noPercents);
                free(quoted);
            }
            free(texWidth);
            break;
        }
        default:
            inlineChildrenToLaTeX(conv,output,node);
            break;
    }
}

static void inlineChildrenToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *parent)
{
    for (DFNode *child = parent->first; child != NULL; child = child->next)
        inlineToLaTeX(conv,output,child);
}

static void listToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *list)
{
    const char *name = (list->tag == HTML_UL) ? "itemize" : "enumerate";
    DFBufferFormat(output,"\\begin{%s}\n\n",name);
    for (DFNode *li = list->first; li != NULL; li = li->next) {
        if (li->tag == HTML_LI) {
            DFBufferFormat(output,"\\item ");
            containerChildrenToLaTeX(conv,output,li);
        }
    }
    DFBufferFormat(output,"\\end{%s}\n\n",name);
}

static void captionToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *child)
{
    // If the caption contains a footnote or endnote, we need to output its content twice. The first time, we
    // exclude the note; this text will be included in the list of figures/tables. The second time, we include
    // the note; this is what is actually displayed in the main part of the document. If we don't do this,
    // typesetting fails.
    if (containsNote(child)) {
        DFBufferFormat(output,"\\caption[");
        conv->excludeNotes++;
        inlineChildrenToLaTeX(conv,output,child);
        conv->excludeNotes--;
        DFBufferFormat(output,"]{");
        inlineChildrenToLaTeX(conv,output,child);
        DFBufferFormat(output,"}\n");
    }
    else {
        DFBufferFormat(output,"\\caption{");
        inlineChildrenToLaTeX(conv,output,child);
        DFBufferFormat(output,"}\n");
    }
}

static void tableToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *table)
{
    DFNode *caption = DFChildWithTag(table,HTML_CAPTION);
    if (caption != NULL)
        DFRemoveNode(caption);

    // Note: the caption node will still be in memory at this point, as nodes are only deallocated
    // with when the document itself is deallocated

    // If the table caption contains a footnote or endnote, we need to wrap the whole thing in a
    // minipage environment, to avoid problems caused by the way in which footnotes and endnotes
    // are implemented by LaTeX.
    int needMinipage = (caption != NULL) && containsNote(caption);

    DFTable *structure = HTML_tableStructure(table);
    DFBufferFormat(output,"\\begin{table}\n");
    if (needMinipage)
        DFBufferFormat(output,"\\begin{minipage}{\\textwidth}\n");
    DFBufferFormat(output,"\\begin{center}\n");
    DFBufferFormat(output,"\\begin{tabular}{|");
    for (unsigned int row = 0; row < structure->cols; row++)
        DFBufferFormat(output,"l|");
    DFBufferFormat(output,"}\n");

    for (unsigned int row = 0; row < structure->rows; row++) {

        // Add inter-row line(s)
        unsigned int start = 0;
        for (unsigned int col = 0; col <= structure->cols; col++) {
            int omitBorder = 0;
            if (col < structure->cols) {
                DFCell *cell = DFTableGetCell(structure,row,col);
                if ((cell != NULL) && (row > cell->row))
                    omitBorder = 1;
            }

            if ((col == structure->cols) || omitBorder) {
                if (start+1 <= col) {
                    if ((start == 0) && (col == structure->cols)) {
                        DFBufferFormat(output,"\\hline");
                    }
                    else {
                        DFBufferFormat(output,"\\cline{%d-%d}",start+1,col);
                    }
                }
                start = col+1;
            }
        }
        DFBufferFormat(output,"\n");

        // Add cell contents
        for (unsigned int col = 0; col < structure->cols; col++) {
            DFCell *cell = DFTableGetCell(structure,row,col);

            if ((cell != NULL) && (row > cell->row)) {
                if (col + cell->colSpan < structure->cols)
                    DFBufferFormat(output," & ");
            }
            else if ((cell != NULL) && (col == cell->col)) {

                if (cell->colSpan > 1) {
                    DFBufferFormat(output,"\\multicolumn{%d}{|l|}{",cell->colSpan);
                }
                if (cell->rowSpan > 1) {
                    DFHashTableAdd(conv->packages,"multirow","");
                    DFBufferFormat(output,"\\multirow{%d}{*}{",cell->rowSpan);
                }

                if (cell != NULL) {
                    inlineChildrenToLaTeX(conv,output,cell->element);
                }

                if (cell->rowSpan > 1) {
                    DFBufferFormat(output,"}");
                }
                if (cell->colSpan > 1) {
                    DFBufferFormat(output,"}");
                }

                if (col + cell->colSpan < structure->cols)
                    DFBufferFormat(output," & ");

            }
        }

        DFBufferFormat(output," \\\\\n");
    }
    DFBufferFormat(output,"\\hline\n");
    DFBufferFormat(output,"\\end{tabular}\n");
    DFBufferFormat(output,"\\end{center}\n");

    if (caption != NULL)
        captionToLaTeX(conv,output,caption);

    if (needMinipage)
        DFBufferFormat(output,"\\end{minipage}\n");

    DFBufferFormat(output,"\\end{table}\n\n");
    DFTableRelease(structure);
}

static void containerChildrenToLaTeX(LaTeXConverter *conv, DFBuffer *output, DFNode *parent)
{
    for (DFNode *child = parent->first; child != NULL; child = child->next) {

        switch (child->tag) {
            case HTML_H1:
                if (conv->headingNumbering)
                    DFBufferFormat(output,"\\section{");
                else
                    DFBufferFormat(output,"\\section*{");
                inlineChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"}\n\n");
                break;
            case HTML_H2:
                if (conv->headingNumbering)
                    DFBufferFormat(output,"\\subsection{");
                else
                    DFBufferFormat(output,"\\subsection*{");
                inlineChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"}\n\n");
                break;
            case HTML_H3:
                if (conv->headingNumbering)
                    DFBufferFormat(output,"\\subsubsection{");
                else
                    DFBufferFormat(output,"\\subsubsection*{");
                inlineChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"}\n\n");
                break;
            case HTML_H4:
                if (conv->headingNumbering)
                    DFBufferFormat(output,"\\paragraph{");
                else
                    DFBufferFormat(output,"\\paragraph*{");
                inlineChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"}\n\n");
                break;
            case HTML_H5:
                if (conv->headingNumbering)
                    DFBufferFormat(output,"\\subparagraph{");
                else
                    DFBufferFormat(output,"\\subparagraph*{");
                inlineChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"}\n\n");
                break;
            case HTML_P: {
                const char *className = DFGetAttribute(child,HTML_CLASS);
                const char *envName = NULL;
                if (DFStringEquals(className,"abstract"))
                    envName = className;

                if (envName != NULL)
                    DFBufferFormat(output,"\\begin{%s}\n",envName);

                inlineChildrenToLaTeX(conv,output,child);
                DFBufferAppendString(output,"\n");

                if (envName != NULL)
                    DFBufferFormat(output,"\\end{%s}\n",envName);

                if ((output->len > 0) && (output->data[output->len-1] == '\n'))
                    DFBufferAppendString(output,"\n");
                else
                    DFBufferAppendString(output,"\n\n");
                break;
            }
            case HTML_UL:
            case HTML_OL:
                listToLaTeX(conv,output,child);
                break;
            case HTML_FIGURE: {
                // If the figure caption contains a footnote or endnote, we need to wrap the whole thing in a
                // minipage environment, to avoid problems caused by the way in which footnotes and endnotes
                // are implemented by LaTeX.
                int needMinipage = containsNote(child);
                DFBufferFormat(output,"\\begin{figure}\n");
                if (needMinipage)
                    DFBufferFormat(output,"\\begin{minipage}{\\textwidth}\n");
                DFBufferFormat(output,"\\begin{center}\n");
                containerChildrenToLaTeX(conv,output,child);
                DFBufferFormat(output,"\\end{center}\n");
                if (needMinipage)
                    DFBufferFormat(output,"\\end{minipage}\n");
                DFBufferFormat(output,"\\end{figure}\n\n");
                break;
            }
            case HTML_FIGCAPTION:
                captionToLaTeX(conv,output,child);
                break;
            case HTML_TABLE:
                tableToLaTeX(conv,output,child);
                break;
            case HTML_NAV: {
                const char *navClass = DFGetAttribute(child,HTML_CLASS);
                if (DFStringEquals(navClass,"tableofcontents"))
                    DFBufferFormat(output,"\\tableofcontents");
                else if (DFStringEquals(navClass,"listoffigures"))
                    DFBufferFormat(output,"\\listoffigures");
                else if (DFStringEquals(navClass,"listoftables"))
                    DFBufferFormat(output,"\\listoftables");
                else
                    nodeTextToLaTeX(conv,output,child);
                DFBufferAppendString(output,"\n\n");
                break;
            }
            default:
                inlineToLaTeX(conv,output,child);
                DFBufferAppendString(output,"\n\n");
                break;
        }
    }
}

#if 0
static void addGeometry(LaTeXConverter *conv, DFBuffer *output)
{
    CSSProperties *properties = CSSSheetBodyProperties(conv->styleSheet);
    const char *cssMarginLeft = CSSGet(properties,"margin-left");
    const char *cssMarginRight = CSSGet(properties,"margin-right");
    const char *cssMarginTop = CSSGet(properties,"margin-top");
    const char *cssMarginBottom = CSSGet(properties,"margin-bottom");

    CSSLength marginLeft = CSSLengthFromString(cssMarginLeft);
    CSSLength marginRight = CSSLengthFromString(cssMarginRight);
    CSSLength marginTop = CSSLengthFromString(cssMarginTop);
    CSSLength marginBottom = CSSLengthFromString(cssMarginBottom);

    DFBufferFormat(output,"\\usepackage[");

    if (CSSLengthIsValid(marginLeft) && (marginLeft.units == UnitsPct))
        DFBufferFormat(output,",lmargin=%f\\paperwidth",marginLeft.value/100);

    if (CSSLengthIsValid(marginRight) && (marginRight.units == UnitsPct))
        DFBufferFormat(output,",rmargin=%f\\paperwidth",marginRight.value/100);

    if (CSSLengthIsValid(marginTop) && (marginTop.units == UnitsPct))
        DFBufferFormat(output,",tmargin=%f\\paperwidth",marginTop.value/100);

    if (CSSLengthIsValid(marginBottom) && (marginBottom.units == UnitsPct))
        DFBufferFormat(output,",bmargin=%f\\paperwidth",marginBottom.value/100);

    DFBufferFormat(output,"]{geometry}\n");

    DFBufferFormat(output,"\n");
}
#endif

static int makeTitleArg(LaTeXConverter *conv, DFNode *node, DFBuffer *output, const char *name, DFNode **savePtr)
{
    const char *className = DFGetAttribute(node,HTML_CLASS);
    if ((node->tag != HTML_P) || !DFStringEqualsCI(className,name))
        return 0;

    DFBufferFormat(output,"\\%s{",name);
    inlineChildrenToLaTeX(conv,output,node);
    DFBufferFormat(output,"}\n");
    *savePtr = node;
    return 1;
}

static void addTitle(LaTeXConverter *conv, DFBuffer *output)
{
    // The LaTeX \maketitle command is normally used to display the title, author, and date of the document. It
    // requires these to be specified using the \title, \author, and \date macros, respectively. When \maketitle
    // is executed, it displays each of these fields in order, with the appropriate fonts and layout.

    // Here, we look at the first three paragraphs of the document, to see if they specify these fields. If all
    // three are present in order, we exclude those from the normal content, and instead use the macros. If only
    // the title and author are present with no date, we just set an empty date. If only the title is present, we
    // set an empty author and date.

    // Since we want to match the appearance of the HTML document, any content the appears before these fields causes
    // the \maketitle macro to not be used in the output, and we just treat these paragraphs as normal content

    DFNode *body = DFChildWithTag(conv->htmlDoc->root,HTML_BODY);
    if (body == NULL)
        return;;

    DFNode *child = body->first;
    DFNode *next;

    // Title
    while ((child != NULL) && !HTML_isBlockLevelTag(child->tag))
        child = child->next;
    next = child->next;
    if (!makeTitleArg(conv,child,output,"title",&conv->titleNode))
        return;
    DFRemoveNode(child);
    child = next;

    // Author
    while ((child != NULL) && !HTML_isBlockLevelTag(child->tag))
        child = child->next;
    next = child->next;
    if (!makeTitleArg(conv,child,output,"author",&conv->authorNode)) {
        DFBufferAppendString(output,"\\date{}\n");
        return;
    }
    DFRemoveNode(child);
    child = next;

    // Date
    while ((child != NULL) && !HTML_isBlockLevelTag(child->tag))
        child = child->next;
    next = child->next;
    if (!makeTitleArg(conv,child,output,"date",&conv->dateNode)) {
        DFBufferAppendString(output,"\\date{}\n");
        return;
    }
    DFRemoveNode(child);
}

char *HTMLToLaTeX(DFDocument *htmlDoc)
{
    LaTeXConverter *conv = LaTeXConverterNew(htmlDoc);
    conv->styleSheet = CSSSheetNew();

    char *cssText = HTMLCopyCSSText(htmlDoc);
    CSSSheetUpdateFromCSSText(conv->styleSheet,cssText);
    free(cssText);

    conv->headingNumbering = CSSSheetHeadingNumbering(conv->styleSheet);

    DFNode *body = DFChildWithTag(htmlDoc->root,HTML_BODY);

    DFBuffer *makeTitleOutput = DFBufferNew();
    addTitle(conv,makeTitleOutput);


    DFBuffer *documentOutput = DFBufferNew();
    if (body != NULL)
        containerChildrenToLaTeX(conv,documentOutput,body);;

    // We build the preamble *after* the content, as the process of going through the document
    // will produce a list of packages that must be included

    DFBuffer *mainOutput = DFBufferNew();
    DFBufferFormat(mainOutput,"\\documentclass[a4paper,12pt]{article}\n");
    DFBufferFormat(mainOutput,"\n");

    const char **packages = DFHashTableCopyKeys(conv->packages);
    DFSortStringsCaseInsensitive(packages);
    if (packages[0] != NULL) {
        for (int i = 0; packages[i]; i++) {
            const char *options = DFHashTableLookup(conv->packages,packages[i]);
            if (strlen(options) > 0)
                DFBufferFormat(mainOutput,"\\usepackage[%s]{%s}\n",options,packages[i]);
            else
                DFBufferFormat(mainOutput,"\\usepackage{%s}\n",packages[i]);
        }
        DFBufferFormat(mainOutput,"\n");
    }
    free(packages);

//    addGeometry(conv,mainOutput);
    DFBufferFormat(mainOutput,"\\setlength{\\parskip}{\\medskipamount}\n");
    DFBufferFormat(mainOutput,"\\setlength{\\parindent}{0pt}\n");
    if (makeTitleOutput->len > 0)
        DFBufferFormat(mainOutput,"\n%s",makeTitleOutput->data);
    DFBufferFormat(mainOutput,"\n");
    DFBufferFormat(mainOutput,"\\begin{document}\n");
    DFBufferFormat(mainOutput,"\n");

    if (conv->titleNode != NULL) {
        DFBufferFormat(mainOutput,"\\maketitle\n\n");
    }

    DFBufferAppendString(mainOutput,documentOutput->data);

    if (DFHashTableLookup(conv->packages,"endnotes"))
        DFBufferFormat(mainOutput,"\\theendnotes\n\n");

    DFBufferFormat(mainOutput,"\\end{document}\n");

    char *result = strdup(mainOutput->data);
    DFBufferRelease(makeTitleOutput);
    DFBufferRelease(documentOutput);
    DFBufferRelease(mainOutput);
    LaTeXConverterFree(conv);
    return result;
}
