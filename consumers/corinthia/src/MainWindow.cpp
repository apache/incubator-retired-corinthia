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

#include "MainWindow.h"
#include <QWebView.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCoreApplication>
#include "Editor.h"
#include "JSInterface.h"
#include "Toolbar.h"

MainWindow::MainWindow(QApplication *app) : QWidget(0)
{
    _app = app;
    _toolbar = new Toolbar(this);
    _editor = new Editor(this);
    QVBoxLayout *vlayout = new QVBoxLayout();
    this->setLayout(vlayout);
    vlayout->addWidget(_toolbar);
    vlayout->addWidget(_editor);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(4,4,4,4);

    QObject::connect(_toolbar->tableButton(),SIGNAL(clicked()),this,SLOT(insertTable()));
    QObject::connect(_toolbar->linkButton(),SIGNAL(clicked()),this,SLOT(insertLink()));
    QObject::connect(_toolbar->characterButton(),SIGNAL(clicked()),this,SLOT(insertCharacter()));
    QObject::connect(_toolbar->backspaceButton(),SIGNAL(clicked()),this,SLOT(backspace()));
    QObject::connect(_toolbar->leftButton(),SIGNAL(clicked()),this,SLOT(moveLeft()));
    QObject::connect(_toolbar->rightButton(),SIGNAL(clicked()),this,SLOT(moveRight()));
    QObject::connect(_toolbar->undoButton(),SIGNAL(clicked()),this,SLOT(undo()));
    QObject::connect(_toolbar->redoButton(),SIGNAL(clicked()),this,SLOT(redo()));

    QString appPath = QCoreApplication::applicationDirPath();
    QString docPath = appPath + "/../share/corinthia/sample.html";
    QUrl url = QUrl::fromLocalFile(docPath);
    qStdOut() << "sample document url = " << url.toString() << endl;
    _editor->webView()->load(url);
}

MainWindow::~MainWindow()
{
    delete _toolbar;
    delete _editor;
}

void MainWindow::insertTable()
{
    _editor->js()->tables.insertTable(4,3,"50%",true,"Table caption",QString::null);
}

void MainWindow::insertLink()
{
    _editor->js()->cursor.insertLink("Corinthia website","http://corinthia.incubator.apache.org");
}

void MainWindow::insertCharacter()
{
    _editor->js()->cursor.insertCharacter('X',true);
}

void MainWindow::backspace()
{
    _editor->js()->cursor.deleteCharacter();
}

void MainWindow::moveLeft()
{
    _editor->js()->cursor.moveLeft();
}

void MainWindow::moveRight()
{
    _editor->js()->cursor.moveRight();
}

void MainWindow::undo()
{
    _editor->js()->undoManager.undo();
}

void MainWindow::redo()
{
    _editor->js()->undoManager.redo();
}

//#include <MainWindow.moc>
