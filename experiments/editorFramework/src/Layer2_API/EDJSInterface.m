//
//  EDJSInterface.m
//  Editor
//
//  Created by Peter Kelly on 22/11/11.
//  Copyright (c) 2011-2014 UX Productivity Pty Ltd. All rights reserved.
//

#import "EDJSInterface.h"
#import "EDScan.h"
#import "EDUtil.h"
#import <FileClient/FCError.h>
#import <FileClient/FCUtil.h>

typedef enum {
    AutoCorrect_correctPrecedingWord,
    AutoCorrect_getCorrection,
    AutoCorrect_getCorrectionCoords,
    AutoCorrect_acceptCorrection,
    AutoCorrect_replaceCorrection,
    ChangeTracking_showChanges,
    ChangeTracking_trackChanges,
    ChangeTracking_setShowChanges,
    ChangeTracking_setTrackChanges,
    Clipboard_cut,
    Clipboard_copy,
    Clipboard_pasteHTML,
    Clipboard_pasteText,
    Cursor_positionCursor,
    Cursor_getCursorPosition,
    Cursor_moveLeft,
    Cursor_moveRight,
    Cursor_moveToStartOfDocument,
    Cursor_moveToEndOfDocument,
    Cursor_insertReference,
    Cursor_insertLink,
    Cursor_insertCharacter,
    Cursor_deleteCharacter,
    Cursor_enterPressed,
    Cursor_getPrecedingWord,
    Cursor_getLinkProperties,
    Cursor_setLinkProperties,
    Cursor_setReferenceTarget,
    Cursor_insertFootnote,
    Cursor_insertEndnote,
    Editor_getBackMessages,
    Equations_insertEquation,
    Figures_insertFigure,
    Figures_getSelectedFigureId,
    Figures_getProperties,
    Figures_setProperties,
    Figures_getGeometry,
    Formatting_getFormatting,
    Formatting_applyFormattingChanges,
    Input_removePosition,
    Input_textInRange,
    Input_replaceRange,
    Input_selectedTextRange,
    Input_setSelectedTextRange,
    Input_markedTextRange,
    Input_setMarkedText,
    Input_unmarkText,
    Input_forwardSelectionAffinity,
    Input_setForwardSelectionAffinity,
    Input_positionFromPositionOffset,
    Input_positionFromPositionInDirectionOffset,
    Input_comparePositionToPosition,
    Input_offsetFromPositionToPosition,
    Input_positionWithinRangeFarthestInDirection,
    Input_characterRangeByExtendingPositionInDirection,
    Input_firstRectForRange,
    Input_caretRectForPosition,
    Input_closestPositionToPoint,
    Input_closestPositionToPointWithinRange,
    Input_characterRangeAtPoint,
    Input_positionWithinRangeAtCharacterOffset,
    Input_characterOffsetOfPositionWithinRange,
    Input_isPositionAtBoundaryGranularityInDirection,
    Input_isPositionWithinTextUnitInDirection,
    Input_positionFromPositionToBoundaryInDirection,
    Input_rangeEnclosingPositionWithGranularityInDirection,
    Lists_increaseIndent,
    Lists_decreaseIndent,
    Lists_clearList,
    Lists_setUnorderedList,
    Lists_setOrderedList,
    Main_getLanguage,
    Main_setLanguage,
    Main_setGenerator,
    Main_init,
    Main_getErrorReportingInfo,
    Main_execute,
    Main_prepareForSave,
    Main_getHTML,
    Main_isEmptyDocument,
    Metadata_getMetadata,
    Metadata_setMetadata,
    Outline_moveSection,
    Outline_deleteItem,
    Outline_goToItem,
    Outline_scheduleUpdateStructure,
    Outline_setNumbered,
    Outline_setTitle,
    Outline_getOutline,
    Outline_insertTableOfContents,
    Outline_insertListOfFigures,
    Outline_insertListOfTables,
    Outline_setPrintMode,
    Outline_examinePrintLayout,
    Outline_detectSectionNumbering,
    Outline_findUsedStyles,
    Preview_showForStyle,
    Scan_reset,
    Scan_next,
    Scan_addMatch,
    Scan_showMatch,
    Scan_replaceMatch,
    Scan_removeMatch,
    Scan_goToMatch,
    Selection_update,
    Selection_selectAll,
    Selection_selectParagraph,
    Selection_selectWordAtCursor,
    Selection_dragSelectionBegin,
    Selection_dragSelectionUpdate,
    Selection_moveStartLeft,
    Selection_moveStartRight,
    Selection_moveEndLeft,
    Selection_moveEndRight,
    Selection_setSelectionStartAtCoords,
    Selection_setSelectionEndAtCoords,
    Selection_setTableSelectionEdgeAtCoords,
    Selection_print,
    Styles_getCSSText,
    Styles_setCSSText,
    Styles_getParagraphClass,
    Styles_setParagraphClass,
    Tables_insertTable,
    Tables_addAdjacentRow,
    Tables_addAdjacentColumn,
    Tables_removeAdjacentRow,
    Tables_removeAdjacentColumn,
    Tables_clearCells,
    Tables_mergeCells,
    Tables_splitSelection,
    Tables_getSelectedTableId,
    Tables_getProperties,
    Tables_setProperties,
    Tables_setColWidths,
    Tables_getGeometry,
    UndoManager_getLength,
    UndoManager_getIndex,
    UndoManager_setIndex,
    UndoManager_undo,
    UndoManager_redo,
    UndoManager_newGroup,
    UndoManager_groupType,
    Viewport_setViewportWidth,
    Viewport_setTextScale,
    JSInterfaceFunctionCount,
} JSInterfaceFunction;

static BOOL functionModifiesDocument(JSInterfaceFunction fun)
{
    switch (fun)
    {
        case AutoCorrect_correctPrecedingWord:
        case AutoCorrect_acceptCorrection:
        case AutoCorrect_replaceCorrection:
        case ChangeTracking_setShowChanges:
        case ChangeTracking_setTrackChanges:
        case Clipboard_cut:
        case Clipboard_pasteHTML:
        case Clipboard_pasteText:
        case Cursor_insertReference:
        case Cursor_insertLink:
        case Cursor_insertCharacter:
        case Cursor_deleteCharacter:
        case Cursor_enterPressed:
        case Cursor_setLinkProperties:
        case Cursor_setReferenceTarget:
        case Cursor_insertFootnote:
        case Cursor_insertEndnote:
        case Equations_insertEquation:
        case Figures_insertFigure:
        case Figures_setProperties:
        case Formatting_applyFormattingChanges:
        case Lists_increaseIndent:
        case Lists_decreaseIndent:
        case Lists_clearList:
        case Lists_setUnorderedList:
        case Lists_setOrderedList:
        case Main_setLanguage:
        case Main_prepareForSave:
        case Metadata_setMetadata:
        case Outline_moveSection:
        case Outline_deleteItem:
        case Outline_scheduleUpdateStructure:
        case Outline_setNumbered:
        case Outline_setTitle:
        case Outline_insertTableOfContents:
        case Outline_insertListOfFigures:
        case Outline_insertListOfTables:
        case Outline_setPrintMode:
        case Outline_examinePrintLayout:
        case Scan_replaceMatch:
        case Styles_setCSSText:
        case Tables_insertTable:
        case Tables_addAdjacentRow:
        case Tables_addAdjacentColumn:
        case Tables_removeAdjacentRow:
        case Tables_removeAdjacentColumn:
        case Tables_clearCells:
        case Tables_mergeCells:
        case Tables_splitSelection:
        case Tables_setProperties:
        case Tables_setColWidths:
        case UndoManager_undo:
        case UndoManager_redo:
            return true;
        case AutoCorrect_getCorrection:
        case AutoCorrect_getCorrectionCoords:
        case ChangeTracking_showChanges:
        case ChangeTracking_trackChanges:
        case Clipboard_copy:
        case Cursor_positionCursor:
        case Cursor_getCursorPosition:
        case Cursor_moveLeft:
        case Cursor_moveRight:
        case Cursor_moveToStartOfDocument:
        case Cursor_moveToEndOfDocument:
        case Cursor_getPrecedingWord:
        case Cursor_getLinkProperties:
        case Editor_getBackMessages:
        case Figures_getSelectedFigureId:
        case Figures_getProperties:
        case Figures_getGeometry:
        case Formatting_getFormatting:
        case Input_removePosition:
        case Input_textInRange:
        case Input_replaceRange:
        case Input_selectedTextRange:
        case Input_setSelectedTextRange:
        case Input_markedTextRange:
        case Input_setMarkedText:
        case Input_unmarkText:
        case Input_forwardSelectionAffinity:
        case Input_setForwardSelectionAffinity:
        case Input_positionFromPositionOffset:
        case Input_positionFromPositionInDirectionOffset:
        case Input_comparePositionToPosition:
        case Input_offsetFromPositionToPosition:
        case Input_positionWithinRangeFarthestInDirection:
        case Input_characterRangeByExtendingPositionInDirection:
        case Input_firstRectForRange:
        case Input_caretRectForPosition:
        case Input_closestPositionToPoint:
        case Input_closestPositionToPointWithinRange:
        case Input_characterRangeAtPoint:
        case Input_positionWithinRangeAtCharacterOffset:
        case Input_characterOffsetOfPositionWithinRange:
        case Input_isPositionAtBoundaryGranularityInDirection:
        case Input_isPositionWithinTextUnitInDirection:
        case Input_positionFromPositionToBoundaryInDirection:
        case Input_rangeEnclosingPositionWithGranularityInDirection:
        case Main_getLanguage:
        case Main_setGenerator:
        case Main_init:
        case Main_getErrorReportingInfo:
        case Main_execute:
        case Main_getHTML:
        case Main_isEmptyDocument:
        case Metadata_getMetadata:
        case Outline_getOutline:
        case Outline_goToItem:
        case Outline_detectSectionNumbering:
        case Outline_findUsedStyles:
        case Preview_showForStyle:
        case Scan_reset:
        case Scan_next:
        case Scan_addMatch:
        case Scan_showMatch:
        case Scan_removeMatch:
        case Scan_goToMatch:
        case Selection_update:
        case Selection_selectAll:
        case Selection_selectParagraph:
        case Selection_selectWordAtCursor:
        case Selection_dragSelectionBegin:
        case Selection_dragSelectionUpdate:
        case Selection_moveStartLeft:
        case Selection_moveStartRight:
        case Selection_moveEndLeft:
        case Selection_moveEndRight:
        case Selection_setSelectionStartAtCoords:
        case Selection_setSelectionEndAtCoords:
        case Selection_setTableSelectionEdgeAtCoords:
        case Selection_print:
        case Styles_getCSSText:
        case Styles_getParagraphClass:
        case Styles_setParagraphClass:
        case Tables_getSelectedTableId:
        case Tables_getProperties:
        case Tables_getGeometry:
        case UndoManager_getLength:
        case UndoManager_getIndex:
        case UndoManager_setIndex:
        case UndoManager_newGroup:
        case UndoManager_groupType:
        case Viewport_setViewportWidth:
        case Viewport_setTextScale:
        case JSInterfaceFunctionCount:
            return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             JSError                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSError

- (JSError *)initWithType:(NSString *)type
                  message:(NSString *)message
                operation:(NSString *)operation
                     html:(NSString *)html
{
    if (!(self = [super init]))
        return nil;
    _type = [type copy];
    _message = [message copy];
    _operation = [operation copy];
    _html = [html copy];
    return self;
}

- (NSString *)description
{
    NSMutableString *errorInfo = [NSMutableString stringWithCapacity: 0];
    [errorInfo appendFormat: @"%@\n", _message];
    [errorInfo appendFormat: @"Operation: %@\n", _operation];
    [errorInfo appendFormat: @"\n"];
    [errorInfo appendFormat: @"HTML:\n%@\n", _html];
    return errorInfo;
}

- (NSString *)localizedDescription
{
    return [self description];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           JSInterface                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSInterface
{
    NSUInteger _numCalls[JSInterfaceFunctionCount];
    NSTimeInterval _timeUsed[JSInterfaceFunctionCount];
    NSTimeInterval _totalTimeUsed;
}

- (JSInterface *)initWithEvaluator:(NSObject<JSEvaluator> *)evaluator
{
    if (!(self = [super init]))
        return nil;
    _evaluator = evaluator;

    _autoCorrect = [[JSAutoCorrect alloc] initWithJS: self];
    _changeTracking = [[JSChangeTracking alloc] initWithJS: self];
    _clipboard = [[JSClipboard alloc] initWithJS: self];
    _cursor = [[JSCursor alloc] initWithJS: self];
    _equations = [[JSEquations alloc] initWithJS: self];
    _figures = [[JSFigures alloc] initWithJS: self];
    _formatting = [[JSFormatting alloc] initWithJS: self];
    _input = [[JSInput alloc] initWithJS: self];
    _lists = [[JSLists alloc] initWithJS: self];
    _main = [[JSMain alloc] initWithJS: self];
    _metadata = [[JSMetadata alloc] initWithJS: self];
    _outline = [[JSOutline alloc] initWithJS: self];
    _preview = [[JSPreview alloc] initWithJS: self];
    _scan = [[JSScan alloc] initWithJS: self];
    _selection = [[JSSelection alloc] initWithJS: self];
    _styles = [[JSStyles alloc] initWithJS: self];
    _tables = [[JSTables alloc] initWithJS: self];
    _undoManager = [[JSUndoManager alloc] initWithJS: self];
    _viewport = [[JSViewport alloc] initWithJS: self];

    return self;
}

- (void)dealloc
{
    _autoCorrect.js = nil;
    _changeTracking.js = nil;
    _clipboard.js = nil;
    _cursor.js = nil;
    _equations.js = nil;
    _figures.js = nil;
    _formatting.js = nil;
    _input.js = nil;
    _lists.js = nil;
    _main.js = nil;
    _metadata.js = nil;
    _outline.js = nil;
    _preview.js = nil;
    _scan.js = nil;
    _selection.js = nil;
    _styles.js = nil;
    _tables.js = nil;
    _undoManager.js = nil;
    _viewport.js = nil;
}

- (BOOL)initJavaScriptWidth:(int)width textScale:(int)textScale cssURL:(NSString *)cssURL
             clientRectsBug:(BOOL)clientRectsBug
{
    // Special case javascript call to Main_init(). We don't use the normal mechanism here because
    // at this point the JS interface has not been initialised, and the check for this in
    // executeJavaScriptWithJSONResult would cause it to fail.
    NSString *code = [NSString stringWithFormat: @"interface_functions[%d](%d,%d,\"%@\",%@)",
                      Main_init, width, textScale, cssURL, clientRectsBug ? @"true" : @"false"];
    NSString *result = [_evaluator stringByEvaluatingJavaScriptFromString: code];
    [self jsCallCompleted];
    if ([@"true" isEqualToString: result]) {
        return YES;
    }
    else {
        self.currentOperation = @"JavaScript initialisation";
        [self reportErrorWithType: nil format: @"%@", result];
        self.currentOperation = nil;
        return NO;
    }
}

- (void)jsCallCompleted
{
    // Process pending callbacks
    NSString *getBackMessages = [NSString stringWithFormat: @"interface_functions[%d]()",
                                 Editor_getBackMessages];
    NSString *str = [_evaluator stringByEvaluatingJavaScriptFromString: getBackMessages];
    NSData *data = [str dataUsingEncoding: NSUTF8StringEncoding];
    NSArray *array = [NSJSONSerialization JSONObjectWithData: data options: 0 error: nil];
    for (NSArray *message in array)
        [self dispatchCallbackMessage: message];
}

- (NSString *)operationStringWithFunction:(JSInterfaceFunction)function nargs:(int)nargs arguments:(va_list)ap
{
    NSMutableString *operation = [NSMutableString stringWithCapacity: 0];
    [operation appendFormat: @"interface_functions[%d](", function];
    NSObject *arg;
    for (int argno = 0; argno < nargs; argno++) {
        arg = va_arg(ap,NSObject*);
        if (argno > 0)
            [operation appendString: @","];
        if (arg == nil) {
            [operation appendString: @"null"];
        }
        else if ([arg isKindOfClass: [NSNumber class]]) {
            [operation appendString: [arg description]];
        }
        else if ([arg isKindOfClass: [NSNull class]]) {
            [operation appendString: @"null"];
        }
        else if ([arg isKindOfClass: [NSArray class]] || [arg isKindOfClass: [NSDictionary class]]) {
            NSError *err = nil;
            NSData *data = [NSJSONSerialization dataWithJSONObject: arg options: 0 error: &err];
            if (data == nil) {
                [NSException raise: @"NSJSONSerialization"
                            format: @"executeJavaScript %d: cannot serialise argument %d to JSON: %@",
                                    function, argno, FCErrorDescription(err)];
            }
            else {
                NSString *str = [[NSString alloc] initWithBytes: data.bytes length: data.length
                                                       encoding: NSUTF8StringEncoding];
                [operation appendString: str];
            }
        }
        else {
            char *quoted = DFQuote(arg.description.UTF8String);
            NSString *nsQuoted = [NSString stringWithUTF8String: quoted];
            [operation appendString: nsQuoted];
            free(quoted);
        }
    }
    [operation appendString: @")"];
    return operation;
}

- (id)executeJavaScriptWithJSONResult:(BOOL)jsonResult function:(JSInterfaceFunction)function nargs:(int)nargs arguments:(va_list)ap
{
    NSDate *start = [NSDate date];
    if (!_jsInitialised) {
        [NSException raise: @"JavaScriptNotInitialised"
                    format: @"Yet to receive a jsInterfaceInitFinished from JavaScript code"];
    }
    self.currentOperation = [self operationStringWithFunction: function nargs: nargs arguments: ap];
    id result;
    if (!jsonResult) {
        NSString *code = [NSString stringWithFormat: @"interface_functions[%d](function() { return %@; });",
                                                     Main_execute, self.currentOperation];
        result = [_evaluator stringByEvaluatingJavaScriptFromString: code];
        [self jsCallCompleted];
    }
    else {
        NSString *code = [NSString stringWithFormat: @"interface_functions[%d](function() { return JSON.stringify(%@); });",
                                                     Main_execute, self.currentOperation];

        NSString *str = [_evaluator stringByEvaluatingJavaScriptFromString: code];
        [self jsCallCompleted];

        if ([str isEqualToString: @"null"]) {
            result = nil;
        }
        else {
            NSData *data = [str dataUsingEncoding: NSUTF8StringEncoding];
            NSError *err = nil;
            result = [NSJSONSerialization JSONObjectWithData: data options: 0 error: &err];
            if (result == nil) {
                [self reportErrorWithType: nil format: @"Cannot deserialise JSON result \"%@\": %@",
                 str, FCErrorDescription(err)];
            }
        }
    }

    NSDate *end = [NSDate date];
    NSTimeInterval interval = [end timeIntervalSinceDate: start];
    _numCalls[function]++;
    _timeUsed[function] += interval;
    _totalTimeUsed += interval;

    self.currentOperation = nil;
    if (functionModifiesDocument(function))
        self.documentModified = YES;
    return result;
}

- (void)printStatistics
{
    if (_totalTimeUsed == 0.0)
        return;

    printf("\n");
    printf("%-10s %-10s %-10s %-10s\n","Function","Calls","Time","Time pct");

    NSMutableArray *order = [NSMutableArray arrayWithCapacity: 0];
    for (NSUInteger i = 0; i < JSInterfaceFunctionCount; i++)
        [order addObject: [NSNumber numberWithUnsignedInteger: i]];
    [order sortUsingComparator:^NSComparisonResult(NSNumber *n1, NSNumber *n2) {
        NSUInteger i1 = n1.unsignedIntValue;
        NSUInteger i2 = n2.unsignedIntValue;
        if (_timeUsed[i1] < _timeUsed[i2])
            return NSOrderedAscending;
        else if (_timeUsed[i1] > _timeUsed[i2])
            return NSOrderedDescending;
        else
            return NSOrderedSame;
    }];
    for (NSNumber *n in order) {
        NSUInteger i = n.unsignedIntValue;
        double pct = 100*(_timeUsed[i]/_totalTimeUsed);
        if (pct >= 0.01) {
            printf("%-10d %-10d %-10d %.2f%%\n",(int)i,(int)_numCalls[i],(int)(_timeUsed[i]*1000),pct);
        }
    }
    printf("totalTimeUsed = %.2fms\n",1000*_totalTimeUsed);
}

- (id)executeJavaScriptJSON:(JSInterfaceFunction)function nargs:(int)nargs, ...
{
    va_list ap;
    va_start(ap,nargs);
    NSString *result = (NSString *)[self executeJavaScriptWithJSONResult: YES
                                                                function: function
                                                                   nargs: nargs
                                                               arguments: ap];
    va_end(ap);
    return result;
}

- (NSString *)executeJavaScript:(JSInterfaceFunction)function nargs:(int)nargs, ...
{
    va_list ap;
    va_start(ap,nargs);
    NSString *result = (NSString *)[self executeJavaScriptWithJSONResult: NO
                                                                function: function
                                                                   nargs: nargs
                                                               arguments: ap];
    va_end(ap);
    return result;
}

- (void)dispatchCallbackMessage:(NSArray*)array
{
    if (array.count == 0)
        return;

    if (![[array objectAtIndex: 0] isKindOfClass: [NSString class]])
        return;

    NSString *functionName = [array objectAtIndex: 0];

    if ([functionName isEqualToString: @"debug"] &&
        ([array count] == 2) &&
        [[array objectAtIndex: 1] isKindOfClass: [NSString class]]) {
        NSString *message = [array objectAtIndex: 1];
        debug(@"%@\n",message);
    }
    else if ([functionName isEqualToString: @"addOutlineItem"] &&
             [_delegate respondsToSelector: @selector(jsAddOutlineItem:type:title:)] &&
             ([array count] == 4) &&
             [[array objectAtIndex: 1] isKindOfClass: [NSString class]] &&
             [[array objectAtIndex: 2] isKindOfClass: [NSString class]]) {
        // 3 could be a a string or null
        NSString *title = [array objectAtIndex: 3];
        if ([title isKindOfClass: [NSNull class]])
            title = nil;
        [_delegate jsAddOutlineItem: [array objectAtIndex: 1]
                              type: [array objectAtIndex: 2]
                             title: title];
    }
    else if ([functionName isEqualToString: @"updateOutlineItem"] &&
             [_delegate respondsToSelector: @selector(jsUpdateOutlineItem:title:)] &&
             ([array count] == 3) &&
             [[array objectAtIndex: 1] isKindOfClass: [NSString class]] &&
             [[array objectAtIndex: 2] isKindOfClass: [NSString class]]) {
        [_delegate jsUpdateOutlineItem: [array objectAtIndex: 1] title: [array objectAtIndex: 2]];
    }
    else if ([functionName isEqualToString: @"removeOutlineItem"] &&
             [_delegate respondsToSelector: @selector(jsRemoveOutlineItem:)] &&
             ([array count] == 2) &&
             [[array objectAtIndex: 1] isKindOfClass: [NSString class]]) {
        [_delegate jsRemoveOutlineItem: [array objectAtIndex: 1]];
    }
    else if ([functionName isEqualToString: @"outlineUpdated"] &&
             [_delegate respondsToSelector: @selector(jsOutlineUpdated)] &&
             [array count] == 1) {
        [_delegate jsOutlineUpdated];
    }
    else if ([functionName isEqualToString: @"setSelectionHandles"] &&
             [_delegate respondsToSelector: @selector(jsSetSelectionHandlesX1:y1:height1:x2:y2:height2:)] &&
             ([array count] == 7) &&
             ([[array objectAtIndex: 1] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 2] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 3] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 4] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 5] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 6] isKindOfClass: [NSNumber class]])) {
        [_delegate jsSetSelectionHandlesX1: ((NSNumber *)[array objectAtIndex: 1]).intValue
                                       y1: ((NSNumber *)[array objectAtIndex: 2]).intValue
                                  height1: ((NSNumber *)[array objectAtIndex: 3]).intValue
                                       x2: ((NSNumber *)[array objectAtIndex: 4]).intValue
                                       y2: ((NSNumber *)[array objectAtIndex: 5]).intValue
                                  height2: ((NSNumber *)[array objectAtIndex: 6]).intValue];
    }
    else if ([functionName isEqualToString: @"setTableSelection"] &&
             [_delegate respondsToSelector: @selector(jsSetTableSelectionX:y:width:height:)] &&
             ([array count] == 5) &&
             ([[array objectAtIndex: 1] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 2] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 3] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 4] isKindOfClass: [NSNumber class]])) {
        [_delegate jsSetTableSelectionX: ((NSNumber *)[array objectAtIndex: 1]).intValue
                                     y: ((NSNumber *)[array objectAtIndex: 2]).intValue
                                 width: ((NSNumber *)[array objectAtIndex: 3]).intValue
                                height: ((NSNumber *)[array objectAtIndex: 4]).intValue];
    }
    else if ([functionName isEqualToString: @"setCursor"] &&
             [_delegate respondsToSelector: @selector(jsSetCursorX:y:width:height:)] &&
             ([array count] == 5) &&
             ([[array objectAtIndex: 1] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 2] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 3] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 4] isKindOfClass: [NSNumber class]])) {
        [_delegate jsSetCursorX: ((NSNumber *)[array objectAtIndex: 1]).intValue
                             y: ((NSNumber *)[array objectAtIndex: 2]).intValue
                         width: ((NSNumber *)[array objectAtIndex: 3]).intValue
                        height: ((NSNumber *)[array objectAtIndex: 4]).intValue];
    }
    else if ([functionName isEqualToString: @"clearSelectionHandlesAndCursor"] &&
             [_delegate respondsToSelector: @selector(jsClearSelectionHandlesAndCursor)] &&
             ([array count] == 1)) {
        [_delegate jsClearSelectionHandlesAndCursor];
    }
    else if ([functionName isEqualToString: @"setSelectionBounds"] &&
             [_delegate respondsToSelector: @selector(jsSetSelectionBoundsLeft:top:right:bottom:)] &&
             ([array count] == 5) &&
             ([[array objectAtIndex: 1] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 2] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 3] isKindOfClass: [NSNumber class]]) &&
             ([[array objectAtIndex: 4] isKindOfClass: [NSNumber class]])) {
        [_delegate jsSetSelectionBoundsLeft: ((NSNumber *)[array objectAtIndex: 1]).intValue
                                       top: ((NSNumber *)[array objectAtIndex: 2]).intValue
                                     right: ((NSNumber *)[array objectAtIndex: 3]).intValue
                                    bottom: ((NSNumber *)[array objectAtIndex: 4]).intValue];
    }
    else if ([functionName isEqualToString: @"updateAutoCorrect"] &&
             [_delegate respondsToSelector: @selector(jsUpdateAutoCorrect)]) {
        [_delegate jsUpdateAutoCorrect];
    }
    else if ([functionName isEqualToString: @"error"] &&
             (array.count == 3) &&
             [[array objectAtIndex: 1] isKindOfClass: [NSString class]] &&
             [[array objectAtIndex: 2] isKindOfClass: [NSString class]]) {
        NSString *message = [array objectAtIndex: 1];
        NSString *type = [array objectAtIndex: 2];
        if (type.length == 0)
            type = nil;
        [self reportErrorWithType: type format: @"%@", message];
    }
}

- (void)reportErrorWithType:(NSString *)type format:(NSString *)format, ...
{
    if (self.error != nil)
        return;

    va_list ap;
    va_start(ap,format);
    NSString *basicError = [[NSString alloc] initWithFormat: format arguments: ap];
    va_end(ap);

    NSString *getErrorReportingInfo = [NSString stringWithFormat: @"interface_functions[%d]()",
                                       Main_getErrorReportingInfo];
    NSString *html = [_evaluator stringByEvaluatingJavaScriptFromString: getErrorReportingInfo];

    self.error = [[JSError alloc] initWithType: type
                                       message: basicError
                                     operation: self.currentOperation
                                          html: html];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSModule                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSModule

- (JSModule *)initWithJS:(JSInterface *)js
{
    if (!(self = [super init]))
        return nil;
    _js = js;
    return self;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          JSAutoCorrect                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSAutoCorrect

- (void)correctPreceding:(int)numChars word:(NSString *)replacement confirmed:(BOOL)confirmed;
{
    [self.js executeJavaScript: AutoCorrect_correctPrecedingWord nargs: 3,
     [NSNumber numberWithInt: numChars], replacement, [NSNumber numberWithBool: confirmed]];
}

- (NSDictionary *)getCorrection
{
    return [self.js executeJavaScriptJSON: AutoCorrect_getCorrection nargs: 0];
}

- (NSDictionary *)getCorrectionCoords
{
    return [self.js executeJavaScriptJSON: AutoCorrect_getCorrectionCoords nargs: 0];
}

- (void)acceptCorrection
{
    [self.js executeJavaScript: AutoCorrect_acceptCorrection nargs: 0];
}

- (void)replaceCorrection:(NSString *)replacement
{
    [self.js executeJavaScript: AutoCorrect_replaceCorrection nargs: 1, replacement];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        JSChangeTracking                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSChangeTracking

- (BOOL)showChanges
{
    NSString *result = [self.js executeJavaScript: ChangeTracking_showChanges nargs: 0];
    return [result isEqualToString: @"true"];
}

- (BOOL)trackChanges
{
    NSString *result = [self.js executeJavaScript: ChangeTracking_trackChanges nargs: 0];
    return [result isEqualToString: @"true"];
}

- (void)setShowChanges:(BOOL)showChanges
{
    [self.js executeJavaScript: ChangeTracking_setShowChanges
                         nargs: 1, [NSNumber numberWithBool: showChanges]];
}

- (void)setTrackChanges:(BOOL)trackChanges
{
    [self.js executeJavaScript: ChangeTracking_setTrackChanges
                         nargs: 1, [NSNumber numberWithBool: trackChanges]];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           JSClipboard                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSClipboard

// The copy operation is called clipboardCopy to keep the static analyzer happy, because it thinks
// that it relates to making a copy of the object and reports a memory leak. The cut operation
// is named similarly just for consistency.

- (NSDictionary *)clipboardCut
{
    return [self.js executeJavaScriptJSON: Clipboard_cut nargs: 0];
}

- (NSDictionary *)clipboardCopy
{
    return [self.js executeJavaScriptJSON: Clipboard_copy nargs: 0];
}

- (void)pasteHTML:(NSString *)html
{
    [self.js executeJavaScript: Clipboard_pasteHTML nargs: 1, html];
}

- (void)pasteText:(NSString *)text
{
    [self.js executeJavaScript: Clipboard_pasteText nargs: 1, text];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSCursor                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Cursor.js

@implementation JSCursor

- (NSString *)positionCursorX:(int)x y:(int)y wordBoundary:(BOOL)wordBoundary
{
    return [self.js executeJavaScript: Cursor_positionCursor nargs: 3,
            [NSNumber numberWithInt: x], [NSNumber numberWithInt: y],
            [NSNumber numberWithBool: wordBoundary]];
}

- (CGRect)getCursorPosition
{
    NSDictionary *result = [self.js executeJavaScriptJSON: Cursor_getCursorPosition nargs: 0];
    return CGRectMake(NSDictionaryGetInt(result,@"x"),
                      NSDictionaryGetInt(result,@"y"),
                      NSDictionaryGetInt(result,@"width"),
                      NSDictionaryGetInt(result,@"height"));
}

- (void)moveLeft
{
    [self.js executeJavaScript: Cursor_moveLeft nargs: 0];
}

- (void)moveRight
{
    [self.js executeJavaScript: Cursor_moveRight nargs: 0];
}

- (void)moveToStartOfDocument
{
    [self.js executeJavaScript: Cursor_moveToStartOfDocument nargs: 0];
}

- (void)moveToEndOfDocument
{
    [self.js executeJavaScript: Cursor_moveToEndOfDocument nargs: 0];
}

- (void)insertReference:(NSString *)itemId
{
    [self.js executeJavaScript: Cursor_insertReference nargs: 1, itemId];
}

- (void)insertLinkWithText:(NSString *)text URL:(NSString *)URL
{
    [self.js executeJavaScript: Cursor_insertLink nargs: 2, text, URL];
}

- (void)insertCharacter:(unichar)character allowInvalidPos:(BOOL)allowInvalidPos
{
    NSString *str = [NSString stringWithFormat: @"%C", character];
    [self.js executeJavaScript: Cursor_insertCharacter nargs: 2, str,
     [NSNumber numberWithBool: allowInvalidPos]];
}

- (void)deleteCharacter
{
    [self.js executeJavaScript: Cursor_deleteCharacter nargs: 0];
}

- (void)enterPressed
{
    [self.js executeJavaScript: Cursor_enterPressed nargs: 0];
}

- (NSString *)getPrecedingWord
{
    return [self.js executeJavaScript: Cursor_getPrecedingWord nargs: 0];
}

- (NSDictionary *)getLinkProperties
{
    return [self.js executeJavaScriptJSON: Cursor_getLinkProperties nargs: 0];
}

- (void)setLinkProperties:(NSDictionary *)properties
{
    [self.js executeJavaScript: Cursor_setLinkProperties nargs: 1, properties];
}

- (void)setReferenceTarget:(NSString *)itemId
{
    [self.js executeJavaScript: Cursor_setReferenceTarget nargs: 1, itemId];
}

- (void)insertFootnote:(NSString *)content
{
    [self.js executeJavaScript: Cursor_insertFootnote nargs: 1, content];
}

- (void)insertEndnote:(NSString *)content
{
    [self.js executeJavaScript: Cursor_insertEndnote nargs: 1, content];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            Equations                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSEquations

- (void)insertEquation
{
    [self.js executeJavaScript: Equations_insertEquation nargs: 0];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSFigures                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSFigures

- (void)insertFigure:(NSString *)filename width:(NSString *)width
            numbered:(BOOL)numbered caption:(NSString *)caption
{
    [self.js executeJavaScript: Figures_insertFigure nargs: 4,
     filename, width, [NSNumber numberWithBool: numbered], caption];
}

- (NSString *)getSelectedFigureId
{
    return [self.js executeJavaScript: Figures_getSelectedFigureId nargs: 0];
}

- (NSDictionary *)getProperties:(NSString *)itemId
{
    return [self.js executeJavaScriptJSON: Figures_getProperties nargs: 1, itemId];
}

- (void)setProperties:(NSString *)itemId width:(NSString *)width src:(NSString *)src
{
    [self.js executeJavaScript: Figures_setProperties nargs: 3, itemId, width, src];
}

- (NSDictionary *)getGeometry:(NSString *)itemId
{
    return [self.js executeJavaScriptJSON: Figures_getGeometry nargs: 1, itemId];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          JSFormatting                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Formatting.js

@implementation JSFormatting

- (NSDictionary *)getFormatting
{
    return [self.js executeJavaScriptJSON: Formatting_getFormatting nargs: 0];
}

- (void)applyFormattingChangesStyle:(NSString *)style properties:(NSDictionary *)properties
{
    [self.js executeJavaScript: Formatting_applyFormattingChanges nargs: 2, style, properties];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             JSInput                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSInput

- (void)removePosition:(int)posId
{
    [self.js executeJavaScript: Input_removePosition nargs: 1, [NSNumber numberWithInt: posId]];
}

- (NSString *)textInRangeStartId:(int)startId startAdjust:(int)startAdjust
                           endId:(int)endId endAdjust:(int)endAdjust
{
    return [self.js executeJavaScript: Input_textInRange nargs: 4,
            [NSNumber numberWithInt: startId],
            [NSNumber numberWithInt: startAdjust],
            [NSNumber numberWithInt: endId],
            [NSNumber numberWithInt: endAdjust]];
}

- (void)replaceRangeStart:(int)startId end:(int)endId withText:(NSString *)text
{
    [self.js executeJavaScript: Input_replaceRange nargs: 3,
     [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId], text];
}

- (NSDictionary *)selectedTextRange
{
    return [self.js executeJavaScriptJSON: Input_selectedTextRange nargs: 0];
}

- (void)setSelectedTextRangeStart:(int)startId end:(int)endId
{
    [self.js executeJavaScript: Input_setSelectedTextRange nargs: 2,
     [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId]];
}

- (NSDictionary *)markedTextRange
{
    return [self.js executeJavaScriptJSON: Input_markedTextRange nargs: 0];
}

- (void)setMarkedText:(NSString *)text startOffset:(int)startOffset endOffset:(int)endOffset
{
    [self.js executeJavaScript: Input_setMarkedText nargs: 3,
     text, [NSNumber numberWithInt: startOffset], [NSNumber numberWithInt: endOffset]];
}

- (void)unmarkText
{
    [self.js executeJavaScript: Input_unmarkText nargs: 0];
}

- (BOOL)forwardSelectionAffinity
{
    NSString *result = [self.js executeJavaScript: Input_forwardSelectionAffinity nargs: 0];
    return [result isEqualToString: @"true"];
}

- (void)setForwardSelectionAffinity:(BOOL)forwardSelectionAffinity
{
    [self.js executeJavaScript: Input_setForwardSelectionAffinity nargs: 1,
     [NSNumber numberWithBool: forwardSelectionAffinity]];
}

- (int)positionFromPosition:(int)posId offset:(int)offset
{
    NSString *result = [self.js executeJavaScript: Input_positionFromPositionOffset nargs: 2,
                        [NSNumber numberWithInt: posId], [NSNumber numberWithInt: offset]];
    return result.intValue;
}

- (int)positionFromPosition:(int)posId inDirection:(NSString *)direction offset:(int)offset
{
    NSString *result = [self.js executeJavaScript: Input_positionFromPositionInDirectionOffset nargs: 3,
                        [NSNumber numberWithInt: posId], direction, [NSNumber numberWithInt: offset]];
    return result.intValue;
}

- (int)comparePosition:(int)positionId toPosition:(int)otherId
{
    NSString *result = [self.js executeJavaScript: Input_comparePositionToPosition nargs: 2,
                        [NSNumber numberWithInt: positionId], [NSNumber numberWithInt: otherId]];
    return result.intValue;
}

- (int)offsetFromPosition:(int)fromPosition toPosition:(int)toPosition
{
    NSString *result = [self.js executeJavaScript: Input_offsetFromPositionToPosition nargs: 2,
                        [NSNumber numberWithInt: fromPosition], [NSNumber numberWithInt: toPosition]];
    return result.intValue;
}

- (int)positionWithinRangeStart:(int)startId end:(int)endId farthestInDirection:(NSString *)direction
{
    NSString *result = [self.js executeJavaScript: Input_positionWithinRangeFarthestInDirection nargs: 3,
                        [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId], direction];
    return result.intValue;
}

- (NSDictionary *)characterRangeByExtendingPosition:(int)positionId inDirection:(NSString *)direction
{
    return [self.js executeJavaScriptJSON: Input_characterRangeByExtendingPositionInDirection nargs: 2,
            [NSNumber numberWithInt: positionId], direction];
}

- (NSDictionary *)firstRectForRangeStart:(int)startId end:(int)endId
{
    return [self.js executeJavaScriptJSON: Input_firstRectForRange nargs: 2,
            [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId]];
}

- (NSDictionary *)caretRectForPosition:(int)posId
{
    return [self.js executeJavaScriptJSON: Input_caretRectForPosition nargs: 1,
            [NSNumber numberWithInt: posId]];
}

- (int)closestPositionToPointX:(int)x y:(int)y
{
    NSString *result = [self.js executeJavaScript: Input_closestPositionToPoint nargs: 2,
                        [NSNumber numberWithInt: x], [NSNumber numberWithInt: y]];
    return result.intValue;
}

- (int)closestPositionToPointX:(int)x y:(int)y withinRangeStart:(int)startId end:(int)endId
{
    NSString *result = [self.js executeJavaScript: Input_closestPositionToPointWithinRange nargs: 4,
                        [NSNumber numberWithInt: x], [NSNumber numberWithInt: y],
                        [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId]];
    return result.intValue;
}

- (NSDictionary *)characterRangeAtPointX:(int)x y:(int)y
{
    return [self.js executeJavaScriptJSON: Input_characterRangeAtPoint nargs: 2,
            [NSNumber numberWithInt: x], [NSNumber numberWithInt: y]];
}

- (int)positionWithinRangeStart:(int)startId end:(int)endId atCharacterOffset:(int)offset
{
    NSString *result = [self.js executeJavaScript: Input_positionWithinRangeAtCharacterOffset nargs: 3,
                        [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId],
                        [NSNumber numberWithInt: offset]];
    return result.intValue;
}

- (int)characterOffsetOfPosition:(int)positionId withinRangeStart:(int)startId end:(int)endId
{
    NSString *result = [self.js executeJavaScript: Input_characterOffsetOfPositionWithinRange nargs: 3,
                        [NSNumber numberWithInt: positionId],
                        [NSNumber numberWithInt: startId], [NSNumber numberWithInt: endId]];
    return result.intValue;
}

// UITextInputTokenizer methods

- (BOOL)isPosition:(int)posId atBoundary:(NSString *)granularity inDirection:(NSString *)direction
{
    NSString *result = [self.js executeJavaScript: Input_isPositionAtBoundaryGranularityInDirection nargs: 3,
                        [NSNumber numberWithInt: posId], granularity, direction];
    return [result isEqualToString: @"true"];
}

- (BOOL)isPosition:(int)posId withinTextUnit:(NSString *)granularity inDirection:(NSString *)direction
{
    NSString *result = [self.js executeJavaScript: Input_isPositionWithinTextUnitInDirection nargs: 3,
                        [NSNumber numberWithInt: posId], granularity, direction];
    return [result isEqualToString: @"true"];
}

- (int)positionFromPosition:(int)posId toBoundary:(NSString *)granularity inDirection:(NSString *)direction
{
    NSString *result = [self.js executeJavaScript: Input_positionFromPositionToBoundaryInDirection nargs: 3,
                        [NSNumber numberWithInt: posId], granularity, direction];
    return result.intValue;
}

- (NSDictionary *)rangeEnclosingPosition:(int)posId withGranularity:(NSString *)granularity inDirection:(NSString *)direction
{
    return [self.js executeJavaScriptJSON: Input_rangeEnclosingPositionWithGranularityInDirection nargs: 3,
            [NSNumber numberWithInt: posId], granularity, direction];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             JSLists                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Lists.js

@implementation JSLists

- (void)increaseIndent
{
    [self.js executeJavaScript: Lists_increaseIndent nargs: 0];
}

- (void)decreaseIndent
{
    [self.js executeJavaScript: Lists_decreaseIndent nargs: 0];
}

- (void)clearList
{
    [self.js executeJavaScript: Lists_clearList nargs: 0];
}

- (void)setUnorderedList
{
    [self.js executeJavaScript: Lists_setUnorderedList nargs: 0];
}

- (void)setOrderedList
{
    [self.js executeJavaScript: Lists_setOrderedList nargs: 0];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             JSMain                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Main.js

@implementation JSMain

- (NSString *)getLanguage
{
    return [self.js executeJavaScript: Main_getLanguage nargs: 0];
}

- (void)setLanguage:(NSString *)language
{
    [self.js executeJavaScript: Main_setLanguage nargs: 1, language];
}

- (NSString *)setGenerator:(NSString *)generator
{
    return [self.js executeJavaScript: Main_setGenerator nargs: 1, generator];
}

- (BOOL)prepareForSave
{
    NSString *result = [self.js executeJavaScript: Main_prepareForSave nargs: 0];
    return [@"true" isEqualToString: result];
}

- (NSString *)getHTML;
{
    return [self.js executeJavaScript: Main_getHTML nargs: 0];
}

- (BOOL)isEmptyDocument
{
    NSString *result = [self.js executeJavaScript: Main_isEmptyDocument nargs: 0];
    return [result isEqualToString: @"true"];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           JSMetadata                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSMetadata

- (NSDictionary *)getMetadata
{
    return [self.js executeJavaScriptJSON: Metadata_getMetadata nargs: 0];
}

- (void)setMetadata:(NSDictionary *)metadata
{
    [self.js executeJavaScriptJSON: Metadata_setMetadata nargs: 1, metadata];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSOutline                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Outline.js

@implementation JSOutline

- (NSDictionary *)getOutline
{
    return [self.js executeJavaScriptJSON: Outline_getOutline nargs: 0];
}

- (void)moveSection:(NSString *)sectionId parentId:(NSString *)parentId nextId:(NSString *)nextId
{
    [self.js executeJavaScript: Outline_moveSection nargs: 3, sectionId, parentId, nextId];
}

- (void)deleteItem:(NSString *)itemId
{
    [self.js executeJavaScript: Outline_deleteItem nargs: 1, itemId];
}

- (void)scheduleUpdateStructure
{
    [self.js executeJavaScript: Outline_scheduleUpdateStructure nargs: 0];
}

- (void)goToItem:(NSString *)itemId
{
    [self.js executeJavaScript: Outline_goToItem nargs: 1, itemId];
}

- (void)set:(NSString *)itemId numbered:(BOOL)numbered
{
    [self.js executeJavaScript: Outline_setNumbered nargs: 2, itemId, [NSNumber numberWithBool: numbered]];
}

- (void)set:(NSString *)itemId title:(NSString *)title
{
    [self.js executeJavaScript: Outline_setTitle nargs: 2, itemId, title];
}

- (void)insertTableOfContents
{
    [self.js executeJavaScript: Outline_insertTableOfContents nargs: 0];
}

- (void)insertListOfFigures
{
    [self.js executeJavaScript: Outline_insertListOfFigures nargs: 0];
}

- (void)insertListOfTables
{
    [self.js executeJavaScript: Outline_insertListOfTables nargs: 0];
}

- (void)setPrintMode:(BOOL)printMode
{
    [self.js executeJavaScript: Outline_setPrintMode nargs: 1, [NSNumber numberWithBool: printMode]];
}

- (NSDictionary *)examinePrintLayout:(int)pageHeight
{
    return [self.js executeJavaScriptJSON: Outline_examinePrintLayout nargs: 1,
            [NSNumber numberWithInt: pageHeight]];
}

- (BOOL)detectSectionNumbering
{
    NSString *result = [self.js executeJavaScript: Outline_detectSectionNumbering nargs: 0];
    return [result isEqualToString: @"true"];
}

- (NSDictionary *)findUsedStyles
{
    return [self.js executeJavaScriptJSON: Outline_findUsedStyles nargs: 0];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSPreview                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSPreview

- (void)showForStyle:(NSString *)styleId uiName:(NSString *)uiName title:(NSString *)title
{
    [self.js executeJavaScript: Preview_showForStyle nargs: 3, styleId, uiName, title];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             JSScan                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSScan

- (void)reset
{
    [self.js executeJavaScript: Scan_reset nargs: 0];
}

- (EDScanParagraph *)next
{
    NSDictionary *dict = [self.js executeJavaScriptJSON: Scan_next nargs: 0];
    if ((dict == nil) || ![dict isKindOfClass: [NSDictionary class]])
        return nil;

    NSString *text = FCGetString(dict,@"text",nil);
    NSString *sectionId = FCGetString(dict,@"sectionId",nil);
    if (text == nil)
        return nil;

    return [[EDScanParagraph alloc] initWithText: text sectionId: sectionId];
}

- (int)addMatchStart:(int)start end:(int)end
{
    return [self.js executeJavaScript: Scan_addMatch nargs: 2,
            [NSNumber numberWithInt: start],
            [NSNumber numberWithInt: end]].intValue;
}

- (void)showMatch:(int)matchId
{
    [self.js executeJavaScript: Scan_showMatch nargs: 1, [NSNumber numberWithInt: matchId]];
}

- (void)replaceMatch:(int)matchId with:(NSString *)text
{
    [self.js executeJavaScript: Scan_replaceMatch nargs: 2, [NSNumber numberWithInt: matchId], text];
}

- (void)removeMatch:(int)matchId
{
    [self.js executeJavaScript: Scan_removeMatch nargs: 1, [NSNumber numberWithInt: matchId]];
}

- (void)goToMatch:(int)matchId
{
    [self.js executeJavaScript: Scan_goToMatch nargs: 1, [NSNumber numberWithInt: matchId]];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           JSSelection                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Selection.js

@implementation JSSelection

- (void)update
{
    [self.js executeJavaScript: Selection_update nargs: 0];
}

- (void)selectAll
{
    [self.js executeJavaScript: Selection_selectAll nargs: 0];
}

- (void)selectParagraph
{
    [self.js executeJavaScript: Selection_selectParagraph nargs: 0];
}

- (void)selectWordAtCursor
{
    [self.js executeJavaScript: Selection_selectWordAtCursor nargs: 0];
}

- (NSString *)dragSelectionBeginX:(int)x y:(int)y selectWord:(BOOL)selectWord
{
    return [self.js executeJavaScript: Selection_dragSelectionBegin nargs: 3,
            [NSNumber numberWithInt: x], [NSNumber numberWithInt: y],
            [NSNumber numberWithBool: selectWord]];
}

- (NSString *)dragSelectionUpdateX:(int)x y:(int)y selectWord:(BOOL)selectWord
{
    return [self.js executeJavaScript: Selection_dragSelectionUpdate nargs: 3,
            [NSNumber numberWithInt: x], [NSNumber numberWithInt: y],
            [NSNumber numberWithBool: selectWord]];
}

- (NSString *)moveStartLeft
{
    return [self.js executeJavaScript: Selection_moveStartLeft nargs: 0];
}

- (NSString *)moveStartRight
{
    return [self.js executeJavaScript: Selection_moveStartRight nargs: 0];
}

- (NSString *)moveEndLeft
{
    return [self.js executeJavaScript: Selection_moveEndLeft nargs: 0];
}

- (NSString *)moveEndRight
{
    return [self.js executeJavaScript: Selection_moveEndRight nargs: 0];
}

- (void)setSelectionStartAtCoordsX:(int)x y:(int)y
{
    [self.js executeJavaScript: Selection_setSelectionStartAtCoords nargs: 2,
                             [NSNumber numberWithInt: x], [NSNumber numberWithInt: y]];
}

- (void)setSelectionEndAtCoordsX:(int)x y:(int)y
{
    [self.js executeJavaScript: Selection_setSelectionEndAtCoords nargs: 2,
                             [NSNumber numberWithInt: x], [NSNumber numberWithInt: y]];
}

- (void)setTableSelectionEdge:(NSString *)edge atCoordsX:(int)x y:(int)y
{
    [self.js executeJavaScript: Selection_setTableSelectionEdgeAtCoords nargs: 3,
                           edge, [NSNumber numberWithInt: x], [NSNumber numberWithInt: y]];
}

- (void)print
{
    [self.js executeJavaScript: Selection_print nargs: 0];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSStyles                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSStyles

- (NSString *)getCSSText
{
    return [self.js executeJavaScript: Styles_getCSSText nargs: 0];
}

- (void)setCSSText:(NSString *)cssText rules:(NSDictionary *)rules
{
//    debug(@"---------------------- setCSSText ----------------------\n");
//    debug(@"%@",cssText);
//    if ((cssText.length > 0) && ([cssText characterAtIndex: cssText.length-1] != '\n'))
//        debug(@"\n");
//    debug(@"--------------------------------------------------------\n");
    [self.js executeJavaScriptJSON: Styles_setCSSText nargs: 2, cssText, rules];
}

- (NSString *)paragraphClass
{
    return [self.js executeJavaScript: Styles_getParagraphClass nargs: 0];
}

- (void)setParagraphClass:(NSString *)paragraphClass;
{
    [self.js executeJavaScript: Styles_setParagraphClass nargs: 1, paragraphClass];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            JSTables                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Tables.js

@implementation JSTables

- (void)insertTableRows:(int)rows cols:(int)cols width:(NSString *)width numbered:(BOOL)numbered
                caption:(NSString *)caption className:(NSString *)className;
{
    [self.js executeJavaScript: Tables_insertTable nargs: 6,
                             [NSNumber numberWithInt: rows], [NSNumber numberWithInt: cols],
                             width, [NSNumber numberWithBool: numbered], caption, className];
}

- (void)addAdjacentRow
{
    [self.js executeJavaScript: Tables_addAdjacentRow nargs: 0];
}

- (void)addAdjacentColumn
{
    [self.js executeJavaScript: Tables_addAdjacentColumn nargs: 0];
}

- (void)removeAdjacentRow
{
    [self.js executeJavaScript: Tables_removeAdjacentRow nargs: 0];
}

- (void)removeAdjacentColumn
{
    [self.js executeJavaScript: Tables_removeAdjacentColumn nargs: 0];
}

- (void)clearCells
{
    [self.js executeJavaScript: Tables_clearCells nargs: 0];
}

- (void)mergeCells
{
    [self.js executeJavaScript: Tables_mergeCells nargs: 0];
}

- (void)splitSelection
{
    [self.js executeJavaScript: Tables_splitSelection nargs: 0];
}

- (NSString *)getSelectedTableId
{
    return [self.js executeJavaScript: Tables_getSelectedTableId nargs: 0];
}

- (NSDictionary *)getProperties:(NSString *)itemId
{
    return [self.js executeJavaScriptJSON: Tables_getProperties nargs: 1, itemId];
}

- (void)setProperties:(NSString *)itemId width:(NSString *)width
{
    [self.js executeJavaScript: Tables_setProperties nargs: 2, itemId, width];
}

- (void)set:(NSString *)itemId colWidths:(NSArray *)colWidths
{
    [self.js executeJavaScript: Tables_setColWidths nargs: 2, itemId, colWidths];
}

- (NSDictionary *)getGeometry:(NSString *)itemId
{
    return [self.js executeJavaScriptJSON: Tables_getGeometry nargs: 1, itemId];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          JSUndoManager                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation JSUndoManager

- (int)getLength
{
    NSString *result = [self.js executeJavaScript: UndoManager_getLength nargs: 0];
    return result.intValue;
}

- (int)getIndex
{
    NSString *result = [self.js executeJavaScript: UndoManager_getIndex nargs: 0];
    return result.intValue;
}

- (void)setIndex:(int)index
{
    [self.js executeJavaScript: UndoManager_setIndex nargs: 1, [NSNumber numberWithInt: index]];
}

- (void)undo
{
    [self.js executeJavaScript: UndoManager_undo nargs: 0];
}

- (void)redo
{
    [self.js executeJavaScript: UndoManager_redo nargs: 0];
}

- (void)newGroup:(NSString *)name
{
    [self.js executeJavaScript: UndoManager_newGroup nargs: 1, name];
}

- (NSString *)groupType
{
    return [self.js executeJavaScript: UndoManager_groupType nargs: 0];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           JSViewport                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Functions implemented in Viewport.js

@implementation JSViewport

- (void)setViewportWidth:(int)width
{
    [self.js executeJavaScript: Viewport_setViewportWidth nargs: 1, [NSNumber numberWithInt: width]];
}

- (void)setTextScale:(int)textScale
{
    [self.js executeJavaScript: Viewport_setTextScale nargs: 1, [NSNumber numberWithInt: textScale]];
}

@end
