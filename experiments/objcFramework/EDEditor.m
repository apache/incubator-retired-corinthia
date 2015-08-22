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

#import "EDEditor.h"
#import "EDOutline.h"
#import "EDSelectionFormatting.h"
#import "EDTiming.h"
#import "EDObservation.h"
#import "EDJSInterface.h"
#import "EDSaveOperation.h"
#import "EDFileFormat.h"
#import "EDHTMLTidy.h"
#import "EDUtil.h"
#import "EDDocumentSetup.h"
#import <FileClient/FCError.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDEditorDelegate                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDEditor
{
    BOOL _outlineDirty;
    int _programmaticChange;
    BOOL _formattingDirty;
    NSDictionary *_lastReceivedFormatting;
}

static void formattingPropertiesChanged(void *ctx, void *object, void *data)
{
    EDEditor *editor = (__bridge EDEditor *)ctx;
    [editor detectedFormattingChange];
}

- (EDEditor *)initWithFileFormat:(EDFileFormat *)fileFormat generator:(NSString *)generator tempDir:(NSString *)tempDir
{
    if (!(self = [super init]))
        return nil;

    _fileFormat = fileFormat;
    _fileFormat.editor = self;
    _generator = [generator copy];
    _tempDir = [tempDir copy];
    _outline = [[EDOutline alloc] init];
    _styleSheet = CSSSheetNew();
    _loadTiming = [[EDTimingInfo alloc] init];
    _saveTiming = [[EDTimingInfo alloc] init];

    self.formatting = [[EDSelectionFormatting alloc] init];

    return self;
}

- (void)dealloc
{
    self.formatting = nil;
    CSSSheetRelease(_styleSheet);
}

- (void)setFormatting:(EDSelectionFormatting *)newFormatting
{
    if (_formatting != nil) {
        removeObserverForAllProperties(_formatting,self);
        DFCallbackRemove(&_formatting.cssProperties->changeCallbacks,formattingPropertiesChanged,(__bridge void *)self);
    }

    _formatting = newFormatting;

    if (_formatting != nil) {
        addObserverForAllProperties(_formatting,self,0,nil);
        DFCallbackAdd(&_formatting.cssProperties->changeCallbacks,formattingPropertiesChanged,(__bridge void *)self);
    }
}

- (void)setParagraphStyleId:(NSString *)paragraphStyleId
{
    _paragraphStyleId = [paragraphStyleId copy];
    [self detectedFormattingChange];
}

- (void)setLocale:(NSString *)newLocale
{
    _locale = [newLocale copy];
    [_js.main setLanguage: _locale];
}

- (void)updateResizeHandles
{
    BOOL vertical = NO;
    EDItemGeometry *geometry = nil;
    if (_formatting.inFigure) {
        NSString *itemId = [_js.figures getSelectedFigureId];
        if (itemId != nil) {
            NSDictionary *dict = [_js.figures getGeometry: itemId];
            if (dict != nil) {
                geometry = [EDItemGeometry fromDict: dict];
                vertical = YES;
            }
        }
    }
    else if (_formatting.inTable) {
        NSString *itemId = [_js.tables getSelectedTableId];
        if (itemId != nil) {
            NSDictionary *dict = [_js.tables getGeometry: itemId];
            if (dict != nil) {
                geometry = [EDItemGeometry fromDict: dict];
                vertical = NO;
            }
        }
    }

    if (geometry != nil)
        [_delegate editorShowResizeHandles: geometry vertical: vertical];
    else
        [_delegate editorHideResizeHandles];
}

- (void)updateFormatting
{
    if (!_js.jsInitialised)
        return;

    _programmaticChange++;

    NSMutableDictionary *modified = [[_js.formatting getFormatting] mutableCopy];
    if (modified == nil)
        modified = [NSMutableDictionary dictionaryWithCapacity: 0]; // in case of JS exception

    NSString *newStyle = [modified objectForKey: @"-uxwrite-paragraph-style"];
    [modified removeObjectForKey: @"-uxwrite-paragraph-style"];
    if ((newStyle != nil) && (newStyle.length == 0))
        newStyle = @"p";

    _lastReceivedFormatting = modified;

    // FIXME: Receive paragraph style id from javascript as an (element,class) tuple, not a string
    self.paragraphStyleId = newStyle;

    self.formatting = [[EDSelectionFormatting alloc] initWithProperties: modified];
    [self updateResizeHandles];

    _programmaticChange--;
}

- (void)applyFormattingChanges
{
    if (!_formattingDirty)
        return;
    _formattingDirty = false;

    if (_js.jsInitialised) {
        // If some properties that were previously set have been removed (e.g. font-weight in the
        // case where bold has been turned off), set these as null entries in the dictionary we
        // pass to applyFormattingChanges(). Otherwise, they won't be considered as properties
        // that need changing, and will retain their previous values (e.g. font-weight will
        // remain set if there's no explicit instruction to remove it).
        DFHashTable *collapsed = CSSCollapseProperties(_formatting.cssProperties);
        NSMutableDictionary *changes = [NSDictionaryFromHashTable(collapsed) mutableCopy];
        DFHashTableRelease(collapsed);
        if (_lastReceivedFormatting != nil) {
            for (NSString *key in _lastReceivedFormatting) {
                if ([changes objectForKey: key] == nil)
                    [changes setObject: [NSNull null] forKey: key];
            }
        }
        // FIXME: Send paragraph style id to javascript as an (element,class) tuple, not a string
        [_js.formatting applyFormattingChangesStyle: _paragraphStyleId properties: changes];
    }
    [self updateFormatting];
}

- (void)detectedFormattingChange
{
    if (_programmaticChange == 0) {
        _formattingDirty = YES;
        // Invoke applyFormattingChanges after the current iteration of the event loop has finished.
        // This way, if multiple properties of the formatting object are changed at the same time,
        // we will only call the javascript applyFormatting() method once after all the changes
        // have been made.
        [self performSelector: @selector(applyFormattingChanges) withObject: nil afterDelay: 0];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
                        change:(NSDictionary *)change context:(void *)context
{
    if (object == _formatting)
        [self detectedFormattingChange];
}

- (void)updateCSS
{
    [_delegate editorDidUpdateCSS];
}

- (void)retrieveStyles
{
    NSString *cssText = [_js.styles getCSSText];
    CSSSheetUpdateFromCSSText(_styleSheet,cssText.UTF8String);
    [_fileFormat setupInitialStyles];
    CSSSheetUseCSSNumbering(_styleSheet);

    // If any of the built-in styles (e.g. figcaption) are used in the document, but not explicitly
    // mentioned in the CSS stylesheet text, then mark them as non-latent. This will cause their
    // definitions to be explicitly included in the stylesheet text.
    // FIXME: Get these sent from javascript as (element,class) pairs
    NSDictionary *usedStyles = [_js.outline findUsedStyles];
    if (_js.error == nil) {
        for (NSString *rawSelector in usedStyles.allKeys) {
            CSSStyle *style = CSSSheetLookupSelector(_styleSheet,rawSelector.UTF8String,NO,NO);
            if ((style != nil) && (style->latent))
                style->latent = NO;
        }
    }

    // updateCSS causes the documentModified flag to be set, because normally it does actually
    // represent an actual change to the document. But since this function is only called at load
    // time, all we're actually doing here is making sure the HTML document's stylesheet is
    // consistent with what we maintain on the Objective C side. So in this particular case we
    // don't want it to count as a modification - otherwise simp,ly opening and closing the document
    // would cause it to be saved, resulting in an unnecessary upload to the server
    BOOL oldModified = _js.documentModified;
    [self updateCSS];
    _js.documentModified = oldModified;
}

- (void)makeStyleNonLatent:(const char *)ident
{
    // We must set the add parameter to YES here; otherwise, the latent parameter will be ignored
    CSSSheetLookupSelector(_styleSheet,ident,YES,NO);
    [self updateCSS];
}

- (void)updateOutline
{
    if (!_js.jsInitialised)
        return;
    _outlineDirty = NO;
    _outline.json = [_js.outline getOutline];
    [_delegate editorDidUpdateOutline];
}

- (void)setOutlineDirty
{
    if (!_outlineDirty) {
        _outlineDirty = YES;
        [self performSelector: @selector(updateOutline) withObject: nil afterDelay: 0];
    }
}

- (void)loadAndInitSucceeded
{
    _jsInitOk = YES;
    _js.jsInitialised = YES;
    [_fileFormat finishLoad];
    [_loadTiming addEntry: @"File format-specific initialisation"];
    _origGenerator = [_js.main setGenerator: _generator];
    [self retrieveStyles];
    [self updateOutline];

    NSString *localeOrEmpty = [_js.main getLanguage];
    if (localeOrEmpty.length > 0)
        _locale = localeOrEmpty;
    else
        _locale = nil;

    [self updateFormatting];
}

- (void)dumpHTML
{
    NSString *inputStr = [_js.main getHTML];
    NSData *input = [inputStr dataUsingEncoding: NSUTF8StringEncoding];
    [EDHTMLTidy tidy: input isXHTML: NO editor: self completion:^(NSData *output, NSError *error) {
        if (error != nil) {
            debug(@"HTMLTidy failed: %@\n",FCErrorDescription(error));
        }
        else {
            NSString *outputStr = [[NSString alloc] initWithData: output encoding: NSUTF8StringEncoding];
            debug(@"\n");
            debug(@"------------------------------------- HTML -------------------------------------\n");
            debug(@"%@",outputStr);
            debug(@"--------------------------------------------------------------------------------\n");
            debug(@"\n");
        }
    }];
}

- (void)saveTo:(NSString *)path completion:(EDSaveCompletion)completion
{
    if (!_js.jsInitialised)
        return;

    if (_activeSave == nil) {
        _activeSave = [[EDSaveOperation alloc] initWithEditor: self path: path];
        [_activeSave addCompletion: completion];
        [_activeSave start];
    }
    else {
        if (_pendingSave == nil)
            _pendingSave = [[EDSaveOperation alloc] initWithEditor: self path: path];
        [_pendingSave addCompletion: completion];
    }
    [self debugSaveStatus];
}

- (void)debugSaveStatus
{
    NSMutableString *str = [NSMutableString stringWithCapacity: 0];
    [str appendFormat: @"Save status:"];

    if (_activeSave != nil)
        [str appendFormat: @" activeSave = %p (%d)", _activeSave, (int)_activeSave.completionCount];
    else
        [str appendFormat: @" activeSave = nil"];

    if (_pendingSave != nil)
        [str appendFormat: @", pendingSave = %p (%d)", _pendingSave, (int)_pendingSave.completionCount];
    else
        [str appendFormat: @", pendingSave = nil"];

    debug(@"%@\n",str);
}

- (void)undo
{
    if (self.js.jsInitialised) {
        [self.js.undoManager undo];
        [self updateFormatting];
    }
}

- (void)redo
{
    if (self.js.jsInitialised) {
        [self.js.undoManager redo];
        [self updateFormatting];
    }
}

// ResizeDelegate methods

- (void)resizedWidthPct:(CGFloat)widthPct
{
    NSString *widthStr = [NSString stringWithFormat: @"%d%%", (int)round(widthPct)];

    if (_formatting.inFigure) {
        NSString *itemId = [_js.figures getSelectedFigureId];
        if (itemId == nil)
            return;

        NSDictionary *properties = [_js.figures getProperties: itemId];
        if (properties == nil)
            return;

        NSString *src = [properties objectForKey: @"src"];
        [_js.figures setProperties: itemId width: widthStr src: src];

    }
    else if (_formatting.inTable) {
        NSString *itemId = [_js.tables getSelectedTableId];
        if (itemId == nil)
            return;

        [_js.tables setProperties: itemId width: widthStr];
    }

    [self updateFormatting];
}

- (void)resizedColumns:(NSArray *)widthPcts
{
    if (_formatting.inTable) {
        NSString *itemId = [_js.tables getSelectedTableId];
        if (itemId == nil)
            return;

        NSMutableArray *rounded = [NSMutableArray arrayWithCapacity: widthPcts.count];
        for (NSUInteger i = 0; i < widthPcts.count; i++) {
            NSNumber *raw = [widthPcts objectAtIndex: i];
            [rounded addObject: [NSNumber numberWithDouble: round(raw.doubleValue)]];
        }

        [_js.tables set: itemId colWidths: rounded];
    }
    
    [self updateFormatting];
}

@end
