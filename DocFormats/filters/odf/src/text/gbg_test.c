#include "DFPlatform.h"
#include "ODFText.h"
#include "ODFPackage.h"
#include "ODFTextConverter.h"
#include "DFDOM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DFXMLNames.h"
#include "color.h"
#include "gbg_test.h"

#define HTML_VARIOUS 4444444 // one ODF key mapping to various HTML keys
#define ENDMARKER 555555
ODF_to_HTML_key ODF_to_HTML_keys [] = {
    { 1, HTML_A, no_op},
    { 1, HTML_ABBR, no_op},
    { 1, HTML_ABOUT, no_op},
    { 1, HTML_ACCEPT, no_op},
    { 1, HTML_ACCEPT_CHARSET, no_op},
    { 1, HTML_ACCESSKEY, no_op},
    { 1, HTML_ACTION, no_op},
    { 1, HTML_ADDRESS, no_op},
    { 1, HTML_ALT, no_op},
    { 1, HTML_AREA, no_op},
    { 1, HTML_ARTICLE, no_op},
    { 1, HTML_ASIDE, no_op},
    { 1, HTML_ASYNC, no_op},
    { 1, HTML_AUDIO, no_op},
    { 1, HTML_AUTOCOMPLETE, no_op},
    { 1, HTML_AUTOFOCUS, no_op},
    { 1, HTML_AUTOPLAY, no_op},
    { 1, HTML_B, no_op},
    { 1, HTML_BASE, no_op},
    { 1, HTML_BDI, no_op},
    { 1, HTML_BDO, no_op},
    { 1, HTML_BLOCKQUOTE, no_op},
    { OFFICE_BODY, HTML_BODY, no_op },
    { 1, HTML_BR, no_op },
    { 1, HTML_BUTTON, no_op},
    { 1, HTML_CANVAS, no_op},
    { 1, HTML_CAPTION, no_op},
    { 1, HTML_CHALLENGE, no_op},
    { 1, HTML_CHARSET, no_op},
    { 1, HTML_CHECKED, no_op},
    { 1, HTML_CITE, no_op},
    { 1, HTML_CLASS, no_op},
    { 1, HTML_CODE, no_op},
    { 1, HTML_COL, no_op},
    { 1, HTML_COLGROUP, no_op},
    { 1, HTML_COLS, no_op},
    { 1, HTML_COLSPAN, no_op},
    { 1, HTML_COMMAND, no_op},
    { 1, HTML_CONTENT, no_op},
    { 1, HTML_CONTENTEDITABLE, no_op},
    { 1, HTML_CONTEXTMENU, no_op},
    { 1, HTML_CONTROLS, no_op},
    { 1, HTML_COORDS, no_op},
    { 1, HTML_CROSSORIGIN, no_op},
    { 1, HTML_DATA, no_op},
    { 1, HTML_DATALIST, no_op},
    { 1, HTML_DATATYPE, no_op},
    { 1, HTML_DATETIME, no_op},
    { 1, HTML_DD, no_op},
    { 1, HTML_DEFAULT, no_op},
    { 1, HTML_DEFER, no_op},
    { 1, HTML_DEL, no_op},
    { 1, HTML_DETAILS, no_op},
    { 1, HTML_DFN, no_op},
    { 1, HTML_DIALOG, no_op},
    { 1, HTML_DIR, no_op},
    { 1, HTML_DIRNAME, no_op},
    { 1, HTML_DISABLED, no_op},
    { 1, HTML_DIV, no_op},
    { 1, HTML_DL, no_op},
    { 1, HTML_DOWNLOAD, no_op},
    { 1, HTML_DRAGGABLE, no_op},
    { 1, HTML_DROPZONE, no_op},
    { 1, HTML_DT, no_op},
    { 1, HTML_EM, no_op},
    { 1, HTML_EMBED, no_op},
    { 1, HTML_EMBED, no_op},
    { 1, HTML_ENCTYPE, no_op},
    { 1, HTML_FIELDSET, no_op},
    { 1, HTML_FIGCAPTION, no_op},
    { 1, HTML_FIGURE, no_op},
    { 1, HTML_FOOTER, no_op},
    { 1, HTML_FOR, no_op},
    { 1, HTML_FORM, no_op},
    { 1, HTML_FORMACTION, no_op},
    { 1, HTML_FORMENCTYPE, no_op},
    { 1, HTML_FORMMETHOD, no_op},
    { 1, HTML_FORMNOVALIDATE, no_op},
    { 1, HTML_FORMTARGET, no_op},
    { TEXT_H, HTML_VARIOUS, text_h },
    { 1, HTML_HEAD, no_op},
    { 1, HTML_HEADER, no_op},
    { 1, HTML_HEADERS, no_op},
    { 1, HTML_HEIGHT, no_op},
    { 1, HTML_HGROUP, no_op},
    { 1, HTML_HIDDEN, no_op},
    { 1, HTML_HIGH, no_op},
    { 1, HTML_HR, no_op},
    { 1, HTML_HREF, no_op},
    { 1, HTML_HREFLANG, no_op},
    { 1, HTML_HTML, no_op},
    { 1, HTML_HTTP_EQUIV, no_op},
    { 1, HTML_I, no_op},
    { 1, HTML_ICON, no_op},
    { 1, HTML_ID, no_op},
    { 1, HTML_IFRAME, no_op},
    { 1, HTML_IMG, no_op},
    { 1, HTML_INERT, no_op},
    { 1, HTML_INPUT, no_op},
    { 1, HTML_INPUTMODE, no_op},
    { 1, HTML_INS, no_op},
    { 1, HTML_ISMAP, no_op},
    { 1, HTML_ITEMID, no_op},
    { 1, HTML_ITEMPROP, no_op},
    { 1, HTML_ITEMREF, no_op},
    { 1, HTML_ITEMSCOPE, no_op},
    { 1, HTML_ITEMTYPE, no_op},
    { 1, HTML_KBD, no_op},
    { 1, HTML_KEYGEN, no_op},
    { 1, HTML_KEYTYPE, no_op},
    { 1, HTML_KIND, no_op},
    { 1, HTML_LABEL, no_op},
    { 1, HTML_LANG, no_op},
    { 1, HTML_LEGEND, no_op},
    { 1, HTML_LI, no_op},
    { 1, HTML_LINK, no_op},
    { 1, HTML_LIST, no_op},
    { 1, HTML_LOOP, no_op},
    { 1, HTML_LOW, no_op},
    { 1, HTML_MANIFEST, no_op},
    { 1, HTML_MAP, no_op},
    { 1, HTML_MARK, no_op},
    { 1, HTML_MAX, no_op},
    { 1, HTML_MAXLENGTH, no_op},
    { 1, HTML_MEDIA, no_op},
    { 1, HTML_MEDIAGROUP, no_op},
    { 1, HTML_MENU, no_op},
    { 1, HTML_META, no_op},
    { 1, HTML_METER, no_op},
    { 1, HTML_METHOD, no_op},
    { 1, HTML_MIN, no_op},
    { 1, HTML_MULTIPLE, no_op},
    { 1, HTML_MUTED, no_op},
    { 1, HTML_NAME, no_op},
    { 1, HTML_NAV, no_op},
    { 1, HTML_NOSCRIPT, no_op},
    { 1, HTML_NOVALIDATE, no_op},
    { 1, HTML_OBJECT, no_op},
    { 1, HTML_OL, no_op},
    { 1, HTML_ONABORT, no_op},
    { 1, HTML_ONAFTERPRINT, no_op},
    { 1, HTML_ONBEFOREPRINT, no_op},
    { 1, HTML_ONBEFOREUNLOAD, no_op},
    { 1, HTML_ONBLUR, no_op},
    { 1, HTML_ONCANCEL, no_op},
    { 1, HTML_ONCANPLAY, no_op},
    { 1, HTML_ONCANPLAYTHROUGH, no_op},
    { 1, HTML_ONCHANGE, no_op},
    { 1, HTML_ONCLICK, no_op},
    { 1, HTML_ONCLOSE, no_op},
    { 1, HTML_ONCONTEXTMENU, no_op},
    { 1, HTML_ONCUECHANGE, no_op},
    { 1, HTML_ONDBLCLICK, no_op},
    { 1, HTML_ONDRAG, no_op},
    { 1, HTML_ONDRAGEND, no_op},
    { 1, HTML_ONDRAGENTER, no_op},
    { 1, HTML_ONDRAGLEAVE, no_op},
    { 1, HTML_ONDRAGOVER, no_op},
    { 1, HTML_ONDRAGSTART, no_op},
    { 1, HTML_ONDROP, no_op},
    { 1, HTML_ONDURATIONCHANGE, no_op},
    { 1, HTML_ONEMPTIED, no_op},
    { 1, HTML_ONENDED, no_op},
    { 1, HTML_ONERROR, no_op},
    { 1, HTML_ONFOCUS, no_op},
    { 1, HTML_ONHASHCHANGE, no_op},
    { 1, HTML_ONINPUT, no_op},
    { 1, HTML_ONINVALID, no_op},
    { 1, HTML_ONKEYDOWN, no_op},
    { 1, HTML_ONKEYPRESS, no_op},
    { 1, HTML_ONKEYUP, no_op},
    { 1, HTML_ONLOAD, no_op},
    { 1, HTML_ONLOADEDDATA, no_op},
    { 1, HTML_ONLOADEDMETADATA, no_op},
    { 1, HTML_ONLOADSTART, no_op},
    { 1, HTML_ONMESSAGE, no_op},
    { 1, HTML_ONMOUSEDOWN, no_op},
    { 1, HTML_ONMOUSEMOVE, no_op},
    { 1, HTML_ONMOUSEOUT, no_op},
    { 1, HTML_ONMOUSEOVER, no_op},
    { 1, HTML_ONMOUSEUP, no_op},
    { 1, HTML_ONMOUSEWHEEL, no_op},
    { 1, HTML_ONOFFLINE, no_op},
    { 1, HTML_ONONLINE, no_op},
    { 1, HTML_ONPAGEHIDE, no_op},
    { 1, HTML_ONPAGESHOW, no_op},
    { 1, HTML_ONPAUSE, no_op},
    { 1, HTML_ONPLAY, no_op},
    { 1, HTML_ONPLAYING, no_op},
    { 1, HTML_ONPOPSTATE, no_op},
    { 1, HTML_ONPROGRESS, no_op},
    { 1, HTML_ONRATECHANGE, no_op},
    { 1, HTML_ONRESET, no_op},
    { 1, HTML_ONRESIZE, no_op},
    { 1, HTML_ONSCROLL, no_op},
    { 1, HTML_ONSEEKED, no_op},
    { 1, HTML_ONSEEKING, no_op},
    { 1, HTML_ONSELECT, no_op},
    { 1, HTML_ONSHOW, no_op},
    { 1, HTML_ONSTALLED, no_op},
    { 1, HTML_ONSTORAGE, no_op},
    { 1, HTML_ONSUBMIT, no_op},
    { 1, HTML_ONSUSPEND, no_op},
    { 1, HTML_ONTIMEUPDATE, no_op},
    { 1, HTML_ONUNLOAD, no_op},
    { 1, HTML_ONVOLUMECHANGE, no_op},
    { 1, HTML_ONWAITING, no_op},
    { 1, HTML_OPEN, no_op},
    { 1, HTML_OPTGROUP, no_op},
    { 1, HTML_OPTIMUM, no_op},
    { 1, HTML_OPTION, no_op},
    { 1, HTML_OUTPUT, no_op},
    { TEXT_P, HTML_P, no_op},
    { 1, HTML_PARAM, no_op},
    { 1, HTML_PATTERN, no_op},
    { 1, HTML_PING, no_op},
    { 1, HTML_PLACEHOLDER, no_op},
    { 1, HTML_POSTER, no_op},
    { 1, HTML_PRE, no_op},
    { 1, HTML_PRELOAD, no_op},
    { 1, HTML_PROGRESS, no_op},
    { 1, HTML_PROPERTY, no_op},
    { 1, HTML_Q, no_op},
    { 1, HTML_RADIOGROUP, no_op},
    { 1, HTML_READONLY, no_op},
    { 1, HTML_REL, no_op},
    { 1, HTML_REQUIRED, no_op},
    { 1, HTML_REVERSED, no_op},
    { 1, HTML_ROWS, no_op},
    { 1, HTML_ROWSPAN, no_op},
    { 1, HTML_RP, no_op},
    { 1, HTML_RT, no_op},
    { 1, HTML_RUBY, no_op},
    { 1, HTML_S, no_op},
    { 1, HTML_SAMP, no_op},
    { 1, HTML_SANDBOX, no_op},
    { 1, HTML_SCOPE, no_op},
    { 1, HTML_SCOPED, no_op},
    { OFFICE_SCRIPT, HTML_SCRIPT, no_op},
    { 1, HTML_SEAMLESS, no_op},
    { 1, HTML_SECTION, no_op},
    { 1, HTML_SELECT, no_op},
    { 1, HTML_SELECTED, no_op},
    { 1, HTML_SHAPE, no_op},
    { 1, HTML_SIZE, no_op},
    { 1, HTML_SIZES, no_op},
    { 1, HTML_SMALL, no_op},
    { 1, HTML_SOURCE, no_op},
    { 1, HTML_SPAN, no_op},
    { 1, HTML_SPELLCHECK, no_op},
    { 1, HTML_SRC, no_op},
    { 1, HTML_SRCDOC, no_op},
    { 1, HTML_SRCLANG, no_op},
    { 1, HTML_START, no_op},
    { 1, HTML_STEP, no_op},
    { 1, HTML_STRONG, no_op},
    { STYLE_STYLE, HTML_STYLE, no_op },
    { 1, HTML_SUB, no_op },
    { 1, HTML_SUMMARY, no_op},
    { 1, HTML_SUP, no_op},
    { 1, HTML_TABINDEX, no_op},
    { 1, HTML_TABLE, no_op},
    { 1, HTML_TARGET, no_op},
    { 1, HTML_TBODY, no_op},
    { 1, HTML_TD, no_op},
    { OFFICE_TEXT, HTML_TEXTAREA, no_op },
    { 1, HTML_TFOOT, no_op },
    { 1, HTML_TH, no_op},
    { 1, HTML_THEAD, no_op},
    { 1, HTML_TIME, no_op},
    { 1, HTML_TITLE, no_op},
    { 1, HTML_TR, no_op},
    { 1, HTML_TRACK, no_op},
    { 1, HTML_TRANSLATE, no_op},
    { 1, HTML_TYPE, no_op},
    { 1, HTML_TYPEMUSTMATCH, no_op},
    { 1, HTML_U, no_op},
    { 1, HTML_UL, no_op},
    { 1, HTML_USEMAP, no_op},
    { 1, HTML_VALUE, no_op},
    { 1, HTML_VAR, no_op},
    { 1, HTML_VIDEO, no_op},
    { 1, HTML_WBR, no_op},
    { 1, HTML_WIDTH, no_op},
    { 1, HTML_WRAP, no_op},
    { 0,ENDMARKER, no_op},
};

Tag no_op(DFNode *node) 
{
    return 0; 
}

Tag text_h(DFNode *node) 
{
    char *s = node->attrs->value;
    if ((int)s[11] > 55 || strlen(s) == 13)
        return HTML_H6;
    else
        return HTML_H1 + (int)s[11] - 49;
}

Tag locate_HTML(DFNode *odfNode)
{
    int index = (int)odfNode->tag;

    if (index > -1) {
        for (int i = 0; ODF_to_HTML_keys[i].HTML_KEY != ENDMARKER; i++) {
            if (ODF_to_HTML_keys[i].ODF_KEY == index) {
                if (odfNode->attrs)
                    return ODF_to_HTML_keys[i].attribute_function(odfNode); 
                else 
                    return ODF_to_HTML_keys[i].HTML_KEY; 
            }
        }        
    }
    return 0;
}


void show_nodes(DFNode *odfNode)
{
    for (DFNode *odfChild = odfNode->first; odfChild != NULL; odfChild = odfChild->next) {
        printNode(odfChild);
    }
}

void printNode(DFNode *n)
{
    if (n == NULL) return;

    if (n->tag) printf("Tag tag: %zu\n",n->tag);
    printf("unsigned int seqNo: %d\n",n->seqNo);
    // printf("struct DFDocument *doc: %p\n",n->doc);
    if (n->js)      printf("void *js: %p\n",n->js);
    if (n->changed) printf("int changed: %d\n",n->changed);
    if (n->childrenChanged) printf("int childrenChanged %d\n",n->childrenChanged);
    if (n->seqNoHashNext) printf("DFNode *seqNoHashNext %p\n", n->seqNoHashNext);
    if (n->attrs) {
        printf("DFAttribute *attrs: %p ",n->attrs);
        printf(", Tag tags: %zu ",n->attrs->tag);
        printf(", char *value: %s ",n->attrs->value);
        printf("HTML TAG = %d ", n->attrs->tag);
        if (n->tag)
            printf("%s ", translateXMLEnumName[locate_HTML(n)]);
        printf("\n");
    }
    if (n->attrsCount) printf("unsigned int attrsCount: %d\n",n->attrsCount);
    if (n->attrsAlloc) printf("unsigned int attrsAlloc: %d\n", n->attrsAlloc);
    if (n->target) printf("char *target: %s\n", n->target);
    if (n->value) printf("char *value: %s\n", n->value);
    if (n->tag > 2)
        printf("Tag Text = %s\n", translateXMLEnumName[n->tag]);
    printf("================================================================================================\n");
}

char *printMissingTag(Tag tag)
{
    char *s = translateXMLEnumName[tag];
    int len = strlen(s)+14+20;
    char *r = malloc(len);
    snprintf(r, len, "%s Missing tag: %s %s",RED, s, RESET);
    return r;
}
