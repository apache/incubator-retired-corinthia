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

// This file comes from the portion of the UX Write editor that
// works on both Apple platforms (that is, it can run on either
// OS X or iOS). It's in the repository for illustrative purposes
// only, to assist with the creation of the framework for the
// Corinthia editor UI. The code does not compile independently in
// its present form.

#import "EDDocumentSetup.h"
#import "EDUtil.h"
#import <FileClient/FileClient.h>

#define L10NDocStringPreface                         NSLocalizedString(@"DocStringPreface",NULL)
#define L10NDocStringRef                             NSLocalizedString(@"DocStringRef",NULL)
#define L10NDocStringAbstract                        NSLocalizedString(@"DocStringAbstract",NULL)
#define L10NDocStringBib                             NSLocalizedString(@"DocStringBib",NULL)
#define L10NDocStringChapter                         NSLocalizedString(@"DocStringChapter",NULL)
#define L10NDocStringAppendix                        NSLocalizedString(@"DocStringAppendix",NULL)
#define L10NDocStringContents                        NSLocalizedString(@"DocStringContents",NULL)
#define L10NDocStringListFigure                      NSLocalizedString(@"DocStringListFigure",NULL)
#define L10NDocStringListTable                       NSLocalizedString(@"DocStringListTable",NULL)
#define L10NDocStringIndex                           NSLocalizedString(@"DocStringIndex",NULL)
#define L10NDocStringFigure                          NSLocalizedString(@"DocStringFigure",NULL)
#define L10NDocStringTable                           NSLocalizedString(@"DocStringTable",NULL)
#define L10NDocStringPart                            NSLocalizedString(@"DocStringPart",NULL)
#define L10NDocStringEncl                            NSLocalizedString(@"DocStringEncl",NULL)
#define L10NDocStringCc                              NSLocalizedString(@"DocStringCc",NULL)
#define L10NDocStringHeadto                          NSLocalizedString(@"DocStringHeadto",NULL)
#define L10NDocStringPage                            NSLocalizedString(@"DocStringPage",NULL)
#define L10NDocStringSee                             NSLocalizedString(@"DocStringSee",NULL)
#define L10NDocStringAlso                            NSLocalizedString(@"DocStringAlso",NULL)
#define L10NDocStringProof                           NSLocalizedString(@"DocStringProof",NULL)
#define L10NDocStringGlossary                        NSLocalizedString(@"DocStringGlossary",NULL)

// FIXME: This won't work now for Word documents, where styles always have class names
// FIXME: Not covered by tests
void CSSSheetUseCSSNumbering(CSSSheet *sheet)
{
    CSSStyle *style;

    style = CSSSheetLookupElement(sheet,"body",NULL,1,0);
    CSSPut(CSSStyleRule(style),"counter-reset","h1 h2 h3 h4 h5 h6 figure table");

    // Figure caption
    style = CSSSheetLookupElement(sheet,"figcaption",NULL,1,1);
    CSSPut(CSSStyleRule(style),"counter-increment","figure");
    NSString *content = [NSString stringWithFormat: @"\"%@ \" counter(figure) \": \"", L10NDocStringFigure];
    CSSPut(CSSStyleBefore(style),"content",content.UTF8String);

    style = CSSSheetLookupElement(sheet,"figcaption","Unnumbered",1,1);
    CSSPut(CSSStyleRule(style),"counter-increment","figure 0");
    CSSPut(CSSStyleBefore(style),"content","\"\"");

    // Table caption
    style = CSSSheetLookupElement(sheet,"caption",NULL,1,1);
    CSSPut(CSSStyleRule(style),"caption-side","bottom");
    CSSPut(CSSStyleRule(style),"counter-increment","table");
    content = [NSString stringWithFormat: @"\"%@ \" counter(table) \": \"", L10NDocStringTable];
    CSSPut(CSSStyleBefore(style),"content",content.UTF8String);

    style = CSSSheetLookupElement(sheet,"caption","Unnumbered",1,1);
    CSSPut(CSSStyleRule(style),"counter-increment","table 0");
    CSSPut(CSSStyleBefore(style),"content","\"\"");

    // Table of contents
    style = CSSSheetLookupElement(sheet,"nav","tableofcontents",1,1);
    if (CSSGet(CSSStyleBefore(style),"content") == NULL) {
        content = [NSString stringWithFormat: @"\"%@\"", L10NDocStringContents];
        CSSPut(CSSStyleBefore(style),"content",content.UTF8String);
        CSSPut(CSSStyleBefore(style),"font-size","2em");
        CSSSetBold(CSSStyleBefore(style),1);
        CSSPut(CSSStyleBefore(style),"margin-top",".67em");
        CSSPut(CSSStyleBefore(style),"margin-bottom",".67em");
        CSSPut(CSSStyleBefore(style),"display","block");
    }

    // List of figures
    style = CSSSheetLookupElement(sheet,"nav","listoffigures",1,1);
    if (CSSGet(CSSStyleBefore(style),"content") == NULL) {
        content = [NSString stringWithFormat: @"\"%@\"", L10NDocStringListFigure];
        CSSPut(CSSStyleBefore(style),"content",content.UTF8String);
        CSSPut(CSSStyleBefore(style),"font-size","2em");
        CSSSetBold(CSSStyleBefore(style),1);
        CSSPut(CSSStyleBefore(style),"margin-top",".67em");
        CSSPut(CSSStyleBefore(style),"margin-bottom",".67em");
        CSSPut(CSSStyleBefore(style),"display","block");
    }

    // List of tables
    style = CSSSheetLookupElement(sheet,"nav","listoftables",1,1);
    if (CSSGet(CSSStyleBefore(style),"content") == NULL) {
        content = [NSString stringWithFormat: @"\"%@\"", L10NDocStringListTable];
        CSSPut(CSSStyleBefore(style),"content",content.UTF8String);
        CSSPut(CSSStyleBefore(style),"font-size","2em");
        CSSSetBold(CSSStyleBefore(style),1);
        CSSPut(CSSStyleBefore(style),"margin-top",".67em");
        CSSPut(CSSStyleBefore(style),"margin-bottom",".67em");
        CSSPut(CSSStyleBefore(style),"display","block");
    }
}

CSSSheet *CSSSheetCreateDefault(void)
{
    CSSSheet *styleSheet = CSSSheetNew();
    CSSSheetUseCSSNumbering(styleSheet);

    CSSStyle *title = CSSSheetLookupElement(styleSheet,"p","Title",1,0);
    CSSPut(CSSStyleRule(title),"font-size","24pt");
    CSSPut(CSSStyleRule(title),"text-align","center");

    CSSStyle *author = CSSSheetLookupElement(styleSheet,"p","Author",1,0);
    CSSPut(CSSStyleRule(author),"font-size","18pt");
    CSSPut(CSSStyleRule(author),"text-align","center");

    CSSStyle *abstract = CSSSheetLookupElement(styleSheet,"p","Abstract",1,0);
    CSSPut(CSSStyleRule(abstract),"font-style","italic");
    CSSPut(CSSStyleRule(abstract),"margin-left","20%");
    CSSPut(CSSStyleRule(abstract),"margin-right","20%");

    CSSStyle *body = CSSSheetLookupElement(styleSheet,"body",NULL,1,0);
    CSSPut(CSSStyleRule(body),"margin-left","10%");
    CSSPut(CSSStyleRule(body),"margin-right","10%");
    CSSPut(CSSStyleRule(body),"margin-top","10%");
    CSSPut(CSSStyleRule(body),"margin-bottom","10%");
    CSSPut(CSSStyleRule(body),"text-align","justify");

    CSSStyle *page = CSSSheetLookupElement(styleSheet,"@page",NULL,1,0);
    CSSPut(CSSStyleRule(page),"size","a4 portrait");
    return styleSheet;
}

DFDocument *DFHTMLCreateDefault(CSSSheet *styleSheet)
{
    DFDocument *doc = DFDocumentNewWithRoot(HTML_HTML);
    DFNode *head = DFCreateChildElement(doc->root,HTML_HEAD);
    DFNode *body = DFCreateChildElement(doc->root,HTML_BODY);
    DFNode *meta = DFCreateChildElement(head,HTML_META);
    DFSetAttribute(meta,HTML_CHARSET,"utf-8");
    DFNode *style = DFCreateChildElement(head,HTML_STYLE);
    char *cssText = CSSSheetCopyCSSText(styleSheet);
    DFCreateChildTextNode(style,cssText);
    free(cssText);
    DFNode *p = DFCreateChildElement(body,HTML_P);
    DFCreateChildElement(p,HTML_BR);
    return doc;
}

int DFHTMLCreateDefaultFile(const char *filename, DFError **error)
{
    CSSSheet *styleSheet = CSSSheetCreateDefault();
    DFDocument *doc = DFHTMLCreateDefault(styleSheet);
    int ok = DFSerializeXMLFile(doc,0,1,filename,error);
    DFDocumentRelease(doc);
    CSSSheetRelease(styleSheet);
    return ok;
}

static void addDefaultStyles(CSSSheet *styleSheet)
{
    // Explicitly add default heading formatting properties
    for (int i = 1; i <= 6; i++) {
        char *elementName = DFFormatString("h%d",i);
        CSSStyle *style = CSSSheetLookupElement(styleSheet,elementName,NULL,1,0);
        CSSStyleAddDefaultHTMLProperties(style);
        free(elementName);
    }

    // Word uses the same "Caption" style for both figures and tables. So we only need to set
    // caption. Note that in HTML the alignment is (I think) implicit.
    CSSStyle *caption = CSSSheetLookupElement(styleSheet,"caption",NULL,1,0);
    CSSPut(CSSStyleRule(caption),"text-align","center");
    caption->latent = 0;
}

int DFWordCreateDefault(const char *filename, DFError **error)
{
    int ok = 0;
    CSSSheet *styleSheet = NULL;
    DFDocument *htmlDoc = NULL;
    DFStorage *abstractStorage = NULL;
    DFConcreteDocument *concreteDoc = NULL;
    DFAbstractDocument *abstractDoc = NULL;

    styleSheet = CSSSheetCreateDefault();
    addDefaultStyles(styleSheet);

    concreteDoc = DFConcreteDocumentCreateFile(filename,error);
    if (concreteDoc == NULL)
        goto end;

    abstractStorage = DFStorageNewMemory(DFFileFormatHTML);
    abstractDoc = DFAbstractDocumentNew(abstractStorage);

    htmlDoc = DFHTMLCreateDefault(styleSheet);
    DFAbstractDocumentSetHTML(abstractDoc,htmlDoc);

    if (!DFCreate(concreteDoc,abstractDoc,error))
        goto end;

    ok = 1;

end:
    CSSSheetRelease(styleSheet);
    DFDocumentRelease(htmlDoc);
    DFStorageRelease(abstractStorage);
    DFConcreteDocumentRelease(concreteDoc);
    DFAbstractDocumentRelease(abstractDoc);
    return ok;
}
