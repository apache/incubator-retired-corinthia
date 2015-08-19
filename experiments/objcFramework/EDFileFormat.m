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

#import "EDFileFormat.h"
#import "EDEditor.h"
#import "EDHTMLTidy.h"
#import "EDJSInterface.h"
#import "EDTiming.h"
#import "EDStyle.h"
#import "EDDocumentSetup.h"
#import "EDUtil.h"
#import <FileClient/FCError.h>
#import <FileClient/FCUtil.h>

#define L10NStyleNameHeading1                        NSLocalizedString(@"StyleNameHeading1",NULL)
#define L10NStyleNameHeading2                        NSLocalizedString(@"StyleNameHeading2",NULL)
#define L10NStyleNameHeading3                        NSLocalizedString(@"StyleNameHeading3",NULL)
#define L10NStyleNameHeading4                        NSLocalizedString(@"StyleNameHeading4",NULL)
#define L10NStyleNameHeading5                        NSLocalizedString(@"StyleNameHeading5",NULL)
#define L10NStyleNameHeading6                        NSLocalizedString(@"StyleNameHeading6",NULL)
#define L10NStyleNameParagraph                       NSLocalizedString(@"StyleNameParagraph",NULL)
#define L10NStyleNameBlockQuote                      NSLocalizedString(@"StyleNameBlockQuote",NULL)
#define L10NStyleNamePre                             NSLocalizedString(@"StyleNamePre",NULL)
#define L10NStyleNameBody                            NSLocalizedString(@"StyleNameBody",NULL)
#define L10NStyleNameFigure                          NSLocalizedString(@"StyleNameFigure",NULL)
#define L10NStyleNameFigureCaption                   NSLocalizedString(@"StyleNameFigureCaption",NULL)
#define L10NStyleNameTable                           NSLocalizedString(@"StyleNameTable",NULL)
#define L10NStyleNameTableCaption                    NSLocalizedString(@"StyleNameTableCaption",NULL)
#define L10NStyleNameTableOfContents                 NSLocalizedString(@"StyleNameTableOfContents",NULL)
#define L10NStyleNameListOfFigures                   NSLocalizedString(@"StyleNameListOfFigures",NULL)
#define L10NStyleNameListOfTables                    NSLocalizedString(@"StyleNameListOfTables",NULL)

@interface HTMLFormat : EDFileFormat
@end

@interface TextFormat : EDFileFormat
@end

@interface MarkdownFormat : EDFileFormat
@end

@interface LaTeXFormat : EDFileFormat
@end

@interface WordFormat : EDFileFormat
@end

static NSString *HTML_localizedTagName(Tag tag)
{
    switch (tag) {
        case HTML_H1: return L10NStyleNameHeading1;
        case HTML_H2: return L10NStyleNameHeading2;
        case HTML_H3: return L10NStyleNameHeading3;
        case HTML_H4: return L10NStyleNameHeading4;
        case HTML_H5: return L10NStyleNameHeading5;
        case HTML_H6: return L10NStyleNameHeading6;
        case HTML_P: return L10NStyleNameParagraph;
        case HTML_BLOCKQUOTE: return L10NStyleNameBlockQuote;
        case HTML_PRE: return L10NStyleNamePre;
        case HTML_BODY: return L10NStyleNameBody;
        case HTML_FIGURE: return L10NStyleNameFigure;
        case HTML_FIGCAPTION: return L10NStyleNameFigureCaption;
        case HTML_TABLE: return L10NStyleNameTable;
        case HTML_CAPTION: return L10NStyleNameTableCaption;
        default: return nil;
    }
}

NSString *HTML_uiNameForSelector(const char *ident)
{
    NSString *rawSelector = NSStringFromC(ident);
    if ([rawSelector isEqualToString: @"nav.tableofcontents"])
        return L10NStyleNameTableOfContents;
    if ([rawSelector isEqualToString: @"nav.listoffigures"])
        return L10NStyleNameListOfFigures;
    if ([rawSelector isEqualToString: @"nav.listoftables"])
        return L10NStyleNameListOfTables;

    NSString *elementUIName = HTML_localizedTagName(CSSSelectorGetTag(ident));
    if (elementUIName == nil)
        return nil;

    NSString *result = NULL;
    char *className = CSSSelectorCopyClassName(ident);

    if (!CSSSelectorHasClassName(ident))
        result = elementUIName;
    else if (CSSSelectorGetTag(ident) == HTML_P)
        result = NSStringFromC(className);
    else
        result = [NSString stringWithFormat: @"%@ (%@)", elementUIName, NSStringFromC(className)];

    free(className);
    return result;
}

@interface NSOutputStream(CompleteWrite)

- (int)completeWrite:(const uint8_t *)buffer length:(NSUInteger)length;

@end

@implementation NSOutputStream(CompleteWrite)

- (int)completeWrite:(const uint8_t *)buffer length:(NSUInteger)length
{
    NSUInteger offset = 0;
    do {
        NSUInteger remaining = length - offset;
        NSInteger written = [self write: &buffer[offset] maxLength: remaining];
        if (written <= 0)
            return 0;
        offset += written;
    } while (offset < length);
    return 1;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDFileFormat                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDFileFormat

- (EDFileFormat *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (NSString *)fileLocalPath
{
    return self.editor.delegate.editorPath;
}

- (BOOL)supportsBareStyles
{
    return NO;
}

- (NSString *)absoluteHTMLPath
{
    [NSException raise: @"FileFormat" format: @"%@ absoluteHTMLPath not implemented", self.class];
    return nil;
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    completion([NSError error: @"FileFormat save not implemented"]);
}

- (NSString *)prepareForLoad:(NSError **)error
{
    [NSException raise: @"FileFormat" format: @"%@ prepareForLoad not implemented", self.class];
    return nil;
}

- (void)finishLoad
{
    [NSException raise: @"FileFormat" format: @"%@ finishLoad not implemented", self.class];
}

+ (BOOL)create2:(NSString *)filename error:(DFError **)error
{
    NSString *extension = filename.pathExtension.lowercaseString;
    if ([extension isEqualToString: @"html"] || [extension isEqualToString: @"htm"])
        return DFHTMLCreateDefaultFile(filename.UTF8String,error);
    else if ([extension isEqualToString: @"docx"]) {
        return DFWordCreateDefault(filename.UTF8String,error);
    }

    DFErrorFormat(error,"Unsupported file format: %s\n",extension.UTF8String);
    return NO;
}

+ (BOOL)create:(NSString *)filename error:(NSError **)error
{
    DFError *dferror = NULL;
    BOOL ok = [self create2: filename error: &dferror];
    if (!ok)
        DFErrorReleaseToNSError(dferror,error);
    return ok;
}

+ (EDFileFormat *)formatForExtension:(NSString *)extension
{
    extension = extension.lowercaseString;
    if ([extension isEqualToString: @"html"] || [extension isEqualToString: @"htm"])
        return [[HTMLFormat alloc] init];
    else if ([extension isEqualToString: @"txt"])
        return [[TextFormat alloc] init];
    else if ([extension isEqualToString: @"md"])
        return [[MarkdownFormat alloc] init];
    else if ([extension isEqualToString: @"tex"])
        return [[LaTeXFormat alloc] init];
    else if ([extension isEqualToString: @"docx"])
        return [[WordFormat alloc] init];
    return nil;
}

+ (BOOL)isExtensionSupported:(NSString *)extension
{
    return ([self formatForExtension: extension] != nil);
}

- (void)setupInitialStyles
{
    // Ensure there is at least one style present for a given heading level
    NSMutableSet *allElementNames = [NSMutableSet setWithCapacity: 0];
    CSSSheet *styleSheet = self.editor.styleSheet;
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int i = 0; allSelectors[i]; i++) {
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,allSelectors[i],NO,NO);
        [allElementNames addObject: NSStringFromC(style->elementName)];
    }
    free(allSelectors);

    for (int i = 1; i <= 6; i++) {
        NSString *elementName = [NSString stringWithFormat: @"h%d", i];
        if (![allElementNames containsObject: elementName])
            CSSSheetLookupElement(styleSheet,elementName.UTF8String,NULL,YES,YES);
    }
}

- (CSSStyle *)setupTableStyle
{
    int changed = NO;
    CSSSheet *styleSheet = self.editor.styleSheet;
    CSSStyle *result = WordSetupTableGridStyle(styleSheet,&changed);
    if (changed)
        [self.editor updateCSS];
    return result;
}

- (void)setupFigureStyle
{
    CSSStyle *figure = CSSSheetLookupElement(self.editor.styleSheet,"figure",NULL,NO,NO);
    if ((figure == nil) || CSSStyleIsEmpty(figure)) {
        figure = CSSSheetLookupElement(self.editor.styleSheet,"figure",NULL,YES,NO);
        CSSProperties *rule = CSSStyleRule(figure);
        CSSPut(rule,"text-align","center");
        CSSPut(rule,"margin-left","auto");
        CSSPut(rule,"margin-right","auto");
        CSSPut(rule,"margin-top","12pt");
        CSSPut(rule,"margin-bottom","12pt");
        figure->latent = NO;
        [self.editor updateCSS];
    }
}

- (void)setupOutlineStyle
{
}

- (NSString *)imagePath
{
    return [self.fileLocalPath stringByDeletingLastPathComponent];
}

+ (NSString *)findUniqueFilenameWithPrefix:(NSString *)prefix
                                 extension:(NSString *)extension
                             existingNames:(NSArray *)names
{
    // We first need to obtain a listing of all files in the directory, because we want a name
    // for which there is no other file with that name and a different extension
    // (e.g. image001.jpg and image001.png)

    NSMutableSet *existingNames = [NSMutableSet setWithCapacity: 0];
    for (NSString *filename in names)
        [existingNames addObject: [filename.lowercaseString stringByDeletingPathExtension]];

    int num = 1;
    NSString *candidate;
    do {
        candidate = [NSString stringWithFormat: @"%@%03d", prefix, num];
        num++;
    } while ([existingNames containsObject: candidate.lowercaseString]);

    return [candidate stringByAppendingPathExtension: extension];
}

- (NSString *)addImage:(NSData *)data extension:(NSString *)extension error:(NSError **)error
{
    NSString *baseName = [self.fileLocalPath.lastPathComponent stringByDeletingPathExtension];
    NSString *prefix = [NSString stringWithFormat: @"%@_image", baseName];
    return [self.editor.delegate saveResource: data prefix: prefix extension: extension error: error];
}

- (NSString *)localizedStyleName:(CSSStyle *)style
{
    char *cDisplayName = CSSStyleCopyDisplayName(style);
    if (cDisplayName != NULL) {
        NSString *displayName = NSStringFromC(cDisplayName);
        free(cDisplayName);
        return displayName;
    }
    NSString *str = HTML_uiNameForSelector(style->selector);
    if (str != nil)
        return str;
    return HTML_uiNameForSelector(style->selector);
}

- (Comparator)styleComparator
{
    return ^NSComparisonResult(id obj1, id obj2) {
        EDStyle *style1 = (EDStyle *)obj1;
        EDStyle *style2 = (EDStyle *)obj2;
        NSString *name1 = [self localizedStyleName: style1.cssStyle];
        NSString *name2 = [self localizedStyleName: style2.cssStyle];
        return [name1 localizedCaseInsensitiveCompare: name2];
    };
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           HTMLFormat                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation HTMLFormat

- (HTMLFormat *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (BOOL)supportsBareStyles
{
    return YES;
}

- (NSString *)absoluteHTMLPath
{
    return self.fileLocalPath;
}

- (NSSet *)builtinRawSelectors
{
    return [NSSet setWithObjects:
            @"h1",
            @"h2",
            @"h3",
            @"h4",
            @"h5",
            @"h6",
            @"p",
            @"pre",
            @"blockquote",
            @"@page",
            @"body",
            @"figure",
            @"figcaption",
            @"table",
            @"caption",
            @"nav.tableofcontents",
            @"nav.listoffigures",
            @"nav.listoftables",
            nil];
}

- (NSString *)prepareForLoad:(NSError **)error
{
    // Nothing to do here - just return the name of the HTML file to load
    return self.fileLocalPath;
}

- (void)finishLoad
{
    // Nothing to do here - default initialisation is sufficient
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    completion = [completion copy];
    NSString *extension = self.fileLocalPath.pathExtension.lowercaseString;
    BOOL isXHTML = [extension isEqualToString: @"xml"] || [extension isEqualToString: @"xhtml"];
    [self.editor.saveTiming addEntry: @"JS getHTML"];
    if (html.length == 0) {
        completion([NSError error: @"HTML code is 0 bytes"]);
        return;
    }
    html = [@"<!DOCTYPE html>\n" stringByAppendingString: html];

    NSData *rawData = [html dataUsingEncoding: NSUTF8StringEncoding];

    [EDHTMLTidy tidy: rawData isXHTML: isXHTML editor: self.editor completion:^(NSData *output, NSError *error) {
        [self.editor.saveTiming addEntry: @"HTML Tidy"];

        if (error != nil) {
            completion(error);
            return;
        }

        if ((output == nil) || (output.length == 0)) {
            completion([NSError error: @"Output of HTML Tidy is 0 bytes"]);
            return;
        }

        NSOutputStream *out = [NSOutputStream outputStreamToFileAtPath: filename append: NO];
        [out open];
        [out completeWrite: output.bytes length: output.length];
        [out close];

        if (out.streamError != nil) {
            completion(out.streamError);
            return;
        }
        [self.editor.saveTiming addEntry: @"Write temp file"];
        completion(nil);
    }];
}

- (BOOL)setupCellStyle:(NSString *)elementName
{
    CSSStyle *style = CSSSheetLookupElement(self.editor.styleSheet,elementName.UTF8String,NULL,NO,NO);
    if (style != nil)
        return NO;
    style = CSSSheetLookupElement(self.editor.styleSheet,elementName.UTF8String,NULL,YES,NO);

    CSSProperties *base = CSSStyleRuleForSuffix(style,"");
    CSSProperties *firstChild = CSSStyleRuleForSuffix(style," > :first-child");
    CSSProperties *lastChild = CSSStyleRuleForSuffix(style," > :last-child");

    CSSPut(base,"border-left-width","1px");
    CSSPut(base,"border-right-width","1px");
    CSSPut(base,"border-top-width","1px");
    CSSPut(base,"border-bottom-width","1px");

    CSSPut(base,"border-left-style","solid");
    CSSPut(base,"border-right-style","solid");
    CSSPut(base,"border-top-style","solid");
    CSSPut(base,"border-bottom-style","solid");

    CSSPut(base,"border-left-color","black");
    CSSPut(base,"border-right-color","black");
    CSSPut(base,"border-top-color","black");
    CSSPut(base,"border-bottom-color","black");

    CSSPut(firstChild,"margin-top","0");
    CSSPut(lastChild,"margin-bottom","0");
    return YES;
}

- (void)fixCorruptUXWrite10CSS
{
    // Document was edited in UX Write >= 1.1.0 and then later edited in UX Write 1.0.x
    // This results in corrupted counter-increment, counter-reset, and content values,
    // due to a bug in UX Write 1.0.x which serialises these incorrectly into the stylesheet

    // counter-increment and counter-reset are invalid, because WebKit's CSS serialisation
    // code (used by 1.0.x to generate the stylesheet text for these) places commas between
    // all of the values, which (I think) is not valid CSS

    // counter is invalid, because Styles_discoverStyles() in 1.0.x stupidly removes single
    // and double quotes from the start of any property values, on the assumption that any
    // such values are whole strings. This turns the following:
    //
    // h1::before { content: counter(h1) ". " }
    //
    // into:
    //
    // h1::before { content: counter(h1) " }
    //
    // Because CSS strings can't end in a newline, the parser discards the rest of the
    // property, and the ::before rule becomes empty

    // To get around these problems, we simply delete all the counter-increment,
    // counter-reset, and content properties, and re-create them. This is only going to be
    // a problem in a very small number of cases where a document is taken from 1.1.x back
    // to 1.0.x, e.g. due to the upgrade not being completed on all devices

    CSSSheet *styleSheet = self.editor.styleSheet;

    // First determine if heading numbering was on
    // FIXME: Won't work with Word documents, in which all styles have a class name
    CSSStyle *h1 = CSSSheetLookupElement(styleSheet,"h1",NULL,NO,NO);
    BOOL hadHeadingNumbering = (CSSGet(CSSStyleRule(h1),"counter-increment") != NULL);

    // Now clear all the counter-increment, counter-reset, and content properties
    const char **allSelectors = CSSSheetCopySelectors(styleSheet);
    for (int selIndex = 0; allSelectors[selIndex]; selIndex++) {
        const char *selector = allSelectors[selIndex];
        CSSStyle *style = CSSSheetLookupSelector(styleSheet,selector,NO,NO);

        const char **allSuffixes = CSSStyleCopySuffixes(style);
        for (int suffixIndex = 0; allSuffixes[suffixIndex]; suffixIndex++) {
            const char *suffix = allSuffixes[suffixIndex];
            CSSProperties *properties = CSSStyleRuleForSuffix(style,suffix);
            CSSPut(properties,"counter-increment",NULL);
            CSSPut(properties,"counter-reset",NULL);
            CSSPut(properties,"content",NULL);
        }
        free(allSuffixes);
    }
    free(allSelectors);

    // If the document *was* using heading numbering, turn it on again
    if (hadHeadingNumbering)
        CSSSheetSetHeadingNumbering(self.editor.styleSheet,YES);
}

- (void)transitionFromUXWrite10Numbering
{
    if (CSSSheetIsNumberingUsed(self.editor.styleSheet)) {
        // Document was edited in UX Write >= 1.1.0 and then later edited in UX Write 1.0.x
        [self fixCorruptUXWrite10CSS];
    }
    else {
        // Document was created in UX Write 1.0.x and is being opened in 1.1.x for the first time
        // Switch to using CSS counters for numbering
        BOOL sectionNumbering = [self.editor.js.outline detectSectionNumbering];
        if (sectionNumbering)
            CSSSheetSetHeadingNumbering(self.editor.styleSheet,YES);
    }
}

- (void)setupInitialStyles
{
    [super setupInitialStyles];

    for (NSString *rawSelector in self.builtinRawSelectors)
        CSSSheetLookupSelector(self.editor.styleSheet,rawSelector.UTF8String,YES,YES);

    // Styles
    CSSStyle *p = CSSSheetLookupElement(self.editor.styleSheet,"p",NULL,YES,NO);
    CSSSheetSetDefaultStyle(self.editor.styleSheet,p,StyleFamilyParagraph);

    NSString *origGenerator = self.editor.origGenerator;
    if ((origGenerator != nil) && [origGenerator hasPrefix: @"UX Write 1.0"])
        [self transitionFromUXWrite10Numbering];
}

- (CSSStyle *)setupTableStyle
{
//    td-paragraph-margins
//        td > :first-child { margin-top: 0; }
//        td > :last-child { margin-bottom: 0; }
//    th-paragraph-margins
//        th > :first-child { margin-top: 0; }
//        th > :last-child { margin-bottom: 0; }
//    table-borders
//        table { border-collapse: collapse; margin-left: auto; margin-right: auto; }
//        td { border: 1px solid black; }
//        th { border: 1px solid black; }
//    table-caption
//        caption { caption-side: bottom; }

    BOOL changed = NO;

    if ([self setupCellStyle: @"td"])
        changed = YES;
    if ([self setupCellStyle: @"th"])
        changed = YES;

    CSSStyle *table = CSSSheetLookupElement(self.editor.styleSheet,"table",NULL,NO,NO);
    if ((table == nil) || CSSStyleIsEmpty(table)) {
        table = CSSSheetLookupElement(self.editor.styleSheet,"table",NULL,YES,NO);
        CSSProperties *rule = CSSStyleRule(table);
        CSSPut(rule,"border-collapse","collapse");
        CSSPut(rule,"margin-left","auto");
        CSSPut(rule,"margin-right","auto");
        changed = YES;
    }

    if (table->latent) {
        table->latent = NO;
        changed = YES;
    }

    // FIXME: this is already done in useCSSNumbering - need to ignore duplication
    CSSStyle *caption = CSSSheetLookupElement(self.editor.styleSheet,"caption",NULL,NO,NO);
    if ((caption == nil) || CSSStyleIsEmpty(caption)) {
        caption = CSSSheetLookupElement(self.editor.styleSheet,"caption",NULL,YES,NO);
        CSSPut(CSSStyleRule(caption),"caption-side","bottom");
        changed = YES;
    }

    if (changed)
        [self.editor updateCSS];

    return table;
}

- (void)setupOutlineStyle
{
    BOOL changed = NO;

    CSSStyle *style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc1",NO,NO);
    if (style == nil) {
        style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc1",YES,NO);
        CSSProperties *rule = CSSStyleRule(style);
        CSSPut(rule,"margin-bottom","6pt");
        CSSPut(rule,"margin-left","0pt");
        CSSPut(rule,"margin-top","12pt");
        changed = YES;
    }

    style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc2",NO,NO);
    if (style == nil) {
        style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc2",YES,NO);
        CSSProperties *rule = CSSStyleRule(style);
        CSSPut(rule,"margin-bottom","6pt");
        CSSPut(rule,"margin-left","24pt");
        CSSPut(rule,"margin-top","6pt");
        changed = YES;
    }

    style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc3",NO,NO);
    if (style == nil) {
        style = CSSSheetLookupElement(self.editor.styleSheet,"p","toc3",YES,NO);
        CSSProperties *rule = CSSStyleRule(style);
        CSSPut(rule,"margin-bottom","6pt");
        CSSPut(rule,"margin-left","48pt");
        CSSPut(rule,"margin-top","6pt");
        changed = YES;
    }

    if (changed)
        [self.editor updateCSS];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           TextFormat                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation TextFormat

- (TextFormat *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    completion([NSError error: @"TextFormat save not implemented"]);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         MarkdownFormat                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation MarkdownFormat

- (MarkdownFormat *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    completion([NSError error: @"MarkdownFormat save not implemented"]);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           LaTeXFormat                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation LaTeXFormat

- (LaTeXFormat *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    completion([NSError error: @"LaTeXFormat save not implemented"]);
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordFormat                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface WordFormat()

@property (copy) NSString *tempDir;
@property (copy) NSString *idPrefix;

@property (copy, readonly) NSString *originalBackupPath;
@property (copy, readonly) NSString *htmlTempDir;
@property (copy, readonly) NSString *htmlTempFilename;

@end

@implementation WordFormat

- (WordFormat *)init
{
    if (!(self = [super init]))
        return nil;
    _idPrefix = [[NSString alloc] initWithFormat: @"bdt%u-", arc4random()];
    return self;
}

- (void)dealloc
{
    if (_tempDir != nil)
        [[NSFileManager defaultManager] removeItemAtPath: _tempDir error: nil];
}

- (NSString *)originalBackupPath
{
    return [self.tempDir stringByAppendingPathComponent: @"original.docx"];
}

- (NSString *)packageTempDir
{
    return [self.tempDir stringByAppendingPathComponent: @"package"];
}

- (NSString *)htmlTempDir
{
    return [self.tempDir stringByAppendingPathComponent: @"html"];
}

- (NSString *)htmlTempFilename
{
    return [self.htmlTempDir stringByAppendingPathComponent: @"document.html"];
}

- (NSString *)absoluteHTMLPath
{
    return self.htmlTempFilename;
}

- (NSString *)prepareForLoad:(NSError **)error
{
    self.tempDir = FCCreateTemporaryDirectory(error);
    if (self.tempDir == nil)
        return nil;

    NSFileManager *fm = [NSFileManager defaultManager];
    if (![fm createDirectoryAtPath: self.htmlTempDir withIntermediateDirectories: YES
                        attributes: nil error: error])
        return nil;

    NSString *resetSource = [[NSBundle mainBundle] pathForResource: @"reset" ofType: @"css"];
    assert(resetSource != nil);
    NSString *resetDest = [self.htmlTempDir stringByAppendingPathComponent: @"reset.css"];
    if (![fm copyItemAtPath: resetSource toPath: resetDest error: error])
        return nil;

    if (![fm copyItemAtPath: self.fileLocalPath toPath: self.originalBackupPath error: error])
        return nil;

    DFError *dferror = NULL;
    DFConcreteDocument *concreteDoc = DFConcreteDocumentOpenFile(self.originalBackupPath.UTF8String,&dferror);
    if (concreteDoc == NULL) {
        DFErrorReleaseToNSError(dferror,error);
        return nil;
    }

    DFStorage *abstractPackage = DFStorageNewFilesystem(self.htmlTempDir.UTF8String,DFFileFormatHTML);
    DFAbstractDocument *abstractDoc = DFAbstractDocumentNew(abstractPackage);

    int ok = DFGet(concreteDoc,abstractDoc,&dferror);
    DFDocument *htmlDoc = NULL;
    if (ok)
        htmlDoc = DFDocumentRetain(DFAbstractDocumentGetHTML(abstractDoc));

    DFStorageRelease(abstractPackage);
    DFAbstractDocumentRelease(abstractDoc);
    DFConcreteDocumentRelease(concreteDoc);

    if (!ok) {
        DFErrorReleaseToNSError(dferror,error);
        return nil;
    }

    if (htmlDoc == NULL) {
        DFErrorFormat(&dferror,"htmlDoc is NULL");
        DFErrorReleaseToNSError(dferror,error);
        return nil;
    }


    if (!DFSerializeXMLFile(htmlDoc,0,0,self.htmlTempFilename.UTF8String,&dferror)) {
        DFErrorReleaseToNSError(dferror,error);
        DFDocumentRelease(htmlDoc);
        return nil;
    }

    DFDocumentRelease(htmlDoc);
    return self.htmlTempFilename;
}

- (void)finishLoad
{
}

// This method is always run in a background thread, initiated via save
- (BOOL)directSave:(NSString *)filename html:(NSString *)html error:(NSError **)error
{
    DFError *dferror = NULL;
    DFDocument *htmlDoc = DFParseHTMLString(html.UTF8String,1,&dferror);
    if (htmlDoc == nil) {
        DFErrorReleaseToNSError(dferror,error);
        FCErrorPrepend(error,@"Parse HTML");
        return NO;
    }

    // Write out the HTML file in case wer're printing or generating a PDF, so it can load the
    // page content from that
    // FIXME: In this case, we don't really need to save the word document itself - saving *just*
    // the HTML will be sufficient. And when we are actually saving the word document, we don't
    // need to write out the HTML data. So this is just a temporary solution.
    DFError *localError = NULL;
    if (!DFSerializeXMLFile(htmlDoc,0,0,self.htmlTempFilename.UTF8String,&localError)) {
        DFErrorReleaseToNSError(localError,error);
        FCErrorPrepend(error,@"Save HTML");
        DFDocumentRelease(htmlDoc);
        return NO;
    }

    NSString *modifiedPath = [self.tempDir stringByAppendingPathComponent: @"modified.docx"];
    NSFileManager *fm = [NSFileManager defaultManager];
    if (![fm copyItemAtPath: self.originalBackupPath toPath: modifiedPath error: error]) {
        FCErrorPrepend(error,@"Copy original to modified");
        return NO;
    }

    DFConcreteDocument *concreteDoc = DFConcreteDocumentOpenFile(modifiedPath.UTF8String,&dferror);
    if (concreteDoc == NULL) {
        DFErrorReleaseToNSError(dferror,error);
        FCErrorPrepend(error,@"Open document for update");
        return NO;
    }

    BOOL ok = NO;

    DFStorage *abstractPackage = DFStorageNewFilesystem(self.htmlTempDir.UTF8String,DFFileFormatHTML);
    DFAbstractDocument *abstractDoc = DFAbstractDocumentNew(abstractPackage);

    DFAbstractDocumentSetHTML(abstractDoc,htmlDoc);

    if (!DFPut(concreteDoc,abstractDoc,&dferror)) {
        DFErrorReleaseToNSError(dferror,error);
        FCErrorPrepend(error,@"DFPut");
        goto end;
    }

    if ([fm fileExistsAtPath: filename] && ![fm removeItemAtPath: filename error: error]) {
        FCErrorPrepend(error,@"Remove old version of document");
        goto end;
    }

    if (![fm moveItemAtPath: modifiedPath toPath: filename error: error]) {
        FCErrorPrepend(error,@"Move new version of document into place");
        goto end;
    }

    ok = YES;

end:
    DFDocumentRelease(htmlDoc);
    DFStorageRelease(abstractPackage);
    DFAbstractDocumentRelease(abstractDoc);
    DFConcreteDocumentRelease(concreteDoc);
    return ok;
}

- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion
{
    // Create a copy of the completion on the heap, in case we're passed one that's allocated on
    // the stack
    completion = [completion copy];
    [self.editor.system runCommandInBackground:^BOOL(NSError **commandError) {
        return [self directSave: filename html: html error: commandError];
    } completion:^(NSError *completionError) {
        completion(completionError);
    }];
}

- (NSString *)imagePath
{
    return self.htmlTempDir;
}

- (void)setupInitialStyles
{
    [super setupInitialStyles];
    [self.editor.js.styles setParagraphClass: @"Normal"];
}

- (NSString *)addImage:(NSData *)data extension:(NSString *)extension error:(NSError **)error
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *contents = [fm contentsOfDirectoryAtPath: self.imagePath error: error];
    if (contents == nil)
        return nil;

    NSString *filename = [EDFileFormat findUniqueFilenameWithPrefix: @"image"
                                                        extension: extension
                                                    existingNames: contents];

    NSString *fullPath = [self.imagePath stringByAppendingPathComponent: filename];
    if (![data writeToFile: fullPath options: 0 error: error])
        return nil;

    return fullPath;
}

- (NSString *)localizedStyleName:(CSSStyle *)style
{
    char *cDisplayName = CSSStyleCopyDisplayName(style);
    if (cDisplayName != nil) {
        NSString *displayName = NSStringFromC(cDisplayName);
        free(cDisplayName);
        return [[NSBundle mainBundle] localizedStringForKey: displayName.lowercaseString
                                                      value: displayName
                                                      table: @"WordBuiltinStyles"];
    }

    return [super localizedStyleName: style];
}

@end
