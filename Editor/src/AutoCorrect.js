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

var AutoCorrect_init;
var AutoCorrect_removeListeners;
var AutoCorrect_addCorrection;
var AutoCorrect_removeCorrection;
var AutoCorrect_getCorrections;

var AutoCorrect_correctPrecedingWord;
var AutoCorrect_getCorrection;
var AutoCorrect_getCorrectionCoords;
var AutoCorrect_acceptCorrection;
var AutoCorrect_revertCorrection;
var AutoCorrect_replaceCorrection;

(function() {

    function removeCorrectionSpan(span)
    {
        if (span.parentNode == null)
            return;
        Selection_preserveWhileExecuting(function() {
            var firstChild = span.firstChild;
            DOM_removeNodeButKeepChildren(span);
            if (firstChild != null)
                Formatting_mergeWithNeighbours(firstChild,{});
        });
    }

    function Correction(span)
    {
        this.span = span;
        this.modificationListener = function(event) {
            if (DOM_getIgnoreMutations())
                return;
            PostponedActions_add(function() {
                // This will trigger a removeCorrection() call
                removeCorrectionSpan(span);
            });
        };
    }

    Correction.prototype.toString = function()
    {
        return this.span.getAttribute("original")+" -> "+getNodeText(this.span);
    }

    var correctionsByNode = null;
    var correctionList = null;

    // private
    function docNodeInserted(event)
    {
        try {
            recurse(event.target);
        }
        catch (e) {
            Editor_error(e);
        }

        function recurse(node)
        {
            if (isAutoCorrectNode(node))
                AutoCorrect_addCorrection(node);
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }

    // private
    function docNodeRemoved(event)
    {
        try {
            recurse(event.target);
        }
        catch (e) {
            Editor_error(e);
        }

        function recurse(node)
        {
            if (isAutoCorrectNode(node))
                AutoCorrect_removeCorrection(node);
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }

    AutoCorrect_init = function()
    {
        correctionsByNode = new NodeMap();
        correctionList = new Array();
        document.addEventListener("DOMNodeInserted",docNodeInserted);
        document.addEventListener("DOMNodeRemoved",docNodeRemoved);
    }

    // public (for the undo tests, when they report results)
    AutoCorrect_removeListeners = function()
    {
        document.removeEventListener("DOMNodeInserted",docNodeInserted);
        document.removeEventListener("DOMNodeRemoved",docNodeRemoved);
    }

    AutoCorrect_addCorrection = function(span)
    {
        var correction = new Correction(span);
        correctionsByNode.put(span,correction);
        correctionList.push(correction);
        Editor_updateAutoCorrect();

        span.addEventListener("DOMSubtreeModified",correction.modificationListener);
    }

    AutoCorrect_removeCorrection = function(span)
    {
        var correction = correctionsByNode.get(span);
        if (correction == null)
            throw new Error("No autocorrect entry for "+JSON.stringify(getNodeText(span)));

        var index = null;
        for (var i = 0; i < correctionList.length; i++) {
            if (correctionList[i].span == span) {
                index = i;
                break;
            }
        }
        if (index == null)
            throw new Error("Correction "+correction+" not found in correctionList");
        correctionList.splice(index,1);
        Editor_updateAutoCorrect();

        span.removeEventListener("DOMSubtreeModified",correction.modificationListener);
        correctionsByNode.remove(span);
    }

    AutoCorrect_getCorrections = function()
    {
        var result = new Array();
        for (var i = 0; i < correctionList.length; i++) {
            var correction = correctionList[i];
            result.push({ original: correction.span.getAttribute("original"),
                          replacement: getNodeText(correction.span)});
        }
        return result;
    }

    AutoCorrect_correctPrecedingWord = function(numChars,replacement,confirmed)
    {
        Selection_preserveWhileExecuting(function() {
            var selRange = Selection_get();
            if ((selRange == null) && !Range_isEmpty(selRange))
                return;

            var node = selRange.start.node;
            var offset = selRange.start.offset;
            if (node.nodeType != Node.TEXT_NODE)
                return;

            var original = node.nodeValue.substring(offset-numChars,offset);

            if (confirmed) {
                DOM_replaceCharacters(node,offset-numChars,offset,replacement);
                return;
            }

            UndoManager_newGroup("Auto-correct");
            var before = node.nodeValue.substring(0,offset-numChars);
            var beforeText = DOM_createTextNode(document,before);
            var replacementText = DOM_createTextNode(document,replacement);
            var span = DOM_createElement(document,"SPAN");
            DOM_setAttribute(span,"class",Keys.AUTOCORRECT_CLASS);
            DOM_setAttribute(span,"original",original);
            DOM_appendChild(span,replacementText);
            DOM_insertBefore(node.parentNode,beforeText,node);
            DOM_insertBefore(node.parentNode,span,node);
            DOM_deleteCharacters(node,0,offset);
            // Add the new group in a postponed action, so that the change to the style element
            // is not counted as a separate action
            PostponedActions_add(UndoManager_newGroup);
        });
    }

    AutoCorrect_getCorrection = function()
    {
        var correction = getCurrent();
        if (correction == null)
            return null;

        return { original: correction.span.getAttribute("original"),
                 replacement: getNodeText(correction.span) };
    }

    AutoCorrect_getCorrectionCoords = function()
    {
        var correction = getCurrent();
        if (correction == null)
            return null;

        var textNode = correction.span.firstChild;
        if ((textNode == null) || (textNode.nodeType != Node.TEXT_NODE))
            return null;

        var offset = Math.floor(textNode.nodeValue.length/2);
        Selection_set(textNode,offset,textNode,offset);
        Cursor_ensureCursorVisible();
        var rect = Position_displayRectAtPos(new Position(textNode,offset));

        if (rect == null) // FIXME: pos
            throw new Error("no rect for pos "+(new Position(textNode,offset)));

        if (rect == null)
            return null;

        return { x: rect.left, y: rect.top };
    }

    function getCurrent()
    {
        var range = Selection_get();
        if (range != null) {
            var endNode = Position_closestActualNode(range.end);
            for (; endNode != null; endNode = endNode.parentNode) {
                if (isAutoCorrectNode(endNode))
                    return correctionsByNode.get(endNode);
            }
        }

        if (correctionList.length > 0)
            return correctionList[correctionList.length-1];

        return null;
    }

    AutoCorrect_acceptCorrection = function()
    {
        UndoManager_newGroup("Accept");
        var correction = getCurrent();
        if (correction == null)
            return;

        removeCorrectionSpan(correction.span);
        UndoManager_newGroup();
    }

    AutoCorrect_revertCorrection = function()
    {
        var correction = getCurrent();
        if (correction == null)
            return;

        AutoCorrect_replaceCorrection(correction.span.getAttribute("original"));
    }

    AutoCorrect_replaceCorrection = function(replacement)
    {
        UndoManager_newGroup("Replace");
        var correction = getCurrent();
        if (correction == null)
            return;

        Selection_preserveWhileExecuting(function() {
            var text = DOM_createTextNode(document,replacement);
            DOM_insertBefore(correction.span.parentNode,text,correction.span);
            DOM_deleteNode(correction.span);
            Formatting_mergeWithNeighbours(text,{});
        });
        UndoManager_newGroup();
    }

})();
