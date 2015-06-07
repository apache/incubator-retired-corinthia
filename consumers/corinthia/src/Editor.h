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

#include <QWidget>

class Editor;
class EditorJSCallbacks;
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

class Editor : public QWidget
{
    Q_OBJECT
public:
    Editor(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~Editor();
    QWebView *webView() const;
    JSInterface *js() const;
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
