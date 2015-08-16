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

function testUndo(versions,node)
{
    var numSteps = UndoManager_getLength();

    var back1 = new Array();
    var forwards2 = new Array();
    var back2 = new Array();

    var expected = new Array();
    for (var i = 0; i < versions.length; i++)
        expected.push(PrettyPrinter.getHTML(versions[i]));

    for (var i = 0; i < numSteps; i++) {
        UndoManager_undo();
        PostponedActions_perform();
        var version = versions.length-2-i;
        if (PrettyPrinter.getHTML(node) == expected[version])
            back1.push(DOM_createTextNode(document,"First undo to version "+version+": OK"));
        else
            back1.push(DOM_createTextNode(document,"First undo to version "+version+": INVALID"));
    }

    for (var i = 0; i < numSteps; i++) {
        UndoManager_redo();
        PostponedActions_perform();
        var version = i+1;
        if (PrettyPrinter.getHTML(node) == expected[version])
            forwards2.push(DOM_createTextNode(document,"Redo to version "+version+": OK"));
        else
            forwards2.push(DOM_createTextNode(document,"Redo to version "+version+": INVALID"));
    }

    for (var i = 0; i < numSteps; i++) {
        UndoManager_undo();
        PostponedActions_perform();
        var version = versions.length-2-i;
        if (PrettyPrinter.getHTML(node) == expected[version])
            back2.push(DOM_createTextNode(document,"Second undo to version "+version+": OK"));
        else
            back2.push(DOM_createTextNode(document,"Second undo to version "+version+": INVALID"));
    }

    var initialLength = versions.length;

    Array.prototype.push.apply(versions,back1);
    Array.prototype.push.apply(versions,forwards2);
    Array.prototype.push.apply(versions,back2);

    Outline_removeListeners(); // prevent it from adding number spans etc.
    AutoCorrect_removeListeners();
    DOM_deleteAllChildren(document.body);
    for (var i = 0; i < versions.length; i++) {
        if (i < initialLength) {
            var str = "==================== Version "+i+" ====================";
            DOM_appendChild(document.body,DOM_createTextNode(document,str));
        }
        else if (i == initialLength) {
            var str = "===================================================";
            DOM_appendChild(document.body,DOM_createTextNode(document,str));
        }
        DOM_appendChild(document.body,versions[i]);
    }
}

function placeCursorAfterElement(id)
{
    UndoManager_disableWhileExecuting(function() {
        var element = document.getElementById(id);
        var node = element.parentNode;
        var offset = DOM_nodeOffset(element)+1;
        Selection_set(node,offset,node,offset);
    });
}
