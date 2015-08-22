// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
#pragma once
#include <QtWidgets/QApplication>
#include "qt_classes.hpp"


/*
 * Implementation of
 * Interface between implementation dependent toolkit and the API available for the handling.
 *
 * This file implements the smallest possible interface, in order to facilitate easier
 * implementation of other toolkits.
 *
 * The Callbacks are handled through the signal/post function of Qt.
 *
 */



// Static Variables
QApplication * qt_toolkit::app;



// Constructor/Destructor
qt_toolkit::qt_toolkit(toolkit_callback *setCallback, int setDebugLevel) :
    window(),
    callback(setCallback),
    debugLevel(setDebugLevel)
{
    // get notification, when user click on button
    QObject::connect((const QObject *)&window.toolbar.saveButton,   SIGNAL(clicked()),          this, SLOT(saveButton()));
    QObject::connect((const QObject *)&window.toolbar.saveAsButton, SIGNAL(clicked()),          this, SLOT(saveAsButton()));
    QObject::connect((const QObject *)&window.toolbar.loadButton,   SIGNAL(clicked()),          this, SLOT(saveAsButton()));
    QObject::connect((const QObject *)&window.editor.webView,       SIGNAL(loadFinished(bool)), this, SLOT(webViewloadFinished(bool)));
}


// Instanciate the derived class.
toolkit * toolkit::createInstance(toolkit_callback *tk, int setDebugLevel) {
    // Application is only added once 
    if (!qt_toolkit::app) {
        int    argc = 0;
        char **argv = NULL;
        qt_toolkit::app = new QApplication(argc, argv);
    }
    return (toolkit *)new qt_toolkit(tk, setDebugLevel);
}


// Prepare graphic (not used in the Qt implementation)
bool qt_toolkit::startWindow() {
    return true;
}


// Sart message loop, and to not return
void qt_toolkit::run() {
    window.show();
    app->exec();
}


// Activate Javascript function
bool qt_toolkit::callJavascript(const char *function) {
    return true;
}


// Notify save was requested
void qt_toolkit::save() {
}


// Notify saveAs was requested
void qt_toolkit::saveAs() {
}


// Notify load was requested
void qt_toolkit::load() {
}


// Notify load was done
void qt_toolkit::webViewloadFinished(bool ok) {
}