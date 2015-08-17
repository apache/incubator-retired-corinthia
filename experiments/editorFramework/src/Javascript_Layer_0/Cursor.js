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

var Cursor_ensurePositionVisible;
var Cursor_ensureCursorVisible;
var Cursor_scrollDocumentForY;
var Cursor_positionCursor;
var Cursor_getCursorPosition;
var Cursor_moveLeft;
var Cursor_moveRight;
var Cursor_moveToStartOfDocument;
var Cursor_moveToEndOfDocument;
var Cursor_updateBRAtEndOfParagraph;
var Cursor_insertReference;
var Cursor_insertLink;
var Cursor_insertCharacter;
var Cursor_deleteCharacter;
var Cursor_enterPressed;
var Cursor_getPrecedingWord;
var Cursor_getAdjacentNodeWithType;
var Cursor_getLinkProperties;
var Cursor_setLinkProperties;
var Cursor_setReferenceTarget;
var Cursor_makeContainerInsertionPoint;
var Cursor_set;
var Cursor_insertFootnote;
var Cursor_insertEndnote;

(function() {

    var cursorX = null;

    Cursor_ensurePositionVisible = function(pos,center)
    {
        // If we can't find the cursor rect for some reason, just don't do anything.
        // This is better than using an incorrect position or throwing an exception.
        var rect = Position_displayRectAtPos(pos)
        if (rect != null) {
            var extraSpace = 4;

            var cursorTop = rect.top + window.scrollY - extraSpace;
            var cursorBottom = rect.top + rect.height + window.scrollY + extraSpace;

            var windowTop = window.scrollY;
            var windowBottom = window.scrollY + window.innerHeight;

            if (center) {
                var newY = Math.floor(cursorTop + rect.height/2 - window.innerHeight/2);
                window.scrollTo(window.scrollX,newY);
            }
            else if (cursorTop < windowTop) {
                window.scrollTo(window.scrollX,cursorTop);
            }
            else if (cursorBottom > windowBottom) {
                window.scrollTo(window.scrollX,cursorBottom - window.innerHeight);
            }
        }
    }

    // public
    Cursor_ensureCursorVisible = function(center)
    {
        var selRange = Selection_get();
        if (selRange != null)
            Cursor_ensurePositionVisible(selRange.end,center);
    }

    Cursor_scrollDocumentForY = function(y)
    {
        var absY = window.scrollY + y;
        if (absY-44 < window.scrollY) {
            window.scrollTo(window.scrollX,absY-44);
            y = absY - window.scrollY;
        }
        else if (absY+44 >= window.scrollY + window.innerHeight) {
            window.scrollTo(window.scrollX,absY+44 - window.innerHeight);
            y = absY - window.scrollY;
        }
        return y;
    }

    // public
    Cursor_positionCursor = function(x,y,wordBoundary)
    {
        if (UndoManager_groupType() != "Cursor movement")
            UndoManager_newGroup("Cursor movement");

        y = Cursor_scrollDocumentForY(y);

        var result = null;
        var position = Position_atPoint(x,y);
        if (position == null)
            return null;

        var node = Position_closestActualNode(position);
        for (; node != null; node = node.parentNode) {
            var type = node._type;
            if ((type == HTML_A) &&
                (node.hasAttribute("href")) &&
                (result == null)) {

                var arange = new Range(node,0,node,node.childNodes.length);
                var rects = Range_getClientRects(arange);
                var insideLink = false;
                for (var i = 0; i < rects.length; i++) {
                    if (rectContainsPoint(rects[i],x,y))
                        insideLink = true;
                }

                if (insideLink) {
                    var href = node.getAttribute("href");
                    if ((href != null) && (href.charAt(0) == "#")) {
                        if (isInTOC(node))
                            result = "intocreference-"+href.substring(1);
                        else
                            result = "inreference";
                    }
                    else {
                        result = "inlink";
                    }
                }
            }
            else if ((type == HTML_IMG) && (result == null)) {
                for (var anc = node; anc != null; anc = anc.parentNode) {
                    if (anc._type == HTML_FIGURE) {
                        result = "infigure";
                        break;
                    }
                }
            }
            else if (isAutoCorrectNode(node) && (result == null)) {
                result = "incorrection";
            }
            else if (isTOCNode(node)) {
                var rect = node.getBoundingClientRect();
                if (x >= rect.left + rect.width/2)
                    position = new Position(node.parentNode,DOM_nodeOffset(node)+1);
                else
                    position = new Position(node.parentNode,DOM_nodeOffset(node));
                break;
            }
        }

        var position = Position_closestMatchForwards(position,Position_okForMovement);
        if ((position != null) && isOpaqueNode(position.node))
            position = Position_nextMatch(position,Position_okForMovement);
        if (position == null)
            return false;

        var selectionRange = Selection_get();
        var samePosition = ((selectionRange != null) && Range_isEmpty(selectionRange) &&
                            (position.node == selectionRange.start.node) &&
                            (position.offset == selectionRange.start.offset));
        if (samePosition && (result == null))
            result = "same";

        if (wordBoundary) {
            var startOfWord = Selection_posAtStartOfWord(position);
            var endOfWord = Selection_posAtEndOfWord(position);
            if ((startOfWord.node != position.node) || (startOfWord.node != position.node))
                throw new Error("Word boundary in different node");
            var distanceBefore = position.offset - startOfWord.offset;
            var distanceAfter = endOfWord.offset - position.offset;
            if (distanceBefore <= distanceAfter)
                position = startOfWord;
            else
                position = endOfWord;
        }

        Cursor_set(position.node,position.offset);
        return result;
    }

    // public
    Cursor_getCursorPosition = function()
    {
        var selRange = Selection_get();
        if (selRange == null)
            return null;

        // FIXME: in the cases where this is called from Objective C, test what happens if we
        // return a null rect
        var rect = Position_displayRectAtPos(selRange.end);
        if (rect == null)
            return null;

        var left = rect.left + window.scrollX;
        var top = rect.top + window.scrollY;
        var height = rect.height;
        return { x: left, y: top, width: 0, height: height };
    }

    // public
    Cursor_moveLeft = function()
    {
        var range = Selection_get();
        if (range == null)
            return;

        var pos = Position_prevMatch(range.start,Position_okForMovement);
        if (pos != null)
            Cursor_set(pos.node,pos.offset);
        Cursor_ensureCursorVisible();
    }

    // public
    Cursor_moveRight = function()
    {
        var range = Selection_get();
        if (range == null)
            return;

        var pos = Position_nextMatch(range.start,Position_okForMovement);
        if (pos != null)
            Cursor_set(pos.node,pos.offset);
        Cursor_ensureCursorVisible();
    }

    // public
    Cursor_moveToStartOfDocument = function()
    {
        var pos = new Position(document.body,0);
        pos = Position_closestMatchBackwards(pos,Position_okForMovement);
        Cursor_set(pos.node,pos.offset);
        Cursor_ensureCursorVisible();
    }

    // public
    Cursor_moveToEndOfDocument = function()
    {
        var pos = new Position(document.body,document.body.childNodes.length);
        pos = Position_closestMatchForwards(pos,Position_okForMovement);
        Cursor_set(pos.node,pos.offset);
        Cursor_ensureCursorVisible();
    }

    // An empty paragraph does not get shown and cannot be edited. We can fix this by adding
    // a BR element as a child
    // public
    Cursor_updateBRAtEndOfParagraph = function(node)
    {
        var paragraph = node;
        while ((paragraph != null) && !isParagraphNode(paragraph))
            paragraph = paragraph.parentNode;
        if (paragraph != null) {

            var br = null;
            var last = paragraph;
            do {

                var child = last;
                while ((child != null) && isWhitespaceTextNode(child))
                    child = child.previousSibling;

                if ((child != null) && (child._type == HTML_BR))
                    br = child;

                last = last.lastChild;

            } while ((last != null) && isInlineNode(last));

            if (nodeHasContent(paragraph)) {
                // Paragraph has content: don't want BR at end
                if (br != null) {
                    DOM_deleteNode(br);
                }
            }
            else {
                // Paragraph consists only of whitespace: must have BR at end
                if (br == null) {
                    br = DOM_createElement(document,"BR");
                    DOM_appendChild(paragraph,br);
                }
            }
        }
    }

    // public
    Cursor_insertReference = function(itemId)
    {
        var a = DOM_createElement(document,"A");
        DOM_setAttribute(a,"href","#"+itemId);
        Clipboard_pasteNodes([a]);
    }

    // public
    Cursor_insertLink = function(text,url)
    {
        var a = DOM_createElement(document,"A");
        DOM_setAttribute(a,"href",url);
        DOM_appendChild(a,DOM_createTextNode(document,text));
        Clipboard_pasteNodes([a]);
    }

    var nbsp = String.fromCharCode(160);

    function spaceToNbsp(pos)
    {
        var node = pos.node;
        var offset = pos.offset;

        if ((node.nodeType == Node.TEXT_NODE) && (offset > 0) &&
            (isWhitespaceString(node.nodeValue.charAt(offset-1)))) {
            // Insert first, to preserve any tracked positions
            DOM_insertCharacters(node,offset-1,nbsp);
            DOM_deleteCharacters(node,offset,offset+1);
        }
    }

    function nbspToSpace(pos)
    {
        var node = pos.node;
        var offset = pos.offset;

        if ((node.nodeType == Node.TEXT_NODE) && (offset > 0) &&
            (node.nodeValue.charAt(offset-1) == nbsp)) {
            // Insert first, to preserve any tracked positions
            DOM_insertCharacters(node,offset-1," ");
            DOM_deleteCharacters(node,offset,offset+1);
        }
    }

    function checkNbsp()
    {
        Selection_preserveWhileExecuting(function() {
            var selRange = Selection_get();
            if (selRange != null)
                nbspToSpace(selRange.end);
        });
    }

    function isPosAtStartOfParagraph(pos)
    {
        if ((pos.node.nodeType == Node.ELEMENT_NODE) && (pos.offset == 0) &&
            !isInlineNode(pos.node)) {
            return true;
        }



        while (pos != null) {
            if (pos.node.nodeType == Node.ELEMENT_NODE) {
                if ((pos.offset == 0) && !isInlineNode(pos.node))
                    return true;
                else
                    pos = Position_prev(pos);
            }
            else if (pos.node.nodeType == Node.TEXT_NODE) {
                if (pos.offset > 0)
                    return false;
                else
                    pos = Position_prev(pos);
            }
            else {
                return false;
            }
        }

        return false;
    }

    // public
    Cursor_insertCharacter = function(str,allowInvalidPos,allowNoParagraph)
    {
        var firstInsertion = (UndoManager_groupType() != "Insert text");

        if (firstInsertion)
            UndoManager_newGroup("Insert text",checkNbsp);

        if (str == "-") {
            var preceding = Cursor_getPrecedingWord();
            if (preceding.match(/[0-9]\s*$/))
                str = String.fromCharCode(0x2013); // en dash
            else if (preceding.match(/\s+$/))
                str = String.fromCharCode(0x2014); // em dash
        }

        var selRange = Selection_get();
        if (selRange == null)
            return;

        if (!Range_isEmpty(selRange)) {
            Selection_deleteContents(true);
            selRange = Selection_get();
        }
        var pos = selRange.start;
        pos = Position_preferTextPosition(pos);
        if ((str == " ") && isPosAtStartOfParagraph(pos))
            return;
        if (!allowInvalidPos && !Position_okForInsertion(pos)) {
            var elemPos = Position_preferElementPosition(pos);
            if (Position_okForInsertion(elemPos)) {
                pos = elemPos;
            }
            else {
                var oldPos = pos;
                pos = Position_closestMatchForwards(selRange.start,Position_okForInsertion);
                var difference = new Range(oldPos.node,oldPos.offset,pos.node,pos.offset);
                difference = Range_forwards(difference);
                Position_trackWhileExecuting([pos],function() {
                    if (!Range_hasContent(difference)) {
                        Selection_deleteRangeContents(difference,true);
                    }
                });
            }
        }
        var node = pos.node;
        var offset = pos.offset;

        if ((str == " ") &&
            !firstInsertion &&
            (node.nodeType == Node.TEXT_NODE) &&
            (offset > 0) &&
            (node.nodeValue.charAt(offset-1) == nbsp)) {

            if (!node.nodeValue.substring(0,offset).match(/\.\s+$/)) {
                DOM_deleteCharacters(node,offset-1,offset);
                DOM_insertCharacters(node,offset-1,".");
            }
        }

        if (isWhitespaceString(str) && (node.nodeType == Node.TEXT_NODE) && (offset > 0)) {
            var prevChar = node.nodeValue.charAt(offset-1);
            if (isWhitespaceString(prevChar) || (prevChar == nbsp)) {
                Selection_update();
                Cursor_ensureCursorVisible();
                return;
            }
        }

        nbspToSpace(pos);

        // If the user enters two double quotes in succession (open and close), replace them with
        // just one plain double quote character
        if ((str == "”") && (node.nodeType == Node.TEXT_NODE) &&
            (offset > 0) && (node.nodeValue.charAt(offset-1) == "“")) {
            DOM_deleteCharacters(node,offset-1,offset);
            offset--;
            str = "\"";
        }

        if (node.nodeType == Node.ELEMENT_NODE) {
            var emptyTextNode = DOM_createTextNode(document,"");
            if (offset >= node.childNodes.length)
                DOM_appendChild(node,emptyTextNode);
            else
                DOM_insertBefore(node,emptyTextNode,node.childNodes[offset]);
            node = emptyTextNode;
            offset = 0;
        }

        if (str == " ")
            DOM_insertCharacters(node,offset,nbsp);
        else
            DOM_insertCharacters(node,offset,str);

                // must be done *after* inserting the text
        if (!allowNoParagraph) {
            switch (node.parentNode._type) {
            case HTML_CAPTION:
            case HTML_FIGCAPTION:
                // Do nothing
                break;
            default:
                Hierarchy_ensureInlineNodesInParagraph(node,true);
                break;
            }
        }

        offset += str.length;

        pos = new Position(node,offset);
        Position_trackWhileExecuting([pos],function() {
            Formatting_mergeWithNeighbours(pos.node,Formatting_MERGEABLE_INLINE);
        });

        Cursor_set(pos.node,pos.offset);
        Range_trackWhileExecuting(Selection_get(),function() {
            Cursor_updateBRAtEndOfParagraph(pos.node);
        });

        Selection_update();
        Cursor_ensureCursorVisible();
    }

    function tryDeleteEmptyCaption(pos)
    {
        var caption = Position_captionAncestor(pos);
        if ((caption == null) || nodeHasContent(caption))
            return false;

        var container = Position_figureOrTableAncestor(pos);
        if (container == null)
            return false;

        Cursor_set(container.parentNode,DOM_nodeOffset(container)+1);
        Selection_preserveWhileExecuting(function() {
            DOM_deleteNode(caption);
        });

        return true;
    }

    function tryDeleteEmptyNote(pos)
    {
        var note = Position_noteAncestor(pos);
        if ((note == null) || nodeHasContent(note))
            return false;

        var parent = note.parentNode;
        Cursor_set(note.parentNode,DOM_nodeOffset(note)+1);
        Selection_preserveWhileExecuting(function() {
            DOM_deleteNode(note);
        });

        return true;
    }

    // public
    Cursor_deleteCharacter = function()
    {
        if (UndoManager_groupType() != "Delete text")
            UndoManager_newGroup("Delete text",checkNbsp);

        Selection_preferElementPositions();
        var selRange = Selection_get();
        if (selRange == null)
            return;

        if (!Range_isEmpty(selRange)) {
            Selection_deleteContents(true);
        }
        else {
            var currentPos = selRange.start;

            // Special cases of pressing backspace after a table, figure, TOC, hyperlink,
            // footnote, or endnote. For each of these we delete the whole thing.
            var back = Position_closestMatchBackwards(currentPos,Position_okForMovement);
            if ((back != null) && (back.node.nodeType == Node.ELEMENT_NODE) && (back.offset > 0)) {
                var prevNode = back.node.childNodes[back.offset-1];
                if (isSpecialBlockNode(prevNode)) {
                    var p = DOM_createElement(document,"P");
                    DOM_insertBefore(prevNode.parentNode,p,prevNode);
                    DOM_deleteNode(prevNode);
                    Cursor_updateBRAtEndOfParagraph(p);
                    Cursor_set(p,0);
                    Cursor_ensureCursorVisible();
                    return;
                }
                if ((prevNode._type == HTML_A) || isNoteNode(prevNode)) {
                    Cursor_set(back.node,back.offset-1);
                    Selection_preserveWhileExecuting(function() {
                        DOM_deleteNode(prevNode);
                    });
                    return;
                }
            }

            // Backspace inside an empty figure or table caption
            if (tryDeleteEmptyCaption(currentPos))
                return;

            currentPos = Position_preferTextPosition(currentPos);
            var prevPos = Position_prevMatch(currentPos,Position_okForMovement);

            // Backspace inside or just after a footnote or endnote
            if (tryDeleteEmptyNote(currentPos))
                return;
            if ((prevPos != null) && tryDeleteEmptyNote(prevPos))
                return;

            if (prevPos != null) {
                var startBlock = firstBlockAncestor(Position_closestActualNode(prevPos));
                var endBlock = firstBlockAncestor(Position_closestActualNode(selRange.end));
                if ((startBlock != endBlock) &&
                    isParagraphNode(startBlock) && !nodeHasContent(startBlock)) {
                    DOM_deleteNode(startBlock);
                    Cursor_set(selRange.end.node,selRange.end.offset)
                }
                else {
                    var range = new Range(prevPos.node,prevPos.offset,
                                          selRange.end.node,selRange.end.offset);
                    Selection_deleteRangeContents(range,true);
                }
            }
        }

        selRange = Selection_get();
        if (selRange != null)
            spaceToNbsp(selRange.end);
        Selection_update();
        Cursor_ensureCursorVisible();

        function firstBlockAncestor(node)
        {
            while (isInlineNode(node))
                node = node.parentNode;
            return node;
        }
    }

    // public
    Cursor_enterPressed = function()
    {
        UndoManager_newGroup("New paragraph");

        Selection_preferElementPositions();
        var selRange = Selection_get();
        if (selRange == null)
            return;

        Range_trackWhileExecuting(selRange,function() {
            if (!Range_isEmpty(selRange))
                Selection_deleteContents(true);
        });

        // Are we inside a figure or table caption? If so, put an empty paragraph directly after it
        var inCaption = false;
        var inFigCaption = false;
        var closestNode = Position_closestActualNode(selRange.start);
        for (var ancestor = closestNode; ancestor != null; ancestor = ancestor.parentNode) {
            switch (ancestor._type) {
            case HTML_CAPTION:
                inCaption = true;
                break;
            case HTML_FIGCAPTION:
                inFigCaption = true;
                break;
            case HTML_TABLE:
            case HTML_FIGURE:
                if ((inCaption && (ancestor._type == HTML_TABLE)) ||
                    (inFigCaption && (ancestor._type == HTML_FIGURE))) {
                    var p = DOM_createElement(document,"P");
                    DOM_insertBefore(ancestor.parentNode,p,ancestor.nextSibling);
                    Cursor_updateBRAtEndOfParagraph(p);
                    Selection_set(p,0,p,0);
                    return;
                }
                break;
            }
        }

        // Are we inside a footnote or endnote? If so, move the cursor immediately after it
        var note = null;
        if (selRange.start.node.nodeType == Node.TEXT_NODE) {
            note = Position_noteAncestor(selRange.start);
        }
        else {
            // We can't use Position_noteAncestor in this case, because we want to to break
            // the paragraph *before* the note, not after
            var checkNode = selRange.start.node;
            for (var anc = checkNode; anc != null; anc = anc.parentNode) {
                if (isNoteNode(anc)) {
                    note = anc;
                    break;
                }
            }
        }
        if (note != null) {
            var noteOffset = DOM_nodeOffset(note);
            selRange = new Range(note.parentNode,noteOffset+1,note.parentNode,noteOffset+1);
        }

        var check = Position_preferElementPosition(selRange.start);
        if (check.node.nodeType == Node.ELEMENT_NODE) {
            var before = check.node.childNodes[check.offset-1];
            var after = check.node.childNodes[check.offset];
            if (((before != null) && isSpecialBlockNode(before)) ||
                ((after != null) && isSpecialBlockNode(after))) {
                var p = DOM_createElement(document,"P");
                DOM_insertBefore(check.node,p,check.node.childNodes[check.offset]);
                Cursor_updateBRAtEndOfParagraph(p);
                Cursor_set(p,0);
                Cursor_ensureCursorVisible();
                return;
            }
        }

        Range_trackWhileExecuting(selRange,function() {
            Range_ensureInlineNodesInParagraph(selRange);
            Range_ensureValidHierarchy(selRange);
        });

        var pos = selRange.start;

        var detail = Range_detail(selRange);
        switch (detail.startParent._type) {
        case HTML_OL:
        case HTML_UL: {
            var li = DOM_createElement(document,"LI");
            DOM_insertBefore(detail.startParent,li,detail.startChild);

            Cursor_set(li,0);
            Cursor_ensureCursorVisible();
            return;
        }
        }

        if (isAutoCorrectNode(pos.node)) {
            pos = Position_preferTextPosition(pos);
            selRange.start = selRange.end = pos;
        }

        Range_trackWhileExecuting(selRange,function() {

            // If we're directly in a container node, add a paragraph, so we have something to
            // split.
            if (isContainerNode(pos.node) && (pos.node._type != HTML_LI)) {
                var p = DOM_createElement(document,"P");
                DOM_insertBefore(pos.node,p,pos.node.childNodes[pos.offset]);
                pos = new Position(p,0);
            }

            var blockToSplit = getBlockToSplit(pos);
            var stopAt = blockToSplit.parentNode;

            if (positionAtStartOfHeading(pos)) {
                var container = getContainerOrParagraph(pos.node);
                pos = new Position(container,0);
                pos = Formatting_movePreceding(pos,function(n) { return (n == stopAt); },true);
            }
            else if (pos.node.nodeType == Node.TEXT_NODE) {
                pos = Formatting_splitTextAfter(pos,function(n) { return (n == stopAt); },true);
            }
            else {
                pos = Formatting_moveFollowing(pos,function(n) { return (n == stopAt); },true);
            }
        });

        Cursor_set(pos.node,pos.offset);
        selRange = Selection_get();

        Range_trackWhileExecuting(selRange,function() {
            if ((pos.node.nodeType == Node.TEXT_NODE) && (pos.node.nodeValue.length == 0)) {
                DOM_deleteNode(pos.node);
            }

            var detail = Range_detail(selRange);

            // If a preceding paragraph has become empty as a result of enter being pressed
            // while the cursor was in it, then update the BR at the end of the paragraph
            var start = detail.startChild ? detail.startChild : detail.startParent;
            for (var ancestor = start; ancestor != null; ancestor = ancestor.parentNode) {
                var prev = ancestor.previousSibling;
                if ((prev != null) && isParagraphNode(prev) && !nodeHasContent(prev)) {
                    DOM_deleteAllChildren(prev);
                    Cursor_updateBRAtEndOfParagraph(prev);
                    break;
                }
                else if ((prev != null) && (prev._type == HTML_LI) && !nodeHasContent(prev)) {
                    var next;
                    for (var child = prev.firstChild; child != null; child = next) {
                        next = child.nextSibling;
                        if (isWhitespaceTextNode(child))
                            DOM_deleteNode(child);
                        else
                            Cursor_updateBRAtEndOfParagraph(child);
                    }
                    break;
                }
            }

            for (var ancestor = start; ancestor != null; ancestor = ancestor.parentNode) {

                if (isParagraphNode(ancestor)) {
                    var nextSelector = Styles_nextSelectorAfter(ancestor);
                    if (nextSelector != null) {
                        var nextElementName = null;
                        var nextClassName = null;


                        var dotIndex = nextSelector.indexOf(".");
                        if (dotIndex >= 0) {
                            nextElementName = nextSelector.substring(0,dotIndex);
                            nextClassName = nextSelector.substring(dotIndex+1);
                        }
                        else {
                            nextElementName = nextSelector;
                        }

                        ancestor = DOM_replaceElement(ancestor,nextElementName);
                        DOM_removeAttribute(ancestor,"id");
                        DOM_setAttribute(ancestor,"class",nextClassName);
                    }
                }

                if (isParagraphNode(ancestor) && !nodeHasContent(ancestor)) {
                    Cursor_updateBRAtEndOfParagraph(prev);
                    break;
                }
                else if ((ancestor._type == HTML_LI) && !nodeHasContent(ancestor)) {
                    DOM_deleteAllChildren(ancestor);
                    break;
                }
            }

            Cursor_updateBRAtEndOfParagraph(Range_singleNode(selRange));
        });

        Selection_set(selRange.start.node,selRange.start.offset,
                      selRange.end.node,selRange.end.offset);
        cursorX = null;
        Cursor_ensureCursorVisible();

        function getBlockToSplit(pos)
        {
            var blockToSplit = null;
            for (var n = pos.node; n != null; n = n.parentNode) {
                if (n._type == HTML_LI) {
                    blockToSplit = n;
                    break;
                }
            }
            if (blockToSplit == null) {
                blockToSplit = pos.node;
                while (isInlineNode(blockToSplit))
                    blockToSplit = blockToSplit.parentNode;
            }
            return blockToSplit;
        }

        function getContainerOrParagraph(node)
        {
            while ((node != null) && isInlineNode(node))
                node = node.parentNode;
            return node;
        }

        function positionAtStartOfHeading(pos)
        {
            var container = getContainerOrParagraph(pos.node);
            if (isHeadingNode(container)) {
                var startOffset = 0;
                if (isOpaqueNode(container.firstChild))
                    startOffset = 1;
                var range = new Range(container,startOffset,pos.node,pos.offset);
                return !Range_hasContent(range);
            }
            else
                return false;
        }
    }

    Cursor_getPrecedingWord = function() {
        var selRange = Selection_get();
        if ((selRange == null) && !Range_isEmpty(selRange))
            return "";

        var node = selRange.start.node;
        var offset = selRange.start.offset;
        if (node.nodeType != Node.TEXT_NODE)
            return "";

        return node.nodeValue.substring(0,offset);
    }

    Cursor_getAdjacentNodeWithType = function(type)
    {
        var selRange = Selection_get();
        var pos = Position_preferElementPosition(selRange.start);
        var node = pos.node;
        var offset = pos.offset;

        while (true) {

            if (node._type == type)
                return node;

            if (node.nodeType == Node.ELEMENT_NODE) {
                var before = node.childNodes[offset-1];
                if ((before != null) && (before._type == type))
                    return before;

                var after = node.childNodes[offset];
                if ((after != null) && (after._type == type))
                    return after;
            }

            if (node.parentNode == null)
                return null;

            offset = DOM_nodeOffset(node);
            node = node.parentNode;
        }
    }

    Cursor_getLinkProperties = function()
    {
        var a = Cursor_getAdjacentNodeWithType(HTML_A);
        if (a == null)
            return null;

        return { href: a.getAttribute("href"),
                 text: getNodeText(a) };
    }

    Cursor_setLinkProperties = function(properties)
    {
        var a = Cursor_getAdjacentNodeWithType(HTML_A);
        if (a == null)
            return null;

        Selection_preserveWhileExecuting(function() {
            DOM_setAttribute(a,"href",properties.href);
            DOM_deleteAllChildren(a);
            DOM_appendChild(a,DOM_createTextNode(document,properties.text));
        });
    }

    Cursor_setReferenceTarget = function(itemId)
    {
        var a = Cursor_getAdjacentNodeWithType(HTML_A);
        if (a != null)
            Outline_setReferenceTarget(a,itemId);
    }

    // Deletes the current selection contents and ensures that the cursor is located directly
    // inside the nearest container element, i.e. not inside a paragraph or inline node. This
    // is intended for preventing things like inserting a table of contants inside a heading
    Cursor_makeContainerInsertionPoint = function()
    {
        var selRange = Selection_get();
        if (selRange == null)
            return;

        if (!Range_isEmpty(selRange)) {
            Selection_deleteContents();
            selRange = Selection_get();
        }

        var parent;
        var previousSibling;
        var nextSibling;

        if (selRange.start.node.nodeType == Node.ELEMENT_NODE) {
            parent = selRange.start.node;
            nextSibling = selRange.start.node.childNodes[selRange.start.offset];
        }
        else {
            if (selRange.start.offset > 0)
                Formatting_splitTextBefore(selRange.start);
            parent = selRange.start.node.parentNode;
            nextSibling = selRange.start.node;
        }

        var offset = DOM_nodeOffset(nextSibling,parent);

        if (isContainerNode(parent)) {
            Cursor_set(parent,offset);
            return;
        }

        if ((offset > 0) && isItemNumber(parent.childNodes[offset-1]))
            offset--;

        Formatting_moveFollowing(new Position(parent,offset),isContainerNode);
        Formatting_movePreceding(new Position(parent,offset),isContainerNode);

        offset = 0;
        while (!isContainerNode(parent)) {
            var old = parent;
            offset = DOM_nodeOffset(parent);
            parent = parent.parentNode;
            DOM_deleteNode(old);
        }

        Cursor_set(parent,offset);
        cursorX = null;
    }

    Cursor_set = function(node,offset,keepCursorX)
    {
        Selection_set(node,offset,node,offset);
        if (!keepCursorX)
            cursorX = null;
    }

    function moveRangeOutsideOfNote(range)
    {
        var node = range.start.node;
        var offset = range.start.offset;

        for (var anc = node; anc != null; anc = anc.parentNode) {
            if (isNoteNode(anc) && (anc.parentNode != null)) {
                node = anc.parentNode;
                offset = DOM_nodeOffset(anc)+1;
                return new Range(node,offset,node,offset);
            }
        }

        return range;
    }

    function insertNote(className,content)
    {
        var footnote = DOM_createElement(document,"span");
        DOM_setAttribute(footnote,"class",className);
        DOM_appendChild(footnote,DOM_createTextNode(document,content));

        var range = Selection_get();
        range = moveRangeOutsideOfNote(range);
        Formatting_splitAroundSelection(range,false);

        // If we're part-way through a text node, splitAroundSelection will give us an
        // empty text node between the before and after text. For formatting purposes that's
        // fine (not sure if necessary), but when inserting a footnote or endnote we want
        // to avoid this as it causes problems with cursor movement - specifically, the cursor
        // is allowed to go inside the empty text node, and this doesn't show up in the correct
        // position on screen.
        var pos = range.start;
        if ((pos.node._type == HTML_TEXT) &&
            (pos.node.nodeValue.length == 0)) {
            var empty = pos.node;
            pos = new Position(empty.parentNode,DOM_nodeOffset(empty));
            DOM_deleteNode(empty);
        }
        else {
            pos = Position_preferElementPosition(pos);
        }

        DOM_insertBefore(pos.node,footnote,pos.node.childNodes[pos.offset]);
        Selection_set(footnote,0,footnote,footnote.childNodes.length);
        Cursor_updateBRAtEndOfParagraph(footnote);
    }

    Cursor_insertFootnote = function(content)
    {
        insertNote("footnote",content);
    }

    Cursor_insertEndnote = function(content)
    {
        insertNote("endnote",content);
    }

})();
