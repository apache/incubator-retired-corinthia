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

var IMPL_DIR = "../../../Editor/src";

function UXEditor(element)
{
    var self = this;
    var doc = element.ownerDocument;
    var iframeWrapper = null;
    var iframe = null;
    var setupCompletion = null;
    var pendingScripts = {};
    var doneScriptInit = false;

    console.log("UXEditor: element = "+element);
    console.log("UXEditor: doc = "+doc);

    function setup(completion)
    {
        setupCompletion = completion;
        iframeWrapper = doc.createElement("div");
//        iframeWrapper.setAttribute("id","_contentWrapper");
        iframeWrapper.setAttribute("tabindex","0");
        iframeWrapper.style.border = "none";

        iframe = doc.createElement("iframe");
        iframe.setAttribute("id","_content");
        iframe.addEventListener("load",iframeLoaded);
        iframe.setAttribute("src","sample.html");
        iframe.style.border = "none";
//        iframe.setAttribute("class","contentframe");

        iframeWrapper.appendChild(iframe);
        element.appendChild(iframeWrapper);

        self.iframeWrapper = iframeWrapper;
        self.iframe = iframe;

        window.onresize = updateIFrameSize;
        updateIFrameSize();
    }

    function updateIFrameSize()
    {
        var width = window.innerWidth;
        var height = window.innerHeight - 64;

        iframe.style.width = width+"px";
        iframe.style.height = height+"px";
        iframeWrapper.style.width = width+"px";
        iframeWrapper.style.height = height+"px";
    }

    function iframeLoaded()
    {
        console.log("iframe loaded");
        self.cdoc = iframe.contentDocument;
        self.cwin = iframe.contentWindow;

        var javascriptFiles = ["first.js",
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
                               "Range.js",
                               "Selection.js",
                               "3rdparty/showdown/showdown.js",
                               "Scan.js",
                               "StringBuilder.js",
                               "Styles.js",
                               "Tables.js",
                               "Text.js",
                               "traversal.js",
                               "types.js",
                               "UndoManager.js",
                               "util.js",
                               "Viewport.js"];

        // We first create all the script objects and set their onload properties. Only *after*
        // that do we actually add them to the document. This ensures that pendingScripts will
        // only ever become empty after all the scripts have definitely loaded.
        var scripts = [];
        for (var i = 0; i < javascriptFiles.length; i++) {
            var src = IMPL_DIR+"/"+javascriptFiles[i];
            var script = document.createElement("script");
            script.setAttribute("src",src);
            pendingScripts[src] = true;
            script.onload = function(e) { scriptLoaded(e.target); }
            scripts.push(script);
        }

        // Now add the script elements to the document, triggering their load
        for (var i = 0; i < scripts.length; i++)
            self.cdoc.head.appendChild(scripts[i]);
    }

    function scriptLoaded(script)
    {
        delete pendingScripts[script.getAttribute("src")];
        console.log("loadedScript: "+script.getAttribute("src")+
                    "; remaining = "+Object.keys(pendingScripts).length);
        if (Object.keys(pendingScripts).length > 0)
            return;

        if (!doneScriptInit) {
            doneScriptInit = true;
            console.log("Now we should init");
/*
            self.cwin.Editor_debug = Editor_debug;
            self.cwin.Editor_error = Editor_error;
            self.cwin.Editor_addOutlineItem = Editor_addOutlineItem;
            self.cwin.Editor_updateOutlineItem = Editor_updateOutlineItem;
            self.cwin.Editor_removeOutlineItem = Editor_removeOutlineItem;
            self.cwin.Editor_outlineUpdated = Editor_outlineUpdated;
            self.cwin.Editor_setCursor = Editor_setCursor;
            self.cwin.Editor_setSelectionHandles = Editor_setSelectionHandles;
            self.cwin.Editor_setTableSelection = Editor_setTableSelection;
            self.cwin.Editor_setSelectionBounds = Editor_setSelectionBounds;
            self.cwin.Editor_clearSelectionHandlesAndCursor = Editor_clearSelectionHandlesAndCursor;
            self.cwin.Editor_updateAutoCorrect = Editor_updateAutoCorrect;
            self.cwin.debug = Editor_debug;
*/
//            self.cwin.debug("loadedScript: before calling Main_init");
//            self.cwin.eval("Main_init(800,150,'../uxwrite/resources/builtin.css',false)");
            self.cwin.Main_init(800,150,"builtin.css",false);
//            self.cwin.debug("loadedScript: after calling Main_init");

//            self.cdoc.documentElement.addEventListener("mousedown",mouseDown,true);
//            self.cdoc.documentElement.addEventListener("mouseup",mouseUp,true);
//            self.cdoc.documentElement.addEventListener("mousemove",mouseMove,true);

//            ifwrapper.addEventListener("keydown",keyDown,true);
//            ifwrapper.addEventListener("keyup",keyUp,true);
//            ifwrapper.addEventListener("keypress",keyPress,true);
//            ifwrapper.focus();
//            document.body.focus();

            if (setupCompletion != null)
                setupCompletion();
        }
    }

    function invokeCallbacks()
    {
        var messages = JSON.parse(self.cwin.Editor_getBackMessages());

        if (self.callbacks == null)
            return;
        console.log("messages.length = "+messages.length);
        for (var i = 0; i < messages.length; i++) {
            var name = messages[i][0];
            var args = messages[i].slice(1);
            if ((self.callbacks != null) && (self.callbacks[name] != null))
                self.callbacks[name].apply(null,args);
        }
    }

    function wrap(name)
    {
        return function() {
            var argsArray = [];
            var args = new Array();
            console.log("arguments.length = "+arguments.length);
            for (var i = 0; i < arguments.length; i++) {
                args.push(arguments[i]);
            }
            var res = self.cwin.Main_execute(function() {
                return self.cwin[name].apply(null,args);
            });
            invokeCallbacks();

            // Ensure we have a completely separate copy of the data
            if (res == null) {
                return null;
            }
            else {
                var str = JSON.stringify(res);
                return JSON.parse(str);
            }
        }
    }

    var op = {
        clipboard: {
            cut: wrap("Clipboard_cut"),
            copy: wrap("Clipboard_copy"),
            pasteText: wrap("Clipboard_pasteText"),
            pasteHTML: wrap("Clipboard_pasteHTML"),
        },
        cursor: {
            positionCursor: wrap("Cursor_positionCursor"),
            getCursorPosition: wrap("Cursor_getCursorPosition"),
            insertReference: wrap("Cursor_insertReference"),
            insertLink: wrap("Cursor_insertLink"),
            insertCharacter: wrap("Cursor_insertCharacter"),
            deleteCharacter: wrap("Cursor_deleteCharacter"),
            enterPressed: wrap("Cursor_enterPressed"),
            getPrecedingWord: wrap("Cursor_getPrecedingWord"),
            getLinkProperties: wrap("Cursor_getLinkProperties"),
            setLinkProperties: wrap("Cursor_setLinkProperties"),
            setReferenceTarget: wrap("Cursor_setReferenceTarget"),
            insertFootnote: wrap("Cursor_insertFootnote"),
            insertEndnote: wrap("Cursor_insertEndnote"),
            test: wrap("Cursor_test"),
        },
        figures: {
            insertFigure: wrap("Figures_insertFigure"),
            getSelectedFigureId: wrap("Figures_getSelectedFigureId"),
            getProperties: wrap("Figures_getProperties"),
            setProperties: wrap("Figures_setProperties"),
            getGeometry: wrap("Figures_getGeometry"),
        },
        formatting: {
            getFormatting: wrap("Formatting_getFormatting"),
            applyFormattingChanges: wrap("Formatting_applyFormattingChanges"),
        },
        input: {
        },
        lists: {
            increaseIndent: wrap("Lists_increaseIndent"),
            decreaseIndent: wrap("Lists_decreaseIndent"),
            clearList: wrap("Lists_clearList"),
            setUnorderedList: wrap("Lists_setUnorderedList"),
            setOrderedList: wrap("Lists_setOrderedList"),
        },
        main: {
            getLanguage: wrap("Main_getLanguage"),
            setLanguage: wrap("Main_setLanguage"),
            setGenerator: wrap("Main_setGenerator"),
            prepareForSave: wrap("Main_prepareForSave"),
            getHTML: wrap("Main_getHTML"),
            isEmptyDocument: wrap("Main_isEmptyDocument"),
        },
        outline: {
            getOutline: wrap("Outline_getOutline"),
            moveSection: wrap("Outline_moveSection"),
            deleteItem: wrap("Outline_deleteItem"),
            goToItem: wrap("Outline_goToItem"),
            scheduleUpdateStructure: wrap("Outline_scheduleUpdateStructure"),
            setTitle: wrap("Outline_setTitle"),
            setNumbered: wrap("Outline_setNumbered"),
            insertTableOfContents: wrap("Outline_insertTableOfContents"),
            insertListOfFigures: wrap("Outline_insertListOfFigures"),
            insertListOfTables: wrap("Outline_insertListOfTables"),
            setPrintMode: wrap("Outline_setPrintMode"),
            examinePrintLayout: wrap("Outline_examinePrintLayout"),
            detectSectionNumbering: wrap("Outline_detectSectionNumbering"),
            findUsedStyles: wrap("Outline_findUsedStyles"),
        },
        scan: {
            reset: wrap("Scan_reset"),
            next: wrap("Scan_next"),
            addMatch: wrap("Scan_addMatch"),
            showMatch: wrap("Scan_showMatch"),
            replaceMatch: wrap("Scan_replaceMatch"),
            removeMatch: wrap("Scan_removeMatch"),
            goToMatch: wrap("Scan_goToMatch"),
        },
        selection: {
            update: wrap("Selection_update"),
            selectAll: wrap("Selection_selectAll"),
            selectParagraph: wrap("Selection_selectParagraph"),
            selectWordAtCursor: wrap("Selection_selectWordAtCursor"),
            dragSelectionBegin: wrap("Selection_dragSelectionBegin"),
            dragSelectionUpdate: wrap("Selection_dragSelectionUpdate"),
            moveStartLeft: wrap("Selection_moveStartLeft"),
            moveStartRight: wrap("Selection_moveStartRight"),
            moveEndLeft: wrap("Selection_moveEndLeft"),
            moveEndRight: wrap("Selection_moveEndRight"),
            setSelectionStartAtCoords: wrap("Selection_setSelectionStartAtCoords"),
            setSelectionEndAtCoords: wrap("Selection_setSelectionEndAtCoords"),
            setTableSelectionEdgeAtCoords: wrap("Selection_setTableSelectionEdgeAtCoords"),
            print: wrap("Selection_print"),
        },
        styles: {
            getCSSText: wrap("Styles_getCSSText"),
            setCSSText: wrap("Styles_setCSSText"),
            getParagraphClass: wrap("Styles_getParagraphClass"),
            setParagraphClass: wrap("Styles_setParagraphClass"),
        },
        tables: {
            insertTable: wrap("Tables_insertTable"),
            addAdjacentRow: wrap("Tables_addAdjacentRow"),
            addAdjacentColumn: wrap("Tables_addAdjacentColumn"),
            removeAdjacentRow: wrap("Tables_removeAdjacentRow"),
            removeAdjacentColumn: wrap("Tables_removeAdjacentColumn"),
            clearCells: wrap("Tables_clearCells"),
            mergeCells: wrap("Tables_mergeCells"),
            splitSelection: wrap("Tables_splitSelection"),
            getSelectedTableId: wrap("Tables_getSelectedTableId"),
            getProperties: wrap("Tables_getProperties"),
            setProperties: wrap("Tables_setProperties"),
            setColWidths: wrap("Tables_setColWidths"),
            getGeometry: wrap("Tables_getGeometry"),
        },
        undoManager: {
            getLength: wrap("UndoManager_getLength"),
            getIndex: wrap("UndoManager_getIndex"),
            setIndex: wrap("UndoManager_setIndex"),
            undo: wrap("UndoManager_undo"),
            redo: wrap("UndoManager_redo"),
            newGroup: wrap("UndoManager_newGroup"),
            groupType: wrap("UndoManager_groupType"),
        },
    };

    self.setup = setup;
    self.op = op;
    self.callbacks = null;
    self.cdoc = null;
    self.cwin = null;
    self.iframeWrapper = null;
    self.iframe = null;
}
