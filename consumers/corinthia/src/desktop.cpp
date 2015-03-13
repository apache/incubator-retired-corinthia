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
#include <QtWebKit/QWebElement>
#include <QtWebKitWidgets/QWebFrame>
#include "editWindows.h"



DesktopWindow::DesktopWindow(QWidget *parent)
              : QWidget(parent)
{
    setupUi(this);
}



void DesktopWindow::on_elementLineEdit_returnPressed()
{
    QWebFrame *frame               = webView->page()->mainFrame();
    QWebElement document           = frame->documentElement();
    QWebElementCollection elements = document.findAll(elementLineEdit->text());

    foreach (QWebElement element, elements)
        element.setAttribute("style", "background-color: #f0f090");
}



void DesktopWindow::on_highlightButton_clicked()
{
    on_elementLineEdit_returnPressed();
}



void DesktopWindow::setUrl(const QUrl &url)
{
    webView->setUrl(url);
}
