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
#include "windows.hpp"
#include "qt_toolkit.hpp"



// Constructor/Destructor
qt_toolkit::qt_toolkit(toolkit_callback *setCallback, int setDebugLevel) {
    int    argc = 0;
    char **argv = NULL;


    callback   = setCallback;
    debugLevel = setDebugLevel;
    app        = new QApplication(argc, argv);
}
qt_toolkit::~qt_toolkit() {
    if (window)
        delete window;
    if (app)
        delete app;
}



// Instanciate the derived class.
toolkit * toolkit::createInstance(toolkit_callback *tk, int setDebugLevel) {
    return (toolkit *)new qt_toolkit(tk, setDebugLevel);
}



// Prepare graphic
bool qt_toolkit::startWindow() {
    window = new MainWindow(app);
    return true;
}



// Sart message loop, and to not return
void qt_toolkit::run() {
    window->show();
    app->exec();
}



// Activate Javascript function
bool qt_toolkit::callJavascript(const char *function) {
    return true;
}
