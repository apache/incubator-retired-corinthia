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

#include "Editor.h"
#include "JSInterface.h"
#include <QWebView>
#include <QWebFrame>
#include <QFile>
#include <QLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCoreApplication>
#include <QMouseEvent>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             Cursor                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

Cursor::Cursor(QWidget *parent)
{
}

Cursor::~Cursor()
{
}

void Cursor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(0,0,size().width(),size().height(),QColor(0,0,255));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EditorJSCallbacks                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class EditorJSCallbacks : public JSCallbacks
{
public:
    EditorJSCallbacks(Editor *editor) : _editor(editor) {}
    void debug(const QString &message);
    void addOutlineItem(const QString &itemId, const QString &type, const QString &title);
    void updateOutlineItem(const QString &itemId, const QString &title);
    void removeOutlineItem(const QString &itemId);
    void outlineUpdated();
    void setCursor(int x, int y, int width, int height);
    void setSelectionHandles(int x1, int y1, int height1, int x2, int y2, int height2);
    void setTableSelection(int x, int y, int width, int height);
    void setSelectionBounds(int left, int top, int right, int bottom);
    void clearSelectionHandlesAndCursor();
    void updateAutoCorrect();
    void error(const QString &message, const QString &operation);
//private:
    Editor *_editor;
};

class EditorJSEvaluator : public JSEvaluator
{
public:
    EditorJSEvaluator(QWebView *webView, JSCallbacks *callbacks) : _webView(webView), _callbacks(callbacks) {}
    virtual QString evaluate(const QString &script);
    virtual JSCallbacks *callbacks() { return _callbacks; }
private:
    QWebView *_webView;
    JSCallbacks *_callbacks;
};



const char *jsSources[] = {
    "first.js",
    "ElementTypes.js",
    "AutoCorrect.js",
    "ChangeTracking.js",
    "Clipboard.js",
    "Cursor.js",
    "DOM.js",
    "Editor.js",
    "Equations.js",
    "Figures.js",
    "Formatting.js",
    "Hierarchy.js",
    "Input.js",
    "Lists.js",
    "Main.js",
    "Metadata.js",
    "NodeSet.js",
    "Outline.js",
    "Position.js",
    "PostponedActions.js",
    "Preview.js",
    "Range.js",
    "Scan.js",
    "Selection.js",
    "StringBuilder.js",
    "Styles.js",
    "Tables.js",
    "Text.js",
    "traversal.js",
    "types.js",
    "UndoManager.js",
    "util.js",
    "Viewport.js",
    NULL,
};




void EditorJSCallbacks::debug(const QString &message)
{
    qStdOut() << "CB debug \"" << message << "\"" << endl;
}

void EditorJSCallbacks::addOutlineItem(const QString &itemId, const QString &type, const QString &title)
{
    qStdOut() << "CB addOutlineItem " << itemId << " " << type << " \"" << title << "\"" << endl;
}

void EditorJSCallbacks::updateOutlineItem(const QString &itemId, const QString &title)
{
    qStdOut() << "CB updateOutlineItem " << itemId << " \"" << title << "\"" << endl;
}

void EditorJSCallbacks::removeOutlineItem(const QString &itemId)
{
    qStdOut() << "CB removeOutlineItem " << itemId << endl;
}

void EditorJSCallbacks::outlineUpdated()
{
    qStdOut() << "CB outlineUpdated" << endl;
}

void EditorJSCallbacks::setCursor(int x, int y, int width, int height)
{
    qStdOut() << "CB setCursor " << x << " " << y << " " << width << " " << height << endl;
    _editor->cursor()->setVisible(true);
    _editor->cursor()->setGeometry(x,y,width,height);
}

void EditorJSCallbacks::setSelectionHandles(int x1, int y1, int height1, int x2, int y2, int height2)
{
    qStdOut() << "CB setSelectionHandles " << x1 << " " << y1 << " " << height1 << " "
    << x2 << " " << y2 << " " << height2 << endl;
    _editor->cursor()->setVisible(false);
}

void EditorJSCallbacks::setTableSelection(int x, int y, int width, int height)
{
    qStdOut() << "CB setTableSelection" << x << " " << y << " " << width << " " << height << endl;
    _editor->cursor()->setVisible(false);
}

void EditorJSCallbacks::setSelectionBounds(int left, int top, int right, int bottom)
{
    qStdOut() << "CB setSelectionBounds " << left << " " << top << " " << right << " " << bottom << endl;
}

void EditorJSCallbacks::clearSelectionHandlesAndCursor()
{
    qStdOut() << "CB clearSelectionHandlesAndCursor" << endl;
}

void EditorJSCallbacks::updateAutoCorrect()
{
    qStdOut() << "CB updateAutoCorrect" << endl;
}

void EditorJSCallbacks::error(const QString &message, const QString &operation)
{
    qStdOut() << "CB error \"" << message << "\" \"" << operation << "\"" << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EditorJSEvaluator                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

QString EditorJSEvaluator::evaluate(const QString &script)
{
    QWebFrame *frame = _webView->page()->mainFrame();
    QVariant result = frame->evaluateJavaScript(script);
    if (result.userType() != QMetaType::QString)
        return QString::null;
    else
        return result.toString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             Editor                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

Editor::Editor(QWidget *parent, Qt::WindowFlags f) : QWidget(parent,f)
{
    _webView = new QWebView(this);
    _callbacks = new EditorJSCallbacks(this);
    _evaluator = new EditorJSEvaluator(_webView,_callbacks);
    _js = new JSInterface(_evaluator);
    _selecting = false;
    _cursor = new Cursor(this);
    _cursor->setHidden(true);

    QObject::connect(_webView,SIGNAL(loadFinished(bool)),this,SLOT(webViewloadFinished(bool)));
    _webView->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(_webView);
    layout->addWidget(_cursor);
    setLayout(layout);
}

Editor::~Editor()
{
    delete _webView;
    delete _callbacks;
    delete _evaluator;
    delete _js;
}

void Editor::webViewloadFinished(bool ok)
{
    qStdOut() << "webViewloadFinished: ok = " << ok << endl;
    if (!ok)
        return;
    QWebFrame *frame = _webView->page()->mainFrame();
//    frame->evaluateJavaScript("alert('This is a test')");

    QString appPath = QCoreApplication::applicationDirPath();
    QString baseDir = appPath + "/../share/corinthia/js";
    baseDir = QUrl::fromLocalFile(baseDir).toLocalFile();
    qStdOut() << "js base dir = " << baseDir << endl;

    for (int i = 0; jsSources[i] != NULL; i++) {
        QString fullPath = baseDir + "/" + QString(jsSources[i]);
        QFile file(fullPath);
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            QString content = stream.readAll();
            frame->evaluateJavaScript(content);
        }
        else {
            qStdOut() << "Can't open file " << fullPath << endl;
            return;
        }
    }

    frame->evaluateJavaScript("Main_init()");

    processCallbacks(_evaluator);
}

void Editor::mouseDoubleClickEvent(QMouseEvent *event)
{
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
    if (_selecting) {
        js()->selection.dragSelectionUpdate(event->x(),event->y(),false);
    }
}

void Editor::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _selecting = true;
        js()->selection.dragSelectionBegin(event->x(),event->y(),false);
    }
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
    _selecting = false;
}

void Editor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    _js->viewport.setViewportWidth(size().width());
}

bool Editor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _webView) {
        switch (event->type()) {
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
            case QEvent::MouseTrackingChange:
                this->event(event);
                return true;
            default:
                break;
        }
    }
    return QWidget::eventFilter(obj,event);
}
