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

/*
 * Interface between implementation dependent toolkit and the rest of corinthia.
 *
 * This file describes the smallest possible interface, in order to facilitate easier
 * implementation of other toolkits.
 *
 * The toolkit implementation has the following responsibilities:
 * - Make all graphic manipulation and event handling
 * - Generate the main window, with buttons and a text frame (to show the actual text). Layout is in the TK.
 * - Receive and pass events from the windows/buttons/javasscripts
 * - Manipulate the buttons (enable/disable etc)
 * - Handle dialog boxes, layout is in the TK
 * - Start Javascripts on the text
 * - Provide low level call back with results
 *
 * The toolkit generic part has the following responsibilities:
 * - Provide single calls for every javascript function
 * - Provide manipulation calls (enable, focus etc) for every button
 * - Provide high level call back for each javascript function and button
 *
 * Some of the TK implementations might have a license incompatible with a Apache release, as a consequence the toolkit class
 * makes a complete seperation between the apache world, and the potential non-apache world.
 *
 * The interface consist of 2 classes (each singletons)
 * the toolkit class is instanciated in Layer1 (in the individual implementations) and called from generic Layer1 to
 * activate actions.
 *
 * The toolkit_callback class is instanciated in the generic Layer1 and called from the toolkit implementation to pass results
 * back to the generic layer
 */



class toolkit_callback {
    /* Callback interface
     *
     * Callbacks are always invoked *after* the execution of a particular editor library API function,
     * not during. The reason for this design design in the library was to enable support for web view
     * classes that did not provide native support for callbacks (as was the case for iOS, at least at
     * the time the library was originally written).
     *
     * The way that callbacks are invoked is that after each editor API call, a query is performed for a
     * list of pending callback messages. The evaluation logic iterates through these and invokes the
     * appropriate callback method for each. For this reason, callbacks method are all 'void' - they
     * never return a value. Callbacks are for notification purposes only - typically telling layer2
     * to update the UI in some manner.
     */

public:
    // class is a singleton, so the destructor will only be called when terminating the application
    ~toolkit_callback() {}

    // Request a debug message to be passed to the log system 
    // level can have values as defined in the toolkit class
    void debug(int level, const char *message);

    // pass back Javascript result
    void notifyJavascript(const char *message);

    // pass back Button action
    // button can have values as defined in toolkit class
    // (note windows actions are handled as special buttons)
    void notifyButtonPressed(int button);

    // pass back Dialogbox action
    // dialog can have values as defined in toolkit class
    virtual void notifyDialog(int dialog, const char *message) = 0;
};



class toolkit
{
    /* toolkit interface
    *
    * this class is pure virtual, to make sure it gets implemented in toolkit implementation without any dependencies
    * from the generic layer.
    *
    * Methods in this class activate graphical functions
    *
    * A static createInstance() is supplied to allow the TK implementation to instanciate the derived class
    */

public:
    // Create instance
    static const enum {
        DEBUG_NONE,
        DEBUG_INFO,
        DEBUG_WARNING,
        DEBUG_DEBUG,
        DEBUG_ERROR
    };
    static toolkit *createInstance(toolkit_callback *callback, int debugLevel);

    // Start windows etc
    virtual bool startWindow() = 0;

    // Start message loop
    virtual void run() = 0;

    // Start Javascript
    virtual bool callJavascript(const char *function) = 0;
};

