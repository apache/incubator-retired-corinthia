//
//  EDJSInterface.h
//  Editor
//
//  Created by Peter Kelly on 22/11/11.
//  Copyright (c) 2011-2014 UX Productivity Pty Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@class JSAutoCorrect;
@class JSChangeTracking;
@class JSClipboard;
@class JSCursor;
@class JSEquations;
@class JSFigures;
@class JSFormatting;
@class JSInput;
@class JSLists;
@class JSMain;
@class JSMetadata;
@class JSOutline;
@class JSPreview;
@class JSScan;
@class JSSelection;
@class JSStyles;
@class JSTables;
@class JSUndoManager;
@class JSViewport;
@class EDScanParagraph;

@interface JSError : NSError

@property (copy, readonly) NSString *type;
@property (copy, readonly) NSString *message;
@property (copy, readonly) NSString *operation;
@property (copy, readonly) NSString *html;

@end

@protocol JSInterfaceDelegate

- (void)jsAddOutlineItem:(NSString *)itemId type:(NSString *)type title:(NSString *)title;
- (void)jsUpdateOutlineItem:(NSString *)itemId title:(NSString *)title;
- (void)jsRemoveOutlineItem:(NSString *)itemId;
- (void)jsOutlineUpdated;
- (void)jsSetCursorX:(int)x y:(int)y width:(int)width height:(int)height;
- (void)jsSetSelectionHandlesX1:(int)x1 y1:(int)y1 height1:(int)height1
                             x2:(int)x2 y2:(int)y2 height2:(int)height2;
- (void)jsSetTableSelectionX:(int)x y:(int)y width:(int)width height:(int)height;
- (void)jsSetSelectionBoundsLeft:(int)left top:(int)top right:(int)right bottom:(int)bottom;
- (void)jsClearSelectionHandlesAndCursor;
- (void)jsUpdateAutoCorrect;

@end

@protocol JSEvaluator

- (NSString *)stringByEvaluatingJavaScriptFromString:(NSString *)script;

@end

@interface JSInterface : NSObject

@property (weak) NSObject<JSEvaluator> *evaluator;
@property (weak) NSObject<JSInterfaceDelegate> *delegate;
@property (assign) BOOL jsInitialised;
@property (strong) JSAutoCorrect *autoCorrect;
@property (strong) JSChangeTracking *changeTracking;
@property (strong) JSClipboard *clipboard;
@property (strong) JSCursor *cursor;
@property (strong) JSEquations *equations;
@property (strong) JSFigures *figures;
@property (strong) JSFormatting *formatting;
@property (strong) JSInput *input;
@property (strong) JSLists *lists;
@property (strong) JSMain *main;
@property (strong) JSMetadata *metadata;
@property (strong) JSOutline *outline;
@property (strong) JSPreview *preview;
@property (strong) JSScan *scan;
@property (strong) JSSelection *selection;
@property (strong) JSStyles *styles;
@property (strong) JSTables *tables;
@property (strong) JSUndoManager *undoManager;
@property (strong) JSViewport *viewport;
@property (copy) NSString *currentOperation;
@property (strong) JSError *error;
@property (assign) BOOL documentModified;

- (JSInterface *)initWithEvaluator:(NSObject<JSEvaluator> *)evaluator;
- (BOOL)initJavaScriptWidth:(int)width textScale:(int)textScale cssURL:(NSString *)cssURL
             clientRectsBug:(BOOL)clientRectsBug;
- (void)printStatistics;

@end

@interface JSModule : NSObject

@property (assign) JSInterface *js;

- (JSModule *)initWithJS:(JSInterface *)js;

@end

// Functions implemented in AutoCorrect.js

@interface JSAutoCorrect : JSModule

- (void)correctPreceding:(int)numChars word:(NSString *)replacement confirmed:(BOOL)confirmed;
- (NSDictionary *)getCorrection;
- (NSDictionary *)getCorrectionCoords;
- (void)acceptCorrection;
- (void)replaceCorrection:(NSString *)replacement;

@end

// Functions implemented in ChangeTracking.js

@interface JSChangeTracking : JSModule
- (BOOL)showChanges;
- (BOOL)trackChanges;
- (void)setShowChanges:(BOOL)showChanges;
- (void)setTrackChanges:(BOOL)trackChanges;
@end

// Functions implemented in Clipboard.js

@interface JSClipboard : JSModule
- (NSDictionary *)clipboardCut;
- (NSDictionary *)clipboardCopy;
- (void)pasteHTML:(NSString *)html;
- (void)pasteText:(NSString *)text;
@end

// Functions implemented in Cursor.js

@interface JSCursor : JSModule
- (NSString *)positionCursorX:(int)x y:(int)y wordBoundary:(BOOL)wordBoundary;
- (CGRect)getCursorPosition;
- (void)moveLeft;
- (void)moveRight;
- (void)moveToStartOfDocument;
- (void)moveToEndOfDocument;
- (void)insertReference:(NSString *)itemId;
- (void)insertLinkWithText:(NSString *)text URL:(NSString *)URL;
- (void)insertCharacter:(unichar)character allowInvalidPos:(BOOL)allowInvalidPos;
- (void)deleteCharacter;
- (void)enterPressed;
- (NSString *)getPrecedingWord;
- (NSDictionary *)getLinkProperties;
- (void)setLinkProperties:(NSDictionary *)properties;
- (void)setReferenceTarget:(NSString *)itemId;
- (void)insertFootnote:(NSString *)content;
- (void)insertEndnote:(NSString *)content;
@end

// Functions implemented in Equations.js

@interface JSEquations : JSModule
- (void)insertEquation;
@end

// Functions implemented in Figures.js

@interface JSFigures : JSModule
- (void)insertFigure:(NSString *)filename width:(NSString *)width
            numbered:(BOOL)numbered caption:(NSString *)caption;
- (NSString *)getSelectedFigureId;
- (NSDictionary *)getProperties:(NSString *)itemId;
- (void)setProperties:(NSString *)itemId width:(NSString *)width src:(NSString *)src;
- (NSDictionary *)getGeometry:(NSString *)itemId;
@end

// Functions implemented in Formatting.js

@interface JSFormatting : JSModule
- (NSDictionary *)getFormatting;
- (void)applyFormattingChangesStyle:(NSString *)style properties:(NSDictionary *)properties;
@end

// Functions implemented in Input.js

@interface JSInput : JSModule
- (void)removePosition:(int)posId;

// UITextInput methods
- (NSString *)textInRangeStartId:(int)startId startAdjust:(int)startAdjust
                           endId:(int)endId endAdjust:(int)endAdjust;
- (void)replaceRangeStart:(int)startId end:(int)endId withText:(NSString *)text;
- (NSDictionary *)selectedTextRange;
- (void)setSelectedTextRangeStart:(int)startId end:(int)endId;
- (NSDictionary *)markedTextRange;
- (void)setMarkedText:(NSString *)text startOffset:(int)startOffset endOffset:(int)endOffset;
- (void)unmarkText;
- (BOOL)forwardSelectionAffinity;
- (void)setForwardSelectionAffinity:(BOOL)forwardSelectionAffinity;
- (int)positionFromPosition:(int)posId offset:(int)offset;
- (int)positionFromPosition:(int)posId inDirection:(NSString *)direction offset:(int)offset;
- (int)comparePosition:(int)positionId toPosition:(int)otherId;
- (int)offsetFromPosition:(int)fromPosition toPosition:(int)toPosition;
- (int)positionWithinRangeStart:(int)startId end:(int)endId farthestInDirection:(NSString *)direction;
- (NSDictionary *)characterRangeByExtendingPosition:(int)positionId inDirection:(NSString *)direction;
- (NSDictionary *)firstRectForRangeStart:(int)startId end:(int)endId;
- (NSDictionary *)caretRectForPosition:(int)posId;
- (int)closestPositionToPointX:(int)x y:(int)y;
- (int)closestPositionToPointX:(int)x y:(int)y withinRangeStart:(int)startId end:(int)endId;
- (NSDictionary *)characterRangeAtPointX:(int)x y:(int)y;
- (int)positionWithinRangeStart:(int)startId end:(int)endId atCharacterOffset:(int)offset;
- (int)characterOffsetOfPosition:(int)positionId withinRangeStart:(int)startId end:(int)endId;

// UITextInputTokenizer methods
- (BOOL)isPosition:(int)posId atBoundary:(NSString *)granularity inDirection:(NSString *)direction;
- (BOOL)isPosition:(int)posId withinTextUnit:(NSString *)granularity inDirection:(NSString *)direction;
- (int)positionFromPosition:(int)posId toBoundary:(NSString *)granularity inDirection:(NSString *)direction;
- (NSDictionary *)rangeEnclosingPosition:(int)posId withGranularity:(NSString *)granularity inDirection:(NSString *)direction;
@end

// Functions implemented in Lists.js

@interface JSLists : JSModule
- (void)increaseIndent;
- (void)decreaseIndent;
- (void)clearList;
- (void)setUnorderedList;
- (void)setOrderedList;
@end

// Functions implemented in Main.js

@interface JSMain : JSModule
- (NSString *)getLanguage;
- (void)setLanguage:(NSString *)language;
- (NSString *)setGenerator:(NSString *)generator;
- (BOOL)prepareForSave;
- (NSString *)getHTML;
- (BOOL)isEmptyDocument;
@end

// Functions implemented in Metadata.js

@interface JSMetadata : JSModule
- (NSDictionary *)getMetadata;
- (void)setMetadata:(NSDictionary *)metadata;
@end

// Functions implemented in Outline.js

@interface JSOutline : JSModule
- (NSDictionary *)getOutline;
- (void)moveSection:(NSString *)sectionId parentId:(NSString *)parentId nextId:(NSString *)nextId;
- (void)deleteItem:(NSString *)itemId;
- (void)goToItem:(NSString *)itemId;
- (void)scheduleUpdateStructure;
- (void)set:(NSString *)itemId numbered:(BOOL)numbered;
- (void)set:(NSString *)itemId title:(NSString *)title;
- (void)insertTableOfContents;
- (void)insertListOfFigures;
- (void)insertListOfTables;
- (void)setPrintMode:(BOOL)printMode;
- (NSDictionary *)examinePrintLayout:(int)pageHeight;
- (BOOL)detectSectionNumbering;
- (NSDictionary *)findUsedStyles;
@end

// Functions implemented in Preview.js

@interface JSPreview : JSModule
- (void)showForStyle:(NSString *)styleId uiName:(NSString *)uiName title:(NSString *)title;
@end

// Functions implemented in Scan.js

@interface JSScan : JSModule
- (void)reset;
- (EDScanParagraph *)next;
- (int)addMatchStart:(int)start end:(int)end;
- (void)showMatch:(int)matchId;
- (void)replaceMatch:(int)matchId with:(NSString *)text;
- (void)removeMatch:(int)matchId;
- (void)goToMatch:(int)matchId;
@end

// Functions implemented in Selection.js

@interface JSSelection : JSModule
- (void)update;
- (void)selectAll;
- (void)selectParagraph;
- (void)selectWordAtCursor;
- (NSString *)dragSelectionBeginX:(int)x y:(int)y selectWord:(BOOL)selectWord;
- (NSString *)dragSelectionUpdateX:(int)x y:(int)y selectWord:(BOOL)selectWord;
- (NSString *)moveStartLeft;
- (NSString *)moveStartRight;
- (NSString *)moveEndLeft;
- (NSString *)moveEndRight;
- (void)setSelectionStartAtCoordsX:(int)x y:(int)y;
- (void)setSelectionEndAtCoordsX:(int)x y:(int)y;
- (void)setTableSelectionEdge:(NSString *)edge atCoordsX:(int)x y:(int)y;
- (void)print;
@end

// Functions implemented in Styles.js

@interface JSStyles : JSModule
- (NSString *)getCSSText;
- (void)setCSSText:(NSString *)cssText rules:(NSDictionary *)rules;
- (NSString *)paragraphClass;
- (void)setParagraphClass:(NSString *)paragraphClass;
@end

// Functions implemented in Tables.js

@interface JSTables : JSModule
- (void)insertTableRows:(int)rows cols:(int)cols width:(NSString *)width numbered:(BOOL)numbered
                caption:(NSString *)caption className:(NSString *)className;
- (void)addAdjacentRow;
- (void)addAdjacentColumn;
- (void)removeAdjacentRow;
- (void)removeAdjacentColumn;
- (void)clearCells;
- (void)mergeCells;
- (void)splitSelection;
- (NSString *)getSelectedTableId;
- (NSDictionary *)getProperties:(NSString *)itemId;
- (void)setProperties:(NSString *)itemId width:(NSString *)width;
- (void)set:(NSString *)itemId colWidths:(NSArray *)colWidths;
- (NSDictionary *)getGeometry:(NSString *)itemId;
@end

// Functions implemented in UndoManager.js

@interface JSUndoManager : JSModule
- (int)getLength;
- (int)getIndex;
- (void)setIndex:(int)index;
- (void)undo;
- (void)redo;
- (void)newGroup:(NSString *)name;
- (NSString *)groupType;
@end

// Functions implemented in Viewport.js

@interface JSViewport : JSModule
- (void)setViewportWidth:(int)width;
- (void)setTextScale:(int)textScale;
@end
