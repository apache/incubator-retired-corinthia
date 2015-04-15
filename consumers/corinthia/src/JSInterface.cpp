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

#include "JSInterface.h"
#include <QJsonDocument>
#include <assert.h>

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

class JSArgs
{
public:
    QString toString() const;
    JSArgs &operator<<(int value);
    JSArgs &operator<<(unsigned int value);
    JSArgs &operator<<(double value);
    JSArgs &operator<<(bool value);
    JSArgs &operator<<(const char *value);
    JSArgs &operator<<(const QString &value);
    JSArgs &operator<<(const QJsonArray &value);
    JSArgs &operator<<(const QJsonObject &value);
    JSArgs &operator<<(const QJsonValue &value);
    JSArgs &operator<<(const QRect &value);
    JSArgs &operator<<(const QPoint &value);
    JSArgs &operator<<(const QSize &value);
    JSArgs &operator<<(const QMap<QString,QString> map);
private:
    QJsonArray _array;
};

QString JSArgs::toString() const
{
    QString argsStr = QJsonDocument(_array).toJson(QJsonDocument::Compact);
    assert(argsStr.size() >= 2);
    assert(argsStr[0] == '[');
    assert(argsStr[argsStr.size()-1] == ']');
    argsStr.remove(0,1);
    argsStr.remove(argsStr.size()-1,1);
    return argsStr;
}

JSArgs &JSArgs::operator<<(int value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(unsigned int value)
{
    _array.append((double)value);
    return *this;
}

JSArgs &JSArgs::operator<<(double value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(bool value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const char *value)
{
    if (value == NULL)
        _array.append(QJsonValue::Null);
    else
        _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const QString &value)
{
    if (value.isNull())
        _array.append(QJsonValue::Null);
    else
        _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const QJsonArray &value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const QJsonObject &value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const QJsonValue &value)
{
    _array.append(value);
    return *this;
}

JSArgs &JSArgs::operator<<(const QRect &value)
{
    QJsonObject obj;
    obj["x"] = QJsonValue(value.x());
    obj["y"] = QJsonValue(value.y());
    obj["width"] = QJsonValue(value.width());
    obj["height"] = QJsonValue(value.height());
    _array.append(obj);
    return *this;
}

JSArgs &JSArgs::operator<<(const QPoint &value)
{
    QJsonObject obj;
    obj["x"] = QJsonValue(value.x());
    obj["y"] = QJsonValue(value.y());
    _array.append(obj);
    return *this;
}

JSArgs &JSArgs::operator<<(const QSize &value)
{
    QJsonObject obj;
    obj["width"] = QJsonValue(value.width());
    obj["height"] = QJsonValue(value.height());
    _array.append(obj);
    return *this;
}

JSArgs &JSArgs::operator<<(const QMap<QString,QString> map)
{
    QJsonObject obj;
    QMapIterator<QString,QString> it(map);
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        QString value = it.value();
        if (value.isNull())
            obj[key] = QJsonValue::Null;
        else
            obj[key] = value;
    }
    _array.append(obj);
    return *this;
}

void testargs()
{
    int intVal = 4294967295;
    unsigned int uintVal = 4294967295;
    QMap<QString,QString> map;
    map["color"] = "red";
    map["font-size"] = "24";
    map["font-family"] = "Helvetica";
    map["background-color"] = QString::null;
    JSArgs args = JSArgs()
    << intVal
    << uintVal
    << 3.2
    << true
    << false
    << "test"
    << (char *)NULL
    << QString("test")
    << QString::null
    << QJsonArray()
    << QJsonObject()
    << QJsonValue("value")
    << QRect(1,2,3,4)
    << QPoint(5,6)
    << QSize(7,8)
    << map;
    qStdOut() << args.toString() << endl;
}

QRect QRectFromJson(QJsonValue value)
{
    if (!value.isObject()) {
        qStdOut() << "QRectFromJson: value is not an object" << endl;
        return QRect();
    }

    QJsonObject obj = value.toObject();
    QJsonValue x = obj["x"];
    QJsonValue y = obj["y"];
    QJsonValue width = obj["width"];
    QJsonValue height = obj["height"];
    if (!x.isDouble() || !y.isDouble() || !width.isDouble() || !height.isDouble()) {
        qStdOut() << "QRectFromJson: invalid value(s)" << endl;
        return QRect();
    }

    return QRect(x.toDouble(),y.toDouble(),width.toDouble(),height.toDouble());
}

QString QRectString(QRect rect)
{
    return QString().sprintf("(%d,%d,%d,%d)",rect.x(),rect.y(),rect.width(),rect.height());
}

void processCallbacks(JSEvaluator *evaluator)
{
    JSCallbacks *callbacks = evaluator->callbacks();

    QString messagesStr = evaluator->evaluate("Editor_getBackMessages()");
    if (messagesStr.isNull()) {
        qStdOut() << "Editor_getBackMessages failed" << endl;
        return;
    }

    //    QJsonDocument doc = QJsonDocument::fromVariant(var);
    QJsonDocument doc = QJsonDocument::fromJson(messagesStr.toUtf8());
    if (doc.isNull()) {
        qStdOut() << "JSON parse failed" << endl;
        return;
    }

    if (!doc.isArray()) {
        qStdOut() << "JSON is not an array" << endl;
        return;
    }

    qStdOut() << "JSON parse succeeded; top-level is an array with " << doc.array().size() << " items" << endl;
    QJsonArray topArray = doc.array();
    for (int i = 0; i < topArray.size(); i++) {
        QJsonValue val = topArray.at(i);
        if (!val.isArray()) {
            qStdOut() << "Entry " << i << " is not an array" << endl;
            return;
        }
        //        printf("entry %d: %s\n",i,qcstring(val.toString()));
        QJsonArray array = val.toArray();
        if (array.size() < 1) {
            qStdOut() << "Callback array " << i << " is empty" << endl;
            return;
        }
        if (!array.at(0).isString()) {
            qStdOut() << "Callback array " << i << " does not start with a string" << endl;
            return;
        }
        QString functionName = array.at(0).toString();

        //        if (functionName == "addOutlineItem") {
        //            qStdOut() << "cbName " << i << " = " << functionName << " *************************" << endl;
        //        }
        //        else {
        //            qStdOut() << "cbName " << i << " = " << functionName << endl;
        //        }

        if ((functionName == "debug") &&
            (array.size() == 2) &&
            (array.at(1).isString())) {
            callbacks->debug(array.at(1).toString());
        }
        else if ((functionName == "addOutlineItem") &&
                 (array.size() == 4) &&
                 (array.at(1).isString()) &&
                 (array.at(2).isString()) &&
                 ((array.at(3).isString()) || (array.at(3).isNull()))) {
            callbacks->addOutlineItem(array.at(1).toString(),array.at(2).toString(),array.at(3).toString());
        }
        else if ((functionName == "updateOutlineItem") &&
                 (array.size() == 3) &&
                 array.at(1).isString() &&
                 array.at(2).isString()) {
            callbacks->updateOutlineItem(array.at(1).toString(),array.at(2).toString());
        }
        else if ((functionName == "removeOutlineItem") &&
                 (array.size() == 2) &&
                 array.at(1).isString()) {
            callbacks->removeOutlineItem(array.at(1).toString());
        }
        else if ((functionName == "outlineUpdated") &&
                 (array.size() == 1)) {
            callbacks->outlineUpdated();
        }
        else if ((functionName == "setSelectionHandles") &&
                 (array.size() == 7) &&
                 array.at(1).isDouble() &&
                 array.at(2).isDouble() &&
                 array.at(3).isDouble() &&
                 array.at(4).isDouble() &&
                 array.at(5).isDouble() &&
                 array.at(6).isDouble()) {
            callbacks->setSelectionHandles(array.at(1).toDouble(),
                                            array.at(2).toDouble(),
                                            array.at(3).toDouble(),
                                            array.at(4).toDouble(),
                                            array.at(5).toDouble(),
                                            array.at(6).toDouble());
        }
        else if ((functionName == "setTableSelection") &&
                 (array.size() == 5) &&
                 array.at(1).isDouble() &&
                 array.at(2).isDouble() &&
                 array.at(3).isDouble() &&
                 array.at(4).isDouble()) {
            callbacks->setTableSelection(array.at(1).toDouble(),
                                          array.at(2).toDouble(),
                                          array.at(3).toDouble(),
                                          array.at(4).toDouble());
        }
        else if ((functionName == "setCursor") &&
                 (array.size() == 5) &&
                 array.at(1).isDouble() &&
                 array.at(2).isDouble() &&
                 array.at(3).isDouble() &&
                 array.at(4).isDouble()) {
            callbacks->setCursor(array.at(1).toDouble(),
                                  array.at(2).toDouble(),
                                  array.at(3).toDouble(),
                                  array.at(4).toDouble());
        }
        else if ((functionName == "clearSelectionHandlesAndCursor") &&
                 (array.size() == 1)) {
            callbacks->clearSelectionHandlesAndCursor();
        }
        else if ((functionName == "setSelectionBounds") &&
                 (array.size() == 5) &&
                 array.at(1).isDouble() &&
                 array.at(2).isDouble() &&
                 array.at(3).isDouble() &&
                 array.at(4).isDouble()) {
            callbacks->setSelectionBounds(array.at(1).toDouble(),
                                           array.at(2).toDouble(),
                                           array.at(3).toDouble(),
                                           array.at(4).toDouble());
        }
        else if ((functionName == "updateAutoCorrect") &&
                 (array.size() == 1)) {
            callbacks->updateAutoCorrect();
        }
        else if ((functionName == "error") &&
                 (array.size() == 3) &&
                 array.at(1).isString() &&
                 array.at(2).isString()) {
            callbacks->error(array.at(1).toString(),array.at(2).toString());
        }
        else {
            qStdOut() << "INVALID CALLBACK OR ARGS: " << functionName << endl;
        }
    }
}

QJsonValue evaljsStr(JSEvaluator *ev, const QString &functionName, const QString &argsStr)
{
    QString script;
    QTextStream stream(&script);
    stream << "Main_execute(function() { return JSON.stringify({ result: ";
    stream << functionName << "(";
    stream << argsStr;
    stream << ")}); });";

    qStdOut() << "EVALUATE: " << script << endl;

    QString resultStr = ev->evaluate(script);
    qStdOut() << "RESULT: " << resultStr << endl;
    processCallbacks(ev);
    if (resultStr.isNull())
        return QJsonValue(QJsonValue::Null);

    QJsonDocument doc = QJsonDocument::fromJson(resultStr.toUtf8());
    if (!doc.isObject()) {
        ev->callbacks()->error("Error parsing returned JSON","evaluate");
        return QJsonValue(QJsonValue::Null);
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("result")) {
        return QJsonValue(QJsonValue::Null);
        //        ev->callbacks()->error("Returned JSON does not contain a result","evaluate");
        //        return QJsonValue(QJsonValue::Null);
    }
    
    return obj["result"];
}

QJsonValue evaljs(JSEvaluator *ev, const QString &functionName, const JSArgs &args)
{
    return evaljsStr(ev,functionName,args.toString());
}

// Functions implemented in AutoCorrect.js

void JSAutoCorrect::correctPrecedingWord(int numChars, const QString &replacement, bool confirmed)
{
    JSArgs args = JSArgs() << numChars << replacement << confirmed;
    evaljs(_evaluator,"AutoCorrect_correctPrecedingWord",args);
}

QJsonObject JSAutoCorrect::getCorrection()
{
    QJsonValue result = evaljs(_evaluator,"AutoCorrect_getCorrection",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

QJsonObject JSAutoCorrect::getCorrectionCoords()
{
    QJsonValue result = evaljs(_evaluator,"AutoCorrect_getCorrectionCoords",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSAutoCorrect::acceptCorrection()
{
    evaljs(_evaluator,"AutoCorrect_acceptCorrection",JSArgs());
}

void JSAutoCorrect::replaceCorrection(const QString &replacement)
{
    JSArgs args = JSArgs() << replacement;
    evaljs(_evaluator,"AutoCorrect_replaceCorrection",args);
}

// Functions implemented in ChangeTracking.js

bool JSChangeTracking::showChanges()
{
    QJsonValue result = evaljs(_evaluator,"ChangeTracking_showChanges",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

bool JSChangeTracking::trackChanges()
{
    QJsonValue result = evaljs(_evaluator,"ChangeTracking_trackChanges",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

void JSChangeTracking::setShowChanges(bool showChanges)
{
    JSArgs args = JSArgs() << showChanges;
    evaljs(_evaluator,"ChangeTracking_setShowChanges",args);
}

void JSChangeTracking::setTrackChanges(bool trackChanges)
{
    JSArgs args = JSArgs() << trackChanges;
    evaljs(_evaluator,"ChangeTracking_setTrackChanges",args);
}

// Functions implemented in Clipboard.js

QJsonObject JSClipboard::clipboardCut()
{
    QJsonValue result = evaljs(_evaluator,"Clipboard_cut",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

QJsonObject JSClipboard::clipboardCopy()
{
    QJsonValue result = evaljs(_evaluator,"Clipboard_copy",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSClipboard::pasteHTML(const QString &html)
{
    JSArgs args = JSArgs() << html;
    evaljs(_evaluator,"Clipboard_pasteHTML",args);
}

void JSClipboard::pasteText(const QString &text)
{
    JSArgs args = JSArgs() << text;
    evaljs(_evaluator,"Clipboard_pasteText",args);
}

// Functions implemented in Cursor.js


QString JSCursor::positionCursor(int x, int y, bool wordBoundary)
{
    JSArgs args = JSArgs() << x << y << wordBoundary;
    QJsonValue result = evaljs(_evaluator,"Cursor_positionCursor",args);
    return result.isString() ? result.toString() : QString::null;
}

QRect JSCursor::getCursorPosition()
{
    QJsonValue result = evaljs(_evaluator,"Cursor_getCursorPosition",JSArgs());
    return QRectFromJson(result);
}

void JSCursor::moveLeft()
{
    evaljs(_evaluator,"Cursor_moveLeft",JSArgs());
}

void JSCursor::moveRight()
{
    evaljs(_evaluator,"Cursor_moveRight",JSArgs());
}

void JSCursor::moveToStartOfDocument()
{
    evaljs(_evaluator,"Cursor_moveToStartOfDocument",JSArgs());
}

void JSCursor::moveToEndOfDocument()
{
    evaljs(_evaluator,"Cursor_moveToEndOfDocument",JSArgs());
}

void JSCursor::insertReference(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    evaljs(_evaluator,"Cursor_insertReference",args);
}

void JSCursor::insertLink(const QString &text, const QString &url)
{
    JSArgs args = JSArgs() << text << url;
    evaljs(_evaluator,"Cursor_insertLink",args);
}

void JSCursor::insertCharacter(unsigned short character, bool allowInvalidPos)
{
    JSArgs args = JSArgs() << QString(character) << allowInvalidPos;
    evaljs(_evaluator,"Cursor_insertCharacter",args);
}

void JSCursor::deleteCharacter()
{
    evaljs(_evaluator,"Cursor_deleteCharacter",JSArgs());
}

void JSCursor::enterPressed()
{
    evaljs(_evaluator,"Cursor_enterPressed",JSArgs());
}

QString JSCursor::getPrecedingWord()
{
    QJsonValue result = evaljs(_evaluator,"Cursor_getPrecedingWord",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QJsonObject JSCursor::getLinkProperties()
{
    QJsonValue result = evaljs(_evaluator,"Cursor_getLinkProperties",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSCursor::setLinkProperties(QJsonObject properties)
{
    JSArgs args = JSArgs() << properties;
    evaljs(_evaluator,"Cursor_setLinkProperties",args);
}

void JSCursor::setReferenceTarget(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    evaljs(_evaluator,"Cursor_setReferenceTarget",args);
}

void JSCursor::insertFootnote(const QString &content)
{
    JSArgs args = JSArgs() << content;
    evaljs(_evaluator,"Cursor_insertFootnote",args);
}

void JSCursor::insertEndnote(const QString &content)
{
    JSArgs args = JSArgs() << content;
    evaljs(_evaluator,"Cursor_insertEndnote",args);
}

// Functions implemented in Equations.js

void JSEquations::insertEquation()
{
    evaljs(_evaluator,"Equations_insertEquation",JSArgs());
}

// Functions implemented in Figures.js

void JSFigures::insertFigure(const QString &filename, const QString &width,
                             bool numbered, const QString &caption)
{
    JSArgs args = JSArgs() << filename << width << numbered << caption;
    evaljs(_evaluator,"Figures_insertFigure",args);
}

QString JSFigures::getSelectedFigureId()
{
    QJsonValue result = evaljs(_evaluator,"Figures_getSelectedFigureId",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QJsonObject JSFigures::getProperties(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    QJsonValue result = evaljs(_evaluator,"Figures_getProperties",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSFigures::setProperties(const QString &itemId, const QString &width, const QString &src)
{
    JSArgs args = JSArgs() << itemId << width << src;
    evaljs(_evaluator,"Figures_setProperties",args);
}

QJsonObject JSFigures::getGeometry(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    QJsonValue result = evaljs(_evaluator,"Figures_getGeometry",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

// Functions implemented in Formatting.js

QJsonObject JSFormatting::getFormatting()
{
    QJsonValue result = evaljs(_evaluator,"Formatting_getFormatting",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSFormatting::applyFormattingChanges(const QString &style, QJsonObject properties)
{
    JSArgs args = JSArgs() << style << properties;
    evaljs(_evaluator,"Formatting_applyFormattingChanges",args);
}

// Functions implemented in Input.js

void JSInput::removePosition(int posId)
{
    JSArgs args = JSArgs() << posId;
    evaljs(_evaluator,"Input_removePosition",args);
}

QString JSInput::textInRange(int startId, int startAdjust, int endId, int endAdjust)
{
    JSArgs args = JSArgs() << startId << startAdjust << endId << endAdjust;
    QJsonValue result = evaljs(_evaluator,"Input_textInRange",args);
    return result.isString() ? result.toString() : QString::null;
}

void JSInput::replaceRange(int startId, int endId, const QString &text)
{
    JSArgs args = JSArgs() << startId << endId << text;
    evaljs(_evaluator,"Input_replaceRange",args);
}

QJsonObject JSInput::selectedTextRange()
{
    QJsonValue result = evaljs(_evaluator,"Input_selectedTextRange",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSInput::setSelectedTextRange(int startId, int endId)
{
    JSArgs args = JSArgs() << startId << endId;
    evaljs(_evaluator,"Input_setSelectedTextRange",args);
}

QJsonObject JSInput::markedTextRange()
{
    QJsonValue result = evaljs(_evaluator,"Input_markedTextRange",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSInput::setMarkedText(const QString &text, int startOffset, int endOffset)
{
    JSArgs args = JSArgs() << text << startOffset << endOffset;
    evaljs(_evaluator,"Input_setMarkedText",args);
}

void JSInput::unmarkText()
{
    evaljs(_evaluator,"Input_unmarkText",JSArgs());
}

bool JSInput::forwardSelectionAffinity()
{
    QJsonValue result = evaljs(_evaluator,"Input_forwardSelectionAffinity",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

void JSInput::setForwardSelectionAffinity(bool forwardSelectionAffinity)
{
    JSArgs args = JSArgs() << forwardSelectionAffinity;
    evaljs(_evaluator,"Input_setForwardSelectionAffinity",args);
}

int JSInput::positionFromPositionOffset(int posId, int offset)
{
    JSArgs args = JSArgs() << posId << offset;
    QJsonValue result = evaljs(_evaluator,"Input_positionFromPositionOffset",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::positionFromPositionInDirectionOffset(int posId, const QString &direction, int offset)
{
    JSArgs args = JSArgs() << posId << direction << offset;
    QJsonValue result = evaljs(_evaluator,"Input_positionFromPositionInDirectionOffset",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::comparePositionToPosition(int positionId, int otherId)
{
    JSArgs args = JSArgs() << positionId << otherId;
    QJsonValue result = evaljs(_evaluator,"Input_comparePositionToPosition",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::offsetFromPositionToPosition(int fromPosition, int toPosition)
{
    JSArgs args = JSArgs() << fromPosition << toPosition;
    QJsonValue result = evaljs(_evaluator,"Input_offsetFromPositionToPosition",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::positionWithinRangeFarthestInDirection(int startId, int endId, const QString &direction)
{
    JSArgs args = JSArgs() << startId << endId << direction;
    QJsonValue result = evaljs(_evaluator,"Input_positionWithinRangeFarthestInDirection",args);
    return result.isDouble() ? result.toDouble() : 0;
}

QJsonObject JSInput::characterRangeByExtendingPositionInDirection(int positionId, const QString &direction)
{
    JSArgs args = JSArgs() << positionId << direction;
    QJsonValue result = evaljs(_evaluator,"Input_characterRangeByExtendingPositionInDirection",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

QJsonObject JSInput::firstRectForRange(int startId, int endId)
{
    JSArgs args = JSArgs() << startId << endId;
    QJsonValue result = evaljs(_evaluator,"Input_firstRectForRange",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

QJsonObject JSInput::caretRectForPosition(int posId)
{
    JSArgs args = JSArgs() << posId;
    QJsonValue result = evaljs(_evaluator,"Input_caretRectForPosition",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

int JSInput::closestPositionToPoint(int x, int y)
{
    JSArgs args = JSArgs() << x << y;
    QJsonValue result = evaljs(_evaluator,"Input_closestPositionToPoint",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::closestPositionToPointWithinRange(int x, int y, int startId, int endId)
{
    JSArgs args = JSArgs() << x << y << startId << endId;
    QJsonValue result = evaljs(_evaluator,"Input_closestPositionToPointWithinRange",args);
    return result.isDouble() ? result.toDouble() : 0;
}

QJsonObject JSInput::characterRangeAtPoint(int x, int y)
{
    JSArgs args = JSArgs() << x << y;
    QJsonValue result = evaljs(_evaluator,"Input_characterRangeAtPoint",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

int JSInput::positionWithinRangeAtCharacterOffset(int startId, int endId, int offset)
{
    JSArgs args = JSArgs() << startId << endId << offset;
    QJsonValue result = evaljs(_evaluator,"Input_positionWithinRangeAtCharacterOffset",args);
    return result.isDouble() ? result.toDouble() : 0;
}

int JSInput::characterOffsetOfPositionWithinRange(int positionId, int startId, int endId)
{
    JSArgs args = JSArgs() << positionId << startId << endId;
    QJsonValue result = evaljs(_evaluator,"Input_characterOffsetOfPositionWithinRange",args);
    return result.isDouble() ? result.toDouble() : 0;
}

bool JSInput::isPositionAtBoundaryGranularityInDirection(int posId, const QString &granularity,
                                                         const QString &direction)
{
    JSArgs args = JSArgs() << posId << granularity << direction;
    QJsonValue result = evaljs(_evaluator,"Input_isPositionAtBoundaryGranularityInDirection",args);
    return result.isBool() ? result.toBool() : false;
}

bool JSInput::isPositionWithinTextUnitInDirection(int posId, const QString &granularity,
                                                  const QString &direction)
{
    JSArgs args = JSArgs() << posId << granularity << direction;
    QJsonValue result = evaljs(_evaluator,"Input_isPositionWithinTextUnitInDirection",args);
    return result.isBool() ? result.toBool() : false;
}

int JSInput::positionFromPositionToBoundaryInDirection(int posId, const QString &granularity,
                                                       const QString &direction)
{
    JSArgs args = JSArgs() << posId << granularity << direction;
    QJsonValue result = evaljs(_evaluator,"Input_positionFromPositionToBoundaryInDirection",args);
    return result.isDouble() ? result.toDouble() : 0;
}

QJsonObject JSInput::rangeEnclosingPositionWithGranularityInDirection(int posId,
                                                                      const QString &granularity,
                                                                      const QString &direction)
{
    JSArgs args = JSArgs() << posId << granularity << direction;
    QJsonValue result = evaljs(_evaluator,"Input_rangeEnclosingPositionWithGranularityInDirection",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

// Functions implemented in Lists.js

void JSLists::increaseIndent()
{
    evaljs(_evaluator,"Lists_increaseIndent",JSArgs());
}

void JSLists::decreaseIndent()
{
    evaljs(_evaluator,"Lists_decreaseIndent",JSArgs());
}

void JSLists::clearList()
{
    evaljs(_evaluator,"Lists_clearList",JSArgs());
}

void JSLists::setUnorderedList()
{
    evaljs(_evaluator,"Lists_setUnorderedList",JSArgs());
}

void JSLists::setOrderedList()
{
    evaljs(_evaluator,"Lists_setOrderedList",JSArgs());
}

// Functions implemented in Main.js

QString JSMain::getLanguage()
{
    QJsonValue result = evaljs(_evaluator,"Main_getLanguage",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

void JSMain::setLanguage(const QString &language)
{
    JSArgs args = JSArgs() << language;
    evaljs(_evaluator,"Main_setLanguage",args);
}

QString JSMain::setGenerator(const QString &generator)
{
    JSArgs args = JSArgs() << generator;
    QJsonValue result = evaljs(_evaluator,"Main_setGenerator",args);
    return result.isString() ? result.toString() : QString::null;
}

bool JSMain::prepareForSave()
{
    QJsonValue result = evaljs(_evaluator,"Main_prepareForSave",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

QString JSMain::getHTML()
{
    QJsonValue result = evaljs(_evaluator,"Main_getHTML",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

bool JSMain::isEmptyDocument()
{
    QJsonValue result = evaljs(_evaluator,"Main_isEmptyDocument",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

// Functions implemented in Metadata.js

QJsonObject JSMetadata::getMetadata()
{
    QJsonValue result = evaljs(_evaluator,"Metadata_getMetadata",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSMetadata::setMetadata(const QJsonObject &metadata)
{
    JSArgs args = JSArgs() << metadata;
    evaljs(_evaluator,"Metadata_setMetadata",args);
}

// Functions implemented in Outline.js

QJsonObject JSOutline::getOutline()
{
    QJsonValue result = evaljs(_evaluator,"Outline_getOutline",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSOutline::moveSection(const QString &sectionId, const QString &parentId, const QString &nextId)
{
    JSArgs args = JSArgs() << sectionId << parentId << nextId;
    evaljs(_evaluator,"Outline_moveSection",args);
}

void JSOutline::deleteItem(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    evaljs(_evaluator,"Outline_deleteItem",args);
}

void JSOutline::goToItem(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    evaljs(_evaluator,"Outline_goToItem",args);
}

void JSOutline::scheduleUpdateStructure()
{
    evaljs(_evaluator,"Outline_scheduleUpdateStructure",JSArgs());
}

void JSOutline::setNumbered(const QString &itemId, bool numbered)
{
    JSArgs args = JSArgs() << itemId << numbered;
    evaljs(_evaluator,"Outline_setNumbered",args);
}

void JSOutline::setTitle(const QString &itemId, const QString &title)
{
    JSArgs args = JSArgs() << itemId << title;
    evaljs(_evaluator,"Outline_setTitle",args);
}

void JSOutline::insertTableOfContents()
{
    evaljs(_evaluator,"Outline_insertTableOfContents",JSArgs());
}

void JSOutline::insertListOfFigures()
{
    evaljs(_evaluator,"Outline_insertListOfFigures",JSArgs());
}

void JSOutline::insertListOfTables()
{
    evaljs(_evaluator,"Outline_insertListOfTables",JSArgs());
}

void JSOutline::setPrintMode(bool printMode)
{
    JSArgs args = JSArgs() << printMode;
    evaljs(_evaluator,"Outline_setPrintMode",args);
}

QJsonObject JSOutline::examinePrintLayout(int pageHeight)
{
    JSArgs args = JSArgs() << pageHeight;
    QJsonValue result = evaljs(_evaluator,"Outline_examinePrintLayout",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

bool JSOutline::detectSectionNumbering()
{
    QJsonValue result = evaljs(_evaluator,"Outline_detectSectionNumbering",JSArgs());
    return result.isBool() ? result.toBool() : false;
}

QJsonObject JSOutline::findUsedStyles()
{
    QJsonValue result = evaljs(_evaluator,"Outline_findUsedStyles",JSArgs());
    return result.isObject() ? result.toObject() : QJsonObject();
}

// Functions implemented in Preview.js

void JSPreview::showForStyle(const QString &styleId, const QString &uiName, const QString &title)
{
    JSArgs args = JSArgs() << styleId << uiName << title;
    evaljs(_evaluator,"Preview_showForStyle",args);
    // TODO
}

// Functions implemented in Scan.js

void JSScan::reset()
{
    evaljs(_evaluator,"Scan_reset",JSArgs());
}

EDScanParagraph *JSScan::next()
{
    // TODO
    return NULL;
}

int JSScan::addMatch(int start, int end)
{
    JSArgs args = JSArgs() << start << end;
    QJsonValue result = evaljs(_evaluator,"Scan_addMatch",args);
    return result.isDouble() ? result.toDouble() : 0;
}

void JSScan::showMatch(int matchId)
{
    JSArgs args = JSArgs() << matchId;
    evaljs(_evaluator,"Scan_showMatch",args);
}

void JSScan::replaceMatch(int matchId, const QString &text)
{
    JSArgs args = JSArgs() << matchId << text;
    evaljs(_evaluator,"Scan_replaceMatch",args);
}

void JSScan::removeMatch(int matchId)
{
    JSArgs args = JSArgs() << matchId;
    evaljs(_evaluator,"Scan_removeMatch",args);
}

void JSScan::goToMatch(int matchId)
{
    JSArgs args = JSArgs() << matchId;
    evaljs(_evaluator,"Scan_goToMatch",args);
}

// Functions implemented in Selection.js

void JSSelection::update()
{
    evaljs(_evaluator,"Selection_update",JSArgs());
}

void JSSelection::selectAll()
{
    evaljs(_evaluator,"Selection_selectAll",JSArgs());
}

void JSSelection::selectParagraph()
{
    evaljs(_evaluator,"Selection_selectParagraph",JSArgs());
}

void JSSelection::selectWordAtCursor()
{
    evaljs(_evaluator,"Selection_selectWordAtCursor",JSArgs());
}

QString JSSelection::dragSelectionBegin(int x, int y, bool selectWord)
{
    JSArgs args = JSArgs() << x << y << selectWord;
    QJsonValue result = evaljs(_evaluator,"Selection_dragSelectionBegin",args);
    return result.isString() ? result.toString() : QString::null;
}

QString JSSelection::dragSelectionUpdate(int x, int y, bool selectWord)
{
    JSArgs args = JSArgs() << x << y << selectWord;
    QJsonValue result = evaljs(_evaluator,"Selection_dragSelectionUpdate",args);
    return result.isString() ? result.toString() : QString::null;
}

QString JSSelection::moveStartLeft()
{
    QJsonValue result = evaljs(_evaluator,"Selection_moveStartLeft",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QString JSSelection::moveStartRight()
{
    QJsonValue result = evaljs(_evaluator,"Selection_moveStartRight",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QString JSSelection::moveEndLeft()
{
    QJsonValue result = evaljs(_evaluator,"Selection_moveEndLeft",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QString JSSelection::moveEndRight()
{
    QJsonValue result = evaljs(_evaluator,"Selection_moveEndRight",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

void JSSelection::setSelectionStartAtCoords(int x, int y)
{
    JSArgs args = JSArgs() << x << y;
    evaljs(_evaluator,"Selection_setSelectionStartAtCoords",args);
}

void JSSelection::setSelectionEndAtCoords(int x, int y)
{
    JSArgs args = JSArgs() << x << y;
    evaljs(_evaluator,"Selection_setSelectionEndAtCoords",args);
}

void JSSelection::setTableSelectionEdgeAtCoords(const QString &edge, int x, int y)
{
    JSArgs args = JSArgs() << edge << x << y;
    evaljs(_evaluator,"Selection_setTableSelectionEdgeAtCoords",args);
}

void JSSelection::print()
{
    evaljs(_evaluator,"Selection_print",JSArgs());
}

// Functions implemented in Styles.js

QString JSStyles::getCSSText()
{
    QJsonValue result = evaljs(_evaluator,"Styles_getCSSText",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

void JSStyles::setCSSText(const QString &cssText, const QJsonObject &rules)
{
    JSArgs args = JSArgs() << cssText << rules;
    evaljs(_evaluator,"Styles_setCSSText",args);
}

QString JSStyles::paragraphClass()
{
    QJsonValue result = evaljs(_evaluator,"Styles_getParagraphClass",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

void JSStyles::setParagraphClass(const QString &paragraphClass)
{
    JSArgs args = JSArgs() << paragraphClass;
    evaljs(_evaluator,"Styles_setParagraphClass",args);
}

// Functions implemented in Tables.js

void JSTables::insertTable(int rows, int cols, const QString &width, bool numbered,
                           const QString &caption, const QString &className)
{
    JSArgs args = JSArgs() << rows << cols << width << numbered << caption << className;
    evaljs(_evaluator,"Tables_insertTable",args);
}

void JSTables::addAdjacentRow()
{
    evaljs(_evaluator,"Tables_addAdjacentRow",JSArgs());
}

void JSTables::addAdjacentColumn()
{
    evaljs(_evaluator,"Tables_addAdjacentColumn",JSArgs());
}

void JSTables::removeAdjacentRow()
{
    evaljs(_evaluator,"Tables_removeAdjacentRow",JSArgs());
}

void JSTables::removeAdjacentColumn()
{
    evaljs(_evaluator,"Tables_removeAdjacentColumn",JSArgs());
}

void JSTables::clearCells()
{
    evaljs(_evaluator,"Tables_clearCells",JSArgs());
}

void JSTables::mergeCells()
{
    evaljs(_evaluator,"Tables_mergeCells",JSArgs());
}

void JSTables::splitSelection()
{
    evaljs(_evaluator,"Tables_splitSelection",JSArgs());
}

QString JSTables::getSelectedTableId()
{
    QJsonValue result = evaljs(_evaluator,"Tables_getSelectedTableId",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

QJsonObject JSTables::getProperties(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    QJsonValue result = evaljs(_evaluator,"Tables_getProperties",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

void JSTables::setProperties(const QString &itemId, const QString &width)
{
    JSArgs args = JSArgs() << itemId << width;
    evaljs(_evaluator,"Tables_setProperties",args);
}

void JSTables::setColWidths(const QString &itemId, const QJsonArray &colWidths)
{
    JSArgs args = JSArgs() << itemId << colWidths;
    evaljs(_evaluator,"Tables_setColWidths",args);
}

QJsonObject JSTables::getGeometry(const QString &itemId)
{
    JSArgs args = JSArgs() << itemId;
    QJsonValue result = evaljs(_evaluator,"Tables_getGeometry",args);
    return result.isObject() ? result.toObject() : QJsonObject();
}

// Functions implemented in UndoManager.js

int JSUndoManager::getLength()
{
    QJsonValue result = evaljs(_evaluator,"UndoManager_getLength",JSArgs());
    return result.isDouble() ? result.toDouble() : 0;
}

int JSUndoManager::getIndex()
{
    QJsonValue result = evaljs(_evaluator,"UndoManager_getIndex",JSArgs());
    return result.isDouble() ? result.toDouble() : 0;
}

void JSUndoManager::setIndex(int index)
{
    JSArgs args = JSArgs() << index;
    evaljs(_evaluator,"UndoManager_setIndex",args);
}

void JSUndoManager::undo()
{
    evaljs(_evaluator,"UndoManager_undo",JSArgs());
}

void JSUndoManager::redo()
{
    evaljs(_evaluator,"UndoManager_redo",JSArgs());
}

void JSUndoManager::newGroup(const QString &name)
{
    JSArgs args = JSArgs() << name;
    evaljs(_evaluator,"UndoManager_newGroup",args);
}

QString JSUndoManager::groupType()
{
    QJsonValue result = evaljs(_evaluator,"UndoManager_groupType",JSArgs());
    return result.isString() ? result.toString() : QString::null;
}

// Functions implemented in Viewport.js

void JSViewport::setViewportWidth(int width)
{
    JSArgs args = JSArgs() << width;
    evaljs(_evaluator,"Viewport_setViewportWidth",args);
}

void JSViewport::setTextScale(int textScale)
{
    JSArgs args = JSArgs() << textScale;
    evaljs(_evaluator,"Viewport_setTextScale",args);
}

// All modules

JSInterface::JSInterface(JSEvaluator *evaluator)
: autoCorrect(evaluator),
  changeTracking(evaluator),
  clipboard(evaluator),
  cursor(evaluator),
  equations(evaluator),
  figures(evaluator),
  formatting(evaluator),
  input(evaluator),
  lists(evaluator),
  main(evaluator),
  metadata(evaluator),
  outline(evaluator),
  preview(evaluator),
  scan(evaluator),
  selection(evaluator),
  styles(evaluator),
  tables(evaluator),
  undoManager(evaluator),
  viewport(evaluator)
{
}
