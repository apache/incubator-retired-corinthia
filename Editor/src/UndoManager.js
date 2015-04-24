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

// FIXME: place a limit on the number of undo steps recorded - say, 30-50?

var UndoManager_getLength;
var UndoManager_getIndex;
var UndoManager_setIndex;
var UndoManager_print;
var UndoManager_undo;
var UndoManager_redo;
var UndoManager_addAction;
var UndoManager_newGroup;
var UndoManager_groupType;
var UndoManager_disableWhileExecuting;
var UndoManager_isActive;
var UndoManager_isDisabled;
var UndoManager_clear;
var UndoManager_setProperty;
var UndoManager_deleteProperty;

(function() {

    var UNDO_LIMIT = 50;

    function UndoGroup(type,onClose)
    {
        this.type = type;
        this.onClose = onClose;
        this.actions = new Array();
    }

    function UndoAction(fun,args)
    {
        this.fun = fun;
        this.args = args;
    }

    UndoAction.prototype.toString = function()
    {
        var name;
        if (this.fun.wrappedName != null)
            name = this.fun.wrappedName;
        else
            name = this.fun.name;

        var argStrings = new Array();
        for (var i = 0; i < this.args.length; i++) {
            if (this.args[i] instanceof Node)
                argStrings.push(nodeString(this.args[i]));
            else if (this.args[i] == null)
                argStrings.push("null");
            else
                argStrings.push(this.args[i].toString());
        }

        return name + "(" + argStrings.join(",") + ")";
    }

    var undoStack = new Array();
    var redoStack = new Array();
    var inUndo = false;
    var inRedo = false;
    var currentGroup = null;
    var disabled = 0;

    // public
    UndoManager_getLength = function()
    {
        return undoStack.length + redoStack.length;
    }

    // public
    UndoManager_getIndex = function()
    {
        return undoStack.length;
    }

    // public
    UndoManager_setIndex = function(index)
    {
        while (undoStack.length > index)
            UndoManager_undo();
        while (undoStack.length < index)
            UndoManager_redo();
    }

    // public
    UndoManager_print = function()
    {
        debug("");
        debug("--------------------------------------------------------------------");
        debug("Undo stack:");
        for (var groupIndex = 0; groupIndex < undoStack.length; groupIndex++) {
            var group = undoStack[groupIndex];
            debug("    "+group.type);
            for (var actionIndex = 0; actionIndex < group.actions.length; actionIndex++) {
                var action = group.actions[actionIndex];
                debug("        "+action);
            }
        }
        debug("Redo stack:");
        for (var groupIndex = 0; groupIndex < redoStack.length; groupIndex++) {
            var group = redoStack[groupIndex];
            debug("    "+group.type);
            for (var actionIndex = 0; actionIndex < group.actions.length; actionIndex++) {
                var action = group.actions[actionIndex];
                debug("        "+action);
            }
        }
        debug("Current group = "+currentGroup);
        debug("--------------------------------------------------------------------");
        debug("");
    }

    function closeCurrentGroup()
    {
        if ((currentGroup != null) && (currentGroup.onClose != null))
            currentGroup.onClose();
        currentGroup = null;
    }

    // public
    UndoManager_undo = function()
    {
        closeCurrentGroup();
        if (undoStack.length > 0) {
            var group = undoStack.pop();
            inUndo = true;
            for (var i = group.actions.length-1; i >= 0; i--)
                group.actions[i].fun.apply(null,group.actions[i].args);
            inUndo = false;
        }
        closeCurrentGroup();
    }

    // public
    UndoManager_redo = function()
    {
        closeCurrentGroup();
        if (redoStack.length > 0) {
            var group = redoStack.pop();
            inRedo = true;
            for (var i = group.actions.length-1; i >= 0; i--)
                group.actions[i].fun.apply(null,group.actions[i].args);
            inRedo = false;
        }
        closeCurrentGroup();
    }

    // public
    UndoManager_addAction = function(fun)
    {
        if (disabled > 0)
            return;

        // remaining parameters after fun are arguments to be supplied to fun
        var args = new Array();
        for (var i = 1; i < arguments.length; i++)
            args.push(arguments[i]);

        if (!inUndo && !inRedo && (redoStack.length > 0))
            redoStack.length = 0;

        var stack = inUndo ? redoStack : undoStack;
        if (currentGroup == null)
            UndoManager_newGroup(null);

        // Only add a group to the undo stack one it has at least one action, to avoid having
        // empty groups present.
        if (currentGroup.actions.length == 0) {
            if (!inUndo && !inRedo && (stack.length == UNDO_LIMIT))
                stack.shift();
            stack.push(currentGroup);
        }

        currentGroup.actions.push(new UndoAction(fun,args));
    }

    // public
    UndoManager_newGroup = function(type,onClose)
    {
        if (disabled > 0)
            return;

        closeCurrentGroup();

        // We don't actually add the group to the undo stack until the first request to add an
        // action to it. This way we don't end up with empty groups in the undo stack, which
        // simplifies logic for moving back and forward through the undo history.

        if ((type == null) || (type == ""))
            type = "Anonymous";
        currentGroup = new UndoGroup(type,onClose);
    }

    // public
    UndoManager_groupType = function()
    {
        if (undoStack.length > 0)
            return undoStack[undoStack.length-1].type;
        else
            return null;
    }

    UndoManager_disableWhileExecuting = function(fun) {
        disabled++;
        try {
            return fun();
        }
        finally {
            disabled--;
        }
    }

    UndoManager_isActive = function()
    {
        return (inUndo || inRedo);
    }

    UndoManager_isDisabled = function() {
        return (disabled > 0);
    }

    UndoManager_clear = function() {
        undoStack.length = 0;
        redoStack.length = 0;
    }

    function saveProperty(obj,name)
    {
        if (obj.hasOwnProperty(name))
            UndoManager_addAction(UndoManager_setProperty,obj,name,obj[name]);
        else
            UndoManager_addAction(UndoManager_deleteProperty,obj,name);
    }

    UndoManager_setProperty = function(obj,name,value)
    {
        if (obj.hasOwnProperty(name) && (obj[name] == value))
            return; // no point in adding an undo action
        saveProperty(obj,name);
        obj[name] = value;
    }

    UndoManager_deleteProperty = function(obj,name)
    {
        if (!obj.hasOwnProperty(name))
            return; // no point in adding an undo action
        saveProperty(obj,name);
        delete obj[name];
    }

})();

window.undoSupported = true;
