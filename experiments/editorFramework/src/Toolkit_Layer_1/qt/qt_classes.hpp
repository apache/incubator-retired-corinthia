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
#include "../toolkit.hpp"
#include <QtWidgets/QWidget>


/*
 * qt_classes.hpp all the graphical classes inherited from Qt
 * Typically each class would have its own header file, but that
 * would also mean 1 MOC file pr class. Having all Qt definitions
 * in 1 files gives a couple of advantages
 * - only a single moc_qt_classes.cpp file
 * - All qt definitions in one file makes the overview easier
 *
 * The current graphical layout will change once we get a graphical 
 * designer to take a look.
 */


class QPushButton;
class QHBoxLayout;

class Toolbar : public QWidget
{
    Q_OBJECT
public:
    Toolbar();
    virtual ~Toolbar();

    QPushButton *saveButton;
    QPushButton *saveAsButton;
    QPushButton *loadButton;
    QHBoxLayout *_layout;
};









class Editor;
class EditorJSEvaluator;
class QWebView;
class JSInterface;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             Cursor                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class Cursor : public QWidget
{
    Q_OBJECT
public:
    Cursor(QWidget *parent = 0);
    virtual ~Cursor();

protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             Editor                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////
class EditorJSCallbacks;

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~Editor();
    QWebView *webView() const { return _webView; }
    JSInterface *js() const { return _js; }
    Cursor *cursor() const { return _cursor; }

    public slots:
    void webViewloadFinished(bool ok);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QWebView *_webView;
    EditorJSCallbacks *_callbacks;
    EditorJSEvaluator *_evaluator;
    JSInterface *_js;
    bool _selecting;
    Cursor *_cursor;
};







////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           MainWindow                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////
class MainWindow : public QWidget
{
    /* Main window, this adds all other widgets inside */
    Q_OBJECT
public:
    // Constructor/Destructor
    MainWindow();
    ~MainWindow() {};


    Toolbar toolbar;
    Editor  editor;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           qt_toolkit                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////
class qt_toolkit : QObject, toolkit
{
    /* toolkit qt implementation */
    Q_OBJECT


public:
    // Constructor/Destructor
    qt_toolkit(toolkit_callback *setCallback, int setDebugLevel);
    ~qt_toolkit() {};


    // Inherited functions (accessible from the generic layer
    bool startWindow();
    void run();
    bool callJavascript(const char *function);


public slots:
    void save();
    void saveAs();
    void load();

 
private:
    static QApplication *app;
    MainWindow           window;
    toolkit_callback    *callback;
    int                  debugLevel;
};
