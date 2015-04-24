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

function UXInterface(core)
{
    var self = this;
    var elemFormatting = document.getElementById("_formatting");
    var setupDone = false;
    var callbacks = null;
    var lastReceivedFormatting = {};
    var formatting = {};
    var cursorDiv = null;
    var dragging = false;
    var buttons = {};

    function hideCursor()
    {
        if (cursorDiv != null) {
            core.cdoc.body.removeChild(cursorDiv);
            cursorDiv = null;
        }
    }

    function hideOverlaysWhileExecuting(fun)
    {
        if (cursorDiv != null)
            cursorDiv.style.visibility = "hidden";
        try {
            return fun();
        }
        finally {
            if (cursorDiv != null)
                cursorDiv.style.visibility = "visible";
        }
    }

    // DOM event handlers
    function mouseDown(event)
    {
        var x = event.clientX;
        var y = event.clientY;

        var targetClass = event.target.getAttribute("class");
        console.log("mouseDown "+event.clientX+", "+event.clientY+", class "+targetClass);
        console.log("body height = "+core.cdoc.body.getBoundingClientRect().height);

        dragging = true;
        hideOverlaysWhileExecuting(function() {
            core.op.selection.dragSelectionBegin(x,y,false);
        });

        updateFormatting();
        event.preventDefault();
    }

    function mouseUp(event)
    {
        dragging = false;
        updateFormatting();
        event.preventDefault();
    }

    function mouseMove(event)
    {
        if (dragging) {
            core.op.selection.dragSelectionUpdate(event.clientX,event.clientY,false);
            updateFormatting();
        }
        event.preventDefault();
    }


    function keyDown(event)
    {
        if (event.keyCode == 13) {
            core.op.cursor.enterPressed();
            updateFormatting();
            event.preventDefault();
        }
        else if (event.keyCode == 8) {
            core.op.cursor.deleteCharacter();
            updateFormatting();
            event.preventDefault();
        }
    }

    function keyUp(event)
    {
        if ((event.keyCode == 13) || (event.keyCode == 8))
            event.preventDefault();
    }

    function keyPress(event)
    {
        core.op.cursor.insertCharacter(String.fromCharCode(event.charCode));
        updateFormatting();
        event.preventDefault();
    }













    function markFormattingDirty()
    {
        var changes = {};
        for (var key in formatting)
            changes[key] = formatting[key];

        // When calling updateFormatting(), we have to ensure that any properties we
        // wish to clear are present in the object, but set to null. While the following
        // code might appear to do nothing, what it's actually searching for are missing
        // properties, which it explicitly adds as null.
        for (var key in lastReceivedFormatting) {
            console.log("lastReceivedFormatting["+JSON.stringify(key)+"] = "+
                        JSON.stringify(lastReceivedFormatting[key]));
            if (changes[key] == null)
                changes[key] = null;
        }

        console.log("caling applyFormattingChanges: "+JSON.stringify(changes));
        core.op.formatting.applyFormattingChanges(null,changes);
        updateFormatting();
    }

    function toggleBold()
    {
        console.log("toggleBold()");
        if (formatting["font-weight"] != null)
            delete formatting["font-weight"];
        else
            formatting["font-weight"] = "bold";
        markFormattingDirty();
    }

    function toggleItalic()
    {
        console.log("toggleItalic()");
        if (formatting["font-style"] != null)
            delete formatting["font-style"];
        else
            formatting["font-style"] = "italic";
        markFormattingDirty();
    }

    function toggleUnderline()
    {
        console.log("toggleUnderline()");
        if (formatting["text-decoration"] != null)
            delete formatting["text-decoration"];
        else
            formatting["text-decoration"] = "underline";
        markFormattingDirty();
    }

    function pressListUL()
    {
        if (formatting["-uxwrite-in-ul"] != null)
            core.op.lists.clearList();
        else
            core.op.lists.setUnorderedList();
        markFormattingDirty();
    }

    function pressListOL()
    {
        if (formatting["-uxwrite-in-ol"] != null)
            core.op.lists.clearList();
        else
            core.op.lists.setOrderedList();
        markFormattingDirty();
    }

    function pressListOutdent()
    {
        core.op.lists.decreaseIndent();
        markFormattingDirty();
    }

    function pressListIndent()
    {
        var inUL = formatting["-uxwrite-in-ul"];
        var inOL = formatting["-uxwrite-in-ol"];
        if (!inUL && !inOL)
            core.op.lists.setUnorderedList();
        else
            core.op.lists.increaseIndent();
        markFormattingDirty();
    }





    function retrieveStyles()
    {
        console.log("retrieveStyles: this = "+this.constructor.name);
        console.log("retrieveStyles: self = "+self.constructor.name);
    }

    function updateOutline()
    {
    }

    function shallowCopy(object)
    {
        var result = {};
        for (var key in object)
            result[key] = object[key];
        return result;
    }

    function updateFormatting()
    {
        console.log("updateFormatting()");
        lastReceivedFormatting = core.op.formatting.getFormatting();
        formatting = shallowCopy(lastReceivedFormatting);
        console.log("formatting = "+JSON.stringify(formatting));
        while (elemFormatting.firstChild != null)
            elemFormatting.removeChild(elemFormatting.firstChild);
        for (var key in formatting) {
            var str = key+" = "+formatting[key];
            elemFormatting.appendChild(document.createTextNode(str+"\n"));
        }

        var inUL = formatting["-uxwrite-in-ul"];
        var inOL = formatting["-uxwrite-in-ol"];

        buttons.bold.setValue(formatting["font-weight"] != null);
        buttons.italic.setValue(formatting["font-style"] != null);
        buttons.underline.setValue(formatting["text-decoration"] != null);
        buttons.listUL.setValue(inUL);
        buttons.listOL.setValue(inOL);
    }

    function setup(completion, iframeSrc, editorSrc, resources)
    {
        var toolbar = document.getElementById("_toolbar");

        buttons.bold = new Button(resources + "/images/bold");
        buttons.italic = new Button(resources + "/images/italic");
        buttons.underline = new Button(resources + "/images/underline");
        buttons.listUL = new Button(resources + "/images/list-ul");
        buttons.listOL = new Button(resources + "/images/list-ol");
        buttons.listOutdent = new Button(resources + "/images/outdent");
        buttons.listIndent = new Button(resources + "/images/indent");

        toolbar.appendChild(buttons.bold.element);
        toolbar.appendChild(buttons.italic.element);
        toolbar.appendChild(buttons.underline.element);
        toolbar.appendChild(buttons.listUL.element);
        toolbar.appendChild(buttons.listOL.element);
        toolbar.appendChild(buttons.listOutdent.element);
        toolbar.appendChild(buttons.listIndent.element);

        core.callbacks = callbacks;
        core.setup(function() {
            setupDone = true;

            retrieveStyles();
            updateOutline();
            updateFormatting();

            buttons.bold.onMouseDown = toggleBold;
            buttons.italic.onMouseDown = toggleItalic;
            buttons.underline.onMouseDown = toggleUnderline;
            buttons.listUL.onMouseDown = pressListUL;
            buttons.listOL.onMouseDown = pressListOL;
            buttons.listOutdent.onMouseDown = pressListOutdent;
            buttons.listIndent.onMouseDown = pressListIndent;

            core.cdoc.documentElement.addEventListener("mousedown",mouseDown,true);
            core.cdoc.documentElement.addEventListener("mouseup",mouseUp,true);
            core.cdoc.documentElement.addEventListener("mousemove",mouseMove,true);

            document.body.addEventListener("keydown",keyDown,true);
            document.body.addEventListener("keyup",keyUp,true);
            document.body.addEventListener("keypress",keyPress,true);

            if (completion != null)
                completion();
        }, iframeSrc, editorSrc, resources);
    }

    callbacks = {
        debug: function(str) {
            console.log("cb debug("+JSON.stringify(str)+")");
        },
        error: function(error,type) {
            console.log("cb error("+JSON.stringify(error)+","+
                                    JSON.stringify(type)+")");
        },
        addOutlineItem: function(itemId,type,title) {
            console.log("cb addOutlineItem("+JSON.stringify(itemId)+","+
                                             JSON.stringify(type)+","+
                                             JSON.stringify(title)+")");
        },
        updateOutlineItem: function(itemId,title) {
            console.log("cb updateOutlineItem("+JSON.stringify(itemId)+","+
                                                JSON.stringify(title)+")");
        },
        removeOutlineItem: function(itemId) {
            console.log("cb removeOutlineItem("+JSON.stringify(itemId)+")");
        },
        outlineUpdated: function() {
            console.log("cb outlineUpdated()");
        },
        setCursor: function(x,y,width,height) {
            console.log("cb setCursor("+x+","+y+","+width+","+height+")");


            if (cursorDiv == null) {
                cursorDiv = document.createElement("div");
                core.cdoc.body.appendChild(cursorDiv);
            }

            var iframeRect = core.iframe.getBoundingClientRect();
            cursorDiv.style.display = "block";
            cursorDiv.style.position = "absolute";
            cursorDiv.style.backgroundColor = "blue";
            cursorDiv.style.left = x+"px";
            cursorDiv.style.top = y+"px";
            cursorDiv.style.width = width+"px";
            cursorDiv.style.height = height+"px";
            cursorDiv.setAttribute("class","cursor");
        },
        setSelectionHandles: function(x1,y1,height1,x2,y2,height2) {
            console.log("cb setSelectionHandles("+x1+","+y1+","+height1+","+
                                                  x2+","+y2+","+height2+")");
            hideCursor();
        },
        setTableSelection: function(x,y,width,height) {
            console.log("cb setTableSelection("+x+","+y+","+width+","+height+")");
            hideCursor();
        },
        setSelectionBounds: function(left,top,right,bottom) {
            console.log("cb setSelectionBounds("+left+","+top+","+right+","+bottom+")");
        },
        clearSelectionHandlesAndCursor: function() {
            console.log("cb clearSelectionHandlesAndCursor()");
            hideCursor();
        },
        updateAutoCorrect: function() {
            console.log("cb updateAutoCorrect()");
        },
    };

    this.setup = setup;
}
