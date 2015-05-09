#include "DFPlatform.h"
#include "ODFText.h"
#include "ODFPackage.h"
#include "ODFTextConverter.h"
#include "DFDOM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DFXMLNames.h"
#include "gbg_test.h"

void printNode(DFNode *node);  // temp func, just for some convenience
/// Helper functions


// it may be that the Tag Attribute is not needed.
typedef struct {
    Tag ODF_KEY;
    Tag HTML_KEY;
    Tag attribute;
    char *attribute_value;
} ODF_to_HTML_key;

#define ENDMARKER 555555
ODF_to_HTML_key ODF_to_HTML_keys [] = {
    { 1, HTML_A, 0, NULL},
    { 1, HTML_ABBR, 0, NULL},
    { 1, HTML_ABOUT, 0, NULL},
    { 1, HTML_ACCEPT, 0, NULL},
    { 1, HTML_ACCEPT_CHARSET, 0, NULL},
    { 1, HTML_ACCESSKEY, 0, NULL},
    { 1, HTML_ACTION, 0, NULL},
    { 1, HTML_ADDRESS, 0, NULL},
    { 1, HTML_ALT, 0, NULL},
    { 1, HTML_AREA, 0, NULL},
    { 1, HTML_ARTICLE, 0, NULL},
    { 1, HTML_ASIDE, 0, NULL},
    { 1, HTML_ASYNC, 0, NULL},
    { 1, HTML_AUDIO, 0, NULL},
    { 1, HTML_AUTOCOMPLETE, 0, NULL},
    { 1, HTML_AUTOFOCUS, 0, NULL},
    { 1, HTML_AUTOPLAY, 0, NULL},
    { 1, HTML_B, 0, NULL},
    { 1, HTML_BASE, 0, NULL},
    { 1, HTML_BDI, 0, NULL},
    { 1, HTML_BDO, 0, NULL},
    { 1, HTML_BLOCKQUOTE, 0, NULL},
    { OFFICE_BODY, HTML_BODY, 0, NULL },
    { 1, HTML_BR, 0, NULL },
    { 1, HTML_BUTTON, 0, NULL},
    { 1, HTML_CANVAS, 0, NULL},
    { 1, HTML_CAPTION, 0, NULL},
    { 1, HTML_CHALLENGE, 0, NULL},
    { 1, HTML_CHARSET, 0, NULL},
    { 1, HTML_CHECKED, 0, NULL},
    { 1, HTML_CITE, 0, NULL},
    { 1, HTML_CLASS, 0, NULL},
    { 1, HTML_CODE, 0, NULL},
    { 1, HTML_COL, 0, NULL},
    { 1, HTML_COLGROUP, 0, NULL},
    { 1, HTML_COLS, 0, NULL},
    { 1, HTML_COLSPAN, 0, NULL},
    { 1, HTML_COMMAND, 0, NULL},
    { 1, HTML_CONTENT, 0, NULL},
    { 1, HTML_CONTENTEDITABLE, 0, NULL},
    { 1, HTML_CONTEXTMENU, 0, NULL},
    { 1, HTML_CONTROLS, 0, NULL},
    { 1, HTML_COORDS, 0, NULL},
    { 1, HTML_CROSSORIGIN, 0, NULL},
    { 1, HTML_DATA, 0, NULL},
    { 1, HTML_DATALIST, 0, NULL},
    { 1, HTML_DATATYPE, 0, NULL},
    { 1, HTML_DATETIME, 0, NULL},
    { 1, HTML_DD, 0, NULL},
    { 1, HTML_DEFAULT, 0, NULL},
    { 1, HTML_DEFER, 0, NULL},
    { 1, HTML_DEL, 0, NULL},
    { 1, HTML_DETAILS, 0, NULL},
    { 1, HTML_DFN, 0, NULL},
    { 1, HTML_DIALOG, 0, NULL},
    { 1, HTML_DIR, 0, NULL},
    { 1, HTML_DIRNAME, 0, NULL},
    { 1, HTML_DISABLED, 0, NULL},
    { 1, HTML_DIV, 0, NULL},
    { 1, HTML_DL, 0, NULL},
    { 1, HTML_DOWNLOAD, 0, NULL},
    { 1, HTML_DRAGGABLE, 0, NULL},
    { 1, HTML_DROPZONE, 0, NULL},
    { 1, HTML_DT, 0, NULL},
    { 1, HTML_EM, 0, NULL},
    { 1, HTML_EMBED, 0, NULL},
    { 1, HTML_EMBED, 0, NULL},
    { 1, HTML_ENCTYPE, 0, NULL},
    { 1, HTML_FIELDSET, 0, NULL},
    { 1, HTML_FIGCAPTION, 0, NULL},
    { 1, HTML_FIGURE, 0, NULL},
    { 1, HTML_FOOTER, 0, NULL},
    { 1, HTML_FOR, 0, NULL},
    { 1, HTML_FORM, 0, NULL},
    { 1, HTML_FORMACTION, 0, NULL},
    { 1, HTML_FORMENCTYPE, 0, NULL},
    { 1, HTML_FORMMETHOD, 0, NULL},
    { 1, HTML_FORMNOVALIDATE, 0, NULL},
    { 1, HTML_FORMTARGET, 0, NULL},
    { TEXT_H, HTML_H1, 2310, "Heading_20_1" },
    { TEXT_H, HTML_H2, 2310, "Heading_20_2" },
    { TEXT_H, HTML_H3, 2310, "Heading_20_3" },
    { TEXT_H, HTML_H4, 2310, "Heading_20_4" },
    { TEXT_H, HTML_H5, 2310, "Heading_20_5" },
    { TEXT_H, HTML_H6, 2310, "Heading_20_6" },
    { TEXT_H, HTML_H6, 2310, "Heading_20_7" },
    { TEXT_H, HTML_H6, 2310, "Heading_20_8" },
    { TEXT_H, HTML_H6, 2310, "Heading_20_9" },
    { TEXT_H, HTML_H6, 2310, "Heading_20_10" },
    { 1, HTML_HEAD, 0, NULL},
    { 1, HTML_HEADER, 0, NULL},
    { 1, HTML_HEADERS, 0, NULL},
    { 1, HTML_HEIGHT, 0, NULL},
    { 1, HTML_HGROUP, 0, NULL},
    { 1, HTML_HIDDEN, 0, NULL},
    { 1, HTML_HIGH, 0, NULL},
    { 1, HTML_HR, 0, NULL},
    { 1, HTML_HREF, 0, NULL},
    { 1, HTML_HREFLANG, 0, NULL},
    { 1, HTML_HTML, 0, NULL},
    { 1, HTML_HTTP_EQUIV, 0, NULL},
    { 1, HTML_I, 0, NULL},
    { 1, HTML_ICON, 0, NULL},
    { 1, HTML_ID, 0, NULL},
    { 1, HTML_IFRAME, 0, NULL},
    { 1, HTML_IMG, 0, NULL},
    { 1, HTML_INERT, 0, NULL},
    { 1, HTML_INPUT, 0, NULL},
    { 1, HTML_INPUTMODE, 0, NULL},
    { 1, HTML_INS, 0, NULL},
    { 1, HTML_ISMAP, 0, NULL},
    { 1, HTML_ITEMID, 0, NULL},
    { 1, HTML_ITEMPROP, 0, NULL},
    { 1, HTML_ITEMREF, 0, NULL},
    { 1, HTML_ITEMSCOPE, 0, NULL},
    { 1, HTML_ITEMTYPE, 0, NULL},
    { 1, HTML_KBD, 0, NULL},
    { 1, HTML_KEYGEN, 0, NULL},
    { 1, HTML_KEYTYPE, 0, NULL},
    { 1, HTML_KIND, 0, NULL},
    { 1, HTML_LABEL, 0, NULL},
    { 1, HTML_LANG, 0, NULL},
    { 1, HTML_LEGEND, 0, NULL},
    { 1, HTML_LI, 0, NULL},
    { 1, HTML_LINK, 0, NULL},
    { 1, HTML_LIST, 0, NULL},
    { 1, HTML_LOOP, 0, NULL},
    { 1, HTML_LOW, 0, NULL},
    { 1, HTML_MANIFEST, 0, NULL},
    { 1, HTML_MAP, 0, NULL},
    { 1, HTML_MARK, 0, NULL},
    { 1, HTML_MAX, 0, NULL},
    { 1, HTML_MAXLENGTH, 0, NULL},
    { 1, HTML_MEDIA, 0, NULL},
    { 1, HTML_MEDIAGROUP, 0, NULL},
    { 1, HTML_MENU, 0, NULL},
    { 1, HTML_META, 0, NULL},
    { 1, HTML_METER, 0, NULL},
    { 1, HTML_METHOD, 0, NULL},
    { 1, HTML_MIN, 0, NULL},
    { 1, HTML_MULTIPLE, 0, NULL},
    { 1, HTML_MUTED, 0, NULL},
    { 1, HTML_NAME, 0, NULL},
    { 1, HTML_NAV, 0, NULL},
    { 1, HTML_NOSCRIPT, 0, NULL},
    { 1, HTML_NOVALIDATE, 0, NULL},
    { 1, HTML_OBJECT, 0, NULL},
    { 1, HTML_OL, 0, NULL},
    { 1, HTML_ONABORT, 0, NULL},
    { 1, HTML_ONAFTERPRINT, 0, NULL},
    { 1, HTML_ONBEFOREPRINT, 0, NULL},
    { 1, HTML_ONBEFOREUNLOAD, 0, NULL},
    { 1, HTML_ONBLUR, 0, NULL},
    { 1, HTML_ONCANCEL, 0, NULL},
    { 1, HTML_ONCANPLAY, 0, NULL},
    { 1, HTML_ONCANPLAYTHROUGH, 0, NULL},
    { 1, HTML_ONCHANGE, 0, NULL},
    { 1, HTML_ONCLICK, 0, NULL},
    { 1, HTML_ONCLOSE, 0, NULL},
    { 1, HTML_ONCONTEXTMENU, 0, NULL},
    { 1, HTML_ONCUECHANGE, 0, NULL},
    { 1, HTML_ONDBLCLICK, 0, NULL},
    { 1, HTML_ONDRAG, 0, NULL},
    { 1, HTML_ONDRAGEND, 0, NULL},
    { 1, HTML_ONDRAGENTER, 0, NULL},
    { 1, HTML_ONDRAGLEAVE, 0, NULL},
    { 1, HTML_ONDRAGOVER, 0, NULL},
    { 1, HTML_ONDRAGSTART, 0, NULL},
    { 1, HTML_ONDROP, 0, NULL},
    { 1, HTML_ONDURATIONCHANGE, 0, NULL},
    { 1, HTML_ONEMPTIED, 0, NULL},
    { 1, HTML_ONENDED, 0, NULL},
    { 1, HTML_ONERROR, 0, NULL},
    { 1, HTML_ONFOCUS, 0, NULL},
    { 1, HTML_ONHASHCHANGE, 0, NULL},
    { 1, HTML_ONINPUT, 0, NULL},
    { 1, HTML_ONINVALID, 0, NULL},
    { 1, HTML_ONKEYDOWN, 0, NULL},
    { 1, HTML_ONKEYPRESS, 0, NULL},
    { 1, HTML_ONKEYUP, 0, NULL},
    { 1, HTML_ONLOAD, 0, NULL},
    { 1, HTML_ONLOADEDDATA, 0, NULL},
    { 1, HTML_ONLOADEDMETADATA, 0, NULL},
    { 1, HTML_ONLOADSTART, 0, NULL},
    { 1, HTML_ONMESSAGE, 0, NULL},
    { 1, HTML_ONMOUSEDOWN, 0, NULL},
    { 1, HTML_ONMOUSEMOVE, 0, NULL},
    { 1, HTML_ONMOUSEOUT, 0, NULL},
    { 1, HTML_ONMOUSEOVER, 0, NULL},
    { 1, HTML_ONMOUSEUP, 0, NULL},
    { 1, HTML_ONMOUSEWHEEL, 0, NULL},
    { 1, HTML_ONOFFLINE, 0, NULL},
    { 1, HTML_ONONLINE, 0, NULL},
    { 1, HTML_ONPAGEHIDE, 0, NULL},
    { 1, HTML_ONPAGESHOW, 0, NULL},
    { 1, HTML_ONPAUSE, 0, NULL},
    { 1, HTML_ONPLAY, 0, NULL},
    { 1, HTML_ONPLAYING, 0, NULL},
    { 1, HTML_ONPOPSTATE, 0, NULL},
    { 1, HTML_ONPROGRESS, 0, NULL},
    { 1, HTML_ONRATECHANGE, 0, NULL},
    { 1, HTML_ONRESET, 0, NULL},
    { 1, HTML_ONRESIZE, 0, NULL},
    { 1, HTML_ONSCROLL, 0, NULL},
    { 1, HTML_ONSEEKED, 0, NULL},
    { 1, HTML_ONSEEKING, 0, NULL},
    { 1, HTML_ONSELECT, 0, NULL},
    { 1, HTML_ONSHOW, 0, NULL},
    { 1, HTML_ONSTALLED, 0, NULL},
    { 1, HTML_ONSTORAGE, 0, NULL},
    { 1, HTML_ONSUBMIT, 0, NULL},
    { 1, HTML_ONSUSPEND, 0, NULL},
    { 1, HTML_ONTIMEUPDATE, 0, NULL},
    { 1, HTML_ONUNLOAD, 0, NULL},
    { 1, HTML_ONVOLUMECHANGE, 0, NULL},
    { 1, HTML_ONWAITING, 0, NULL},
    { 1, HTML_OPEN, 0, NULL},
    { 1, HTML_OPTGROUP, 0, NULL},
    { 1, HTML_OPTIMUM, 0, NULL},
    { 1, HTML_OPTION, 0, NULL},
    { 1, HTML_OUTPUT, 0, NULL},
    { TEXT_P, HTML_P, 0, NULL},
    { 1, HTML_PARAM, 0, NULL},
    { 1, HTML_PATTERN, 0, NULL},
    { 1, HTML_PING, 0, NULL},
    { 1, HTML_PLACEHOLDER, 0, NULL},
    { 1, HTML_POSTER, 0, NULL},
    { 1, HTML_PRE, 0, NULL},
    { 1, HTML_PRELOAD, 0, NULL},
    { 1, HTML_PROGRESS, 0, NULL},
    { 1, HTML_PROPERTY, 0, NULL},
    { 1, HTML_Q, 0, NULL},
    { 1, HTML_RADIOGROUP, 0, NULL},
    { 1, HTML_READONLY, 0, NULL},
    { 1, HTML_REL, 0, NULL},
    { 1, HTML_REQUIRED, 0, NULL},
    { 1, HTML_REVERSED, 0, NULL},
    { 1, HTML_ROWS, 0, NULL},
    { 1, HTML_ROWSPAN, 0, NULL},
    { 1, HTML_RP, 0, NULL},
    { 1, HTML_RT, 0, NULL},
    { 1, HTML_RUBY, 0, NULL},
    { 1, HTML_S, 0, NULL},
    { 1, HTML_SAMP, 0, NULL},
    { 1, HTML_SANDBOX, 0, NULL},
    { 1, HTML_SCOPE, 0, NULL},
    { 1, HTML_SCOPED, 0, NULL},
    { OFFICE_SCRIPT, HTML_SCRIPT, 0, NULL},
    { 1, HTML_SEAMLESS, 0, NULL},
    { 1, HTML_SECTION, 0, NULL},
    { 1, HTML_SELECT, 0, NULL},
    { 1, HTML_SELECTED, 0, NULL},
    { 1, HTML_SHAPE, 0, NULL},
    { 1, HTML_SIZE, 0, NULL},
    { 1, HTML_SIZES, 0, NULL},
    { 1, HTML_SMALL, 0, NULL},
    { 1, HTML_SOURCE, 0, NULL},
    { 1, HTML_SPAN, 0, NULL},
    { 1, HTML_SPELLCHECK, 0, NULL},
    { 1, HTML_SRC, 0, NULL},
    { 1, HTML_SRCDOC, 0, NULL},
    { 1, HTML_SRCLANG, 0, NULL},
    { 1, HTML_START, 0, NULL},
    { 1, HTML_STEP, 0, NULL},
    { 1, HTML_STRONG, 0, NULL},
    { STYLE_STYLE, HTML_STYLE, 0, NULL },
    { 1, HTML_SUB, 0, NULL },
    { 1, HTML_SUMMARY, 0, NULL},
    { 1, HTML_SUP, 0, NULL},
    { 1, HTML_TABINDEX, 0, NULL},
    { 1, HTML_TABLE, 0, NULL},
    { 1, HTML_TARGET, 0, NULL},
    { 1, HTML_TBODY, 0, NULL},
    { 1, HTML_TD, 0, NULL},
    { OFFICE_TEXT, HTML_TEXTAREA, 0, NULL },
    { 1, HTML_TFOOT, 0, NULL },
    { 1, HTML_TH, 0, NULL},
    { 1, HTML_THEAD, 0, NULL},
    { 1, HTML_TIME, 0, NULL},
    { 1, HTML_TITLE, 0, NULL},
    { 1, HTML_TR, 0, NULL},
    { 1, HTML_TRACK, 0, NULL},
    { 1, HTML_TRANSLATE, 0, NULL},
    { 1, HTML_TYPE, 0, NULL},
    { 1, HTML_TYPEMUSTMATCH, 0, NULL},
    { 1, HTML_U, 0, NULL},
    { 1, HTML_UL, 0, NULL},
    { 1, HTML_USEMAP, 0, NULL},
    { 1, HTML_VALUE, 0, NULL},
    { 1, HTML_VAR, 0, NULL},
    { 1, HTML_VIDEO, 0, NULL},
    { 1, HTML_WBR, 0, NULL},
    { 1, HTML_WIDTH, 0, NULL},
    { 1, HTML_WRAP, 0, NULL},
    // known missing item I cannot currently place, may or may not be partnered
    { OFFICE_AUTOMATIC_STYLES, 1, 0, NULL},
    { OFFICE_SCRIPTS, 1, 0, NULL},
    { STYLE_FONT_FACE, 1, 0, NULL},
    { OFFICE_FONT_FACE_DECLS, 1, 0, NULL},
    { STYLE_PARAGRAPH_PROPERTIES, 1, 0, NULL},
    { TEXT_SEQUENCE_DECL, 1, 0, NULL},
    { TEXT_SEQUENCE_DECLS, 1, 0, NULL},
    { OFFICE_DOCUMENT_CONTENT, 1, 0, NULL},
    { 0,ENDMARKER, 0, NULL},
};


void report_tags_found(const char *name, Tag HTML, Tag missing_tag)
{
    if (!REPORT_TAG_FOUND) return;

    if (!missing_tag && REPORT_TAG_FOUND < 2) { 
        printf("Key found:  %s ->  %s \n", name, translateXMLEnumName[HTML]);
        return;
    }

    if (!tagSeen || !strstr(tagSeen, name)) {
        int len = strlen(tagSeen)+strlen(name)+2;
        char *newTagSeen = malloc(len);
        snprintf(newTagSeen, len,"%s%s",tagSeen,name);
        tagSeen = xstrdup(newTagSeen);
        free(newTagSeen);
        
        if (missing_tag == 1) {
            printf("Missing: { %s,\"Add HTML key here\" },\n",name);
        }
        else if (missing_tag == 2) {
            printf("Error: No entry found in DFXMLNames: DFNodeName = %s  Tag: %d\n", name, HTML);
        }
        else if (missing_tag == 3) {
            printf("Attribute missing in %s: %s: \n",translateXMLEnumName[HTML], name);
        }

    }    
}

Tag locate_HTML(DFNode *odfNode)
{
    // subtract the offset of 10 in the enum defined in DFXMLNames.h
    int index = (int)odfNode->tag - 10;
    int attrib_not_found = 0;

    if (index > -1) {
        for (int i = 0; ODF_to_HTML_keys[i].HTML_KEY != ENDMARKER; i++) {
            //printf("Seen: %s\n", translateXMLEnumName[ODF_to_HTML_keys[i].ODF_KEY - 10]);
            if (ODF_to_HTML_keys[i].ODF_KEY - 10 == index) {
                if (ODF_to_HTML_keys[i].attribute_value) {
                    if (strcmp(odfNode->attrs->value, ODF_to_HTML_keys[i].attribute_value)) {
                        attrib_not_found = 1;
                        continue;
                    } else {
                        report_tags_found(translateXMLEnumName[index], ODF_to_HTML_keys[i].HTML_KEY - 10, 0);
                        attrib_not_found = 0;
                        return ODF_to_HTML_keys[i].HTML_KEY;
                    }
                }
                if (attrib_not_found == 1) { // we have attribs, but one is missing
                    report_tags_found(odfNode->attrs->value, ODF_to_HTML_keys[i-1].ODF_KEY - 10, 3);
                    return 0;
                }
            }
        }
        // Valid Tag not found in array
        report_tags_found(translateXMLEnumName[index], 0, 1);
        return 0;
    }
    else {  
        // Tag is not a valid array entry
        report_tags_found(DFNodeName(odfNode), odfNode->tag, 2);
    }
    return 0;
}


void printNode(DFNode *n)
{
    if (n == NULL) return;
    //    printf("Tag = %d Attrcount = %d\t", n->tag, n->attrcount);
    //    printf("seqNo = %zu \t", n->seqNo);
    printf("value = %s \t\t", n->value);
    if (n->attrs) {
        printf("HTML TAG = %d %s  \t", n->attrs->tag,
               translateXMLEnumName[locate_HTML(n)-10]);
        //        printf("attr value = %s \t", n->attrs->value);
    }
    if (n->tag > 2)
        printf("ODFKey = %s ", translateXMLEnumName[n->tag-10]);
    printf("\n");
}
