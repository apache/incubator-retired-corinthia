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

#pragma once

#include <QString>
#include <QRect>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>

/**
 * \file JSInterface.h
 *
 * C++ interface to the JavaScript editor library
 *
 * All of the core editing operations for Corinthia are implemented in the Editor library, which is
 * written in JavaScript. This library can be used either from within a web browser, or, in the case
 * of the Qt application, from an embedded web view. For this app, we use a QWebView instance which
 * maintains the in-memory DOM tree of the document, and has injected into it all of the javascript
 * code that is part of the editor library.
 *
 * The source code of the Editor library lives in (repository-root)/Editor/src. During build of the
 * Qt app, all the javascript files are copied into (build-dir)/share/corinthia/js. If you wish to
 * make changes to the javascript code, you should modify the files in the former location, as the
 * latter files will be overwritten on every build.
 *
 * The purpose of JSInterface.h and JSInterface.cpp is to provide a C++ wrapper over this. All of
 * the methods defined in the classes below (with the exception of callbacks) ultimately result in a
 * call to QWebFrame's evaluateJavaScript() method. See the documentation for JSInterface.cpp for
 * details.
 *
 * The editor library is divided into *modules*, each of which implements a specific aspect of
 * editing functionality. For example, the Cursor module contains methods for moving the cursor
 * around the document, and inserting or deleting text at the current cursor position. Similarly,
 * the Tables module contains methods for inserting, deleting, and modifying tables. A separate C++
 * class is defined for each module, and an instance of each class is maintained by the "container"
 * class, JSInterface. When using the code here, you should do so via a JSInterface instance.
 */

#define JS_MODULE_COMMON(className) \
Q_DISABLE_COPY(className) \
public: \
className(JSEvaluator *evaluator) : JSModule(evaluator) {}

QTextStream& qStdOut();
QString QRectString(QRect rect);

/**
 * Callback interface
 *
 * While the module classes are for making calls from C++ to JavaScript, the JSCallbacks abstract
 * class is for responding to requests from JavaScript to invoke C++ code. This is declared here as
 * an abstract class (that is, with all methods virtual and no implementations provided) to avoid
 * dependencies between the code in this file and other parts of the application. The
 * EditorJSCallbacks class in Editor.cpp provides a concrete implementation of this, which is where
 * the actual callback functions are implemented.
 *
 * Callbacks are always invoked *after* the execution of a particular editor library API function,
 * not during. The reason for this design design in the library was to enable support for web view
 * classes that did not provide native support for callbacks (as was the case for iOS, at least at
 * the time the library was originally written).
 *
 * The way that callbacks are invoked is that after each editor API call, a query is performed for a
 * list of pending callback messages. The evaluation logic iterates through these and invokes the
 * appropriate callback method for each. For this reason, callbacks method are all 'void' - they
 * never return a value. Callbacks are for notification purposes only - typically telling the
 * application to update the UI in some manner.
 */
class JSCallbacks
{
public:
    virtual ~JSCallbacks() {}
    virtual void debug(const QString &message) = 0;
    virtual void addOutlineItem(const QString &itemId, const QString &type, const QString &title) = 0;
    virtual void updateOutlineItem(const QString &itemId, const QString &title) = 0;
    virtual void removeOutlineItem(const QString &itemId) = 0;
    virtual void outlineUpdated() = 0;
    virtual void setCursor(int x, int y, int width, int height) = 0;
    virtual void setSelectionHandles(int x1, int y1, int height1, int x2, int y2, int height2) = 0;
    virtual void setTableSelection(int x, int y, int width, int height) = 0;
    virtual void setSelectionBounds(int left, int top, int right, int bottom) = 0;
    virtual void clearSelectionHandlesAndCursor() = 0;
    virtual void updateAutoCorrect() = 0;
    virtual void error(const QString &message, const QString &operation) = 0;
};

/**
 * The JSEvaluator abstract class provides an evaluate() method which is called (indirectly) by all
 * of the individual module methods. As with JSCallbacks, it is defined as abstract to avoid a
 * dependency on the code outside of this file. The EditorJSEvaluator class in Editor.cpp provides a
 * concrete implementation of this; its evaluate() method simply calls through to the
 * evaluateJavaScript() method of QWebView.
 *
 * JSEvaluator also has a callbacks() method, which must return an instance of JSCallbacks. This
 * makes JSEvaluator the "central point of contact" between the JavaScript interface and the rest of
 * the application, in that it provides the necessary access to call *in* to javascript, and to
 * respond (via callbacks) to calls *out* of javascript. Upon initialisation of a document window,
 * concrete implementations of both JSCallbacks and JSEvaluator are created, the latter maintaining
 * a reference to the former. See Editor::Editor() for where ths is actually done.
 */
class JSEvaluator
{
public:
    virtual ~JSEvaluator() {}
    virtual QString evaluate(const QString &script) = 0;
    virtual JSCallbacks *callbacks() = 0;
};

class JSAutoCorrect;
class JSChangeTracking;
class JSClipboard;
class JSCursor;
class JSEquations;
class JSFigures;
class JSFormatting;
class JSInput;
class JSLists;
class JSMain;
class JSMetadata;
class JSOutline;
class JSPreview;
class JSScan;
class JSSelection;
class JSStyles;
class JSTables;
class JSUndoManager;
class JSViewport;
class EDScanParagraph;

class JSError
{
public:
    const QString &type() { return _type; }
    const QString &message() { return _message; }
    const QString &operation() { return _operation; }
    const QString &html() { return _html; }

private:

    QString _type;
    QString _message;
    QString _operation;
    QString _html;
};

class JSModule
{
    Q_DISABLE_COPY(JSModule)
public:
    JSModule(JSEvaluator *evaluator) : _evaluator(evaluator) {};
protected:
    JSEvaluator *_evaluator;
};

// Functions implemented in AutoCorrect.js

class JSAutoCorrect : public JSModule
{
    JS_MODULE_COMMON(JSAutoCorrect)
    void correctPrecedingWord(int numChars, const QString &replacement, bool confirmed);
    QJsonObject getCorrection();
    QJsonObject getCorrectionCoords();
    void acceptCorrection();
    void replaceCorrection(const QString &replacement);
};

// Functions implemented in ChangeTracking.js

class JSChangeTracking : public JSModule
{
    JS_MODULE_COMMON(JSChangeTracking)
    bool showChanges();
    bool trackChanges();
    void setShowChanges(bool showChanges);
    void setTrackChanges(bool trackChanges);
};

// Functions implemented in Clipboard.js

class JSClipboard : public JSModule
{
    JS_MODULE_COMMON(JSClipboard)
    QJsonObject clipboardCut();
    QJsonObject clipboardCopy();
    void pasteHTML(const QString &html);
    void pasteText(const QString &text);
};

// Functions implemented in Cursor.js

class JSCursor : public JSModule
{
    JS_MODULE_COMMON(JSCursor)
    QString positionCursor(int x, int y, bool wordBoundary);
    QRect getCursorPosition();
    void moveLeft();
    void moveRight();
    void moveToStartOfDocument();
    void moveToEndOfDocument();
    void insertReference(const QString &itemId);
    void insertLink(const QString &text, const QString &url);
    void insertCharacter(unsigned short character, bool allowInvalidPos);
    void deleteCharacter();
    void enterPressed();
    QString getPrecedingWord();
    QJsonObject getLinkProperties();
    void setLinkProperties(QJsonObject properties);
    void setReferenceTarget(const QString &itemId);
    void insertFootnote(const QString &content);
    void insertEndnote(const QString &content);
};

// Functions implemented in Equations.js

class JSEquations : public JSModule
{
    JS_MODULE_COMMON(JSEquations)
    void insertEquation();
};

// Functions implemented in Figures.js

class JSFigures : public JSModule
{
    JS_MODULE_COMMON(JSFigures)
    void insertFigure(const QString &filename, const QString &width,
                      bool numbered, const QString &caption);
    QString getSelectedFigureId();
    QJsonObject getProperties(const QString &itemId);
    void setProperties(const QString &itemId, const QString &width, const QString &src);
    QJsonObject getGeometry(const QString &itemId);
};

// Functions implemented in Formatting.js

class JSFormatting : public JSModule
{
    JS_MODULE_COMMON(JSFormatting)
    QJsonObject getFormatting();
    void applyFormattingChanges(const QString &style, QJsonObject properties);
};

// Functions implemented in Input.js

class JSInput : public JSModule
{
    JS_MODULE_COMMON(JSInput)
    void removePosition(int posId);

    QString textInRange(int startId, int startAdjust, int endId, int endAdjust);
    void replaceRange(int startId, int endId, const QString &text);
    QJsonObject selectedTextRange();
    void setSelectedTextRange(int startId, int endId);
    QJsonObject markedTextRange();
    void setMarkedText(const QString &text, int startOffset, int endOffset);
    void unmarkText();
    bool forwardSelectionAffinity();
    void setForwardSelectionAffinity(bool forwardSelectionAffinity);
    int positionFromPositionOffset(int posId, int offset);
    int positionFromPositionInDirectionOffset(int posId, const QString &direction, int offset);
    int comparePositionToPosition(int positionId, int otherId);
    int offsetFromPositionToPosition(int fromPosition, int toPosition);
    int positionWithinRangeFarthestInDirection(int startId, int endId, const QString &direction);
    QJsonObject characterRangeByExtendingPositionInDirection(int positionId, const QString &direction);
    QJsonObject firstRectForRange(int startId, int endId);
    QJsonObject caretRectForPosition(int posId);
    int closestPositionToPoint(int x, int y);
    int closestPositionToPointWithinRange(int x, int y, int startId, int endId);
    QJsonObject characterRangeAtPoint(int x, int y);
    int positionWithinRangeAtCharacterOffset(int startId, int endId, int offset);
    int characterOffsetOfPositionWithinRange(int positionId, int startId, int endId);

    bool isPositionAtBoundaryGranularityInDirection(int posId, const QString &granularity,
                                                    const QString &direction);
    bool isPositionWithinTextUnitInDirection(int posId, const QString &granularity,
                                             const QString &direction);
    int positionFromPositionToBoundaryInDirection(int posId, const QString &granularity,
                                                  const QString &direction);
    QJsonObject rangeEnclosingPositionWithGranularityInDirection(int posId,
                                                                 const QString &granularity,
                                                                 const QString &direction);
};

// Functions implemented in Lists.js

class JSLists : public JSModule
{
    JS_MODULE_COMMON(JSLists)
    void increaseIndent();
    void decreaseIndent();
    void clearList();
    void setUnorderedList();
    void setOrderedList();
};

// Functions implemented in Main.js

class JSMain : public JSModule
{
    JS_MODULE_COMMON(JSMain)
    QString getLanguage();
    void setLanguage(const QString &language);
    QString setGenerator(const QString &generator);
    bool prepareForSave();
    QString getHTML();
    bool isEmptyDocument();
};

// Functions implemented in Metadata.js

class JSMetadata : public JSModule
{
    JS_MODULE_COMMON(JSMetadata)
    QJsonObject getMetadata();
    void setMetadata(const QJsonObject &metadata);
};

// Functions implemented in Outline.js

class JSOutline : public JSModule
{
    JS_MODULE_COMMON(JSOutline)
    QJsonObject getOutline();
    void moveSection(const QString &sectionId, const QString &parentId, const QString &nextId);
    void deleteItem(const QString &itemId);
    void goToItem(const QString &itemId);
    void scheduleUpdateStructure();
    void setNumbered(const QString &itemId, bool numbered);
    void setTitle(const QString &itemId, const QString &title);
    void insertTableOfContents();
    void insertListOfFigures();
    void insertListOfTables();
    void setPrintMode(bool printMode);
    QJsonObject examinePrintLayout(int pageHeight);
    bool detectSectionNumbering();
    QJsonObject findUsedStyles();
};

// Functions implemented in Preview.js

class JSPreview : public JSModule
{
    JS_MODULE_COMMON(JSPreview)
    void showForStyle(const QString &styleId, const QString &uiName, const QString &title);
};

// Functions implemented in Scan.js

class JSScan : public JSModule
{
    JS_MODULE_COMMON(JSScan)
    void reset();
    EDScanParagraph *next();
    int addMatch(int start, int end);
    void showMatch(int matchId);
    void replaceMatch(int matchId, const QString &text);
    void removeMatch(int matchId);
    void goToMatch(int matchId);
};

// Functions implemented in Selection.js

class JSSelection : public JSModule
{
    JS_MODULE_COMMON(JSSelection)
    void update();
    void selectAll();
    void selectParagraph();
    void selectWordAtCursor();
    QString dragSelectionBegin(int x, int y, bool selectWord);
    QString dragSelectionUpdate(int x, int y, bool selectWord);
    QString moveStartLeft();
    QString moveStartRight();
    QString moveEndLeft();
    QString moveEndRight();
    void setSelectionStartAtCoords(int x, int y);
    void setSelectionEndAtCoords(int x, int y);
    void setTableSelectionEdgeAtCoords(const QString &edge, int x, int y);
    void print();
};

// Functions implemented in Styles.js

class JSStyles : public JSModule
{
    JS_MODULE_COMMON(JSStyles)
    QString getCSSText();
    void setCSSText(const QString &cssText, const QJsonObject &rules);
    QString paragraphClass();
    void setParagraphClass(const QString &paragraphClass);
};

// Functions implemented in Tables.js

class JSTables : public JSModule
{
    JS_MODULE_COMMON(JSTables)
    void insertTable(int rows, int cols, const QString &width, bool numbered,
                     const QString &caption, const QString &className);
    void addAdjacentRow();
    void addAdjacentColumn();
    void removeAdjacentRow();
    void removeAdjacentColumn();
    void clearCells();
    void mergeCells();
    void splitSelection();
    QString getSelectedTableId();
    QJsonObject getProperties(const QString &itemId);
    void setProperties(const QString &itemId, const QString &width);
    void setColWidths(const QString &itemId, const QJsonArray &colWidths);
    QJsonObject getGeometry(const QString &itemId);
};

// Functions implemented in UndoManager.js

class JSUndoManager : public JSModule
{
    JS_MODULE_COMMON(JSUndoManager)
    int getLength();
    int getIndex();
    void setIndex(int index);
    void undo();
    void redo();
    void newGroup(const QString &name);
    QString groupType();
};

// Functions implemented in Viewport.js

class JSViewport : public JSModule
{
    JS_MODULE_COMMON(JSViewport)
    void setViewportWidth(int width);
    void setTextScale(int textScale);
};

// All modules

class JSInterface
{
    Q_DISABLE_COPY(JSInterface)
public:
    JSInterface(JSEvaluator *evaluator);
    JSAutoCorrect autoCorrect;
    JSChangeTracking changeTracking;
    JSClipboard clipboard;
    JSCursor cursor;
    JSEquations equations;
    JSFigures figures;
    JSFormatting formatting;
    JSInput input;
    JSLists lists;
    JSMain main;
    JSMetadata metadata;
    JSOutline outline;
    JSPreview preview;
    JSScan scan;
    JSSelection selection;
    JSStyles styles;
    JSTables tables;
    JSUndoManager undoManager;
    JSViewport viewport;
};

void processCallbacks(JSEvaluator *evaluator);
