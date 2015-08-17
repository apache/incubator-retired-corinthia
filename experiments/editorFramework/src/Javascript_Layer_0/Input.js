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

var Input_removePosition;
var Input_addPosition;
var Input_getPosition;
var Input_textInRange;
var Input_replaceRange;
var Input_selectedTextRange;
var Input_setSelectedTextRange;
var Input_markedTextRange;
var Input_setMarkedText;
var Input_unmarkText;
var Input_forwardSelectionAffinity;
var Input_setForwardSelectionAffinity;
var Input_positionFromPositionOffset;
var Input_positionFromPositionInDirectionOffset;
var Input_comparePositionToPosition;
var Input_offsetFromPositionToPosition;
var Input_positionWithinRangeFarthestInDirection;
var Input_characterRangeByExtendingPositionInDirection;
var Input_firstRectForRange;
var Input_caretRectForPosition;
var Input_closestPositionToPoint;
var Input_closestPositionToPointWithinRange;
var Input_characterRangeAtPoint;
var Input_positionWithinRangeAtCharacterOffset;
var Input_characterOffsetOfPositionWithinRange;

var Input_isAtWordBoundary;
var Input_isAtParagraphBoundary;
var Input_isPositionAtBoundaryGranularityInDirection;
var Input_isPositionWithinTextUnitInDirection;
var Input_toWordBoundary;
var Input_toParagraphBoundary;
var Input_toLineBoundary;
var Input_positionFromPositionToBoundaryInDirection;
var Input_rangeEnclosingPositionWithGranularityInDirection;

// FIXME: ensure updateFormatting() is called after any cursor/selection changes
// FIXME: test capitalisation of on-screen keyboard at start of sentence

(function() {

    //function idebug(str)
    //{
    //    debug(str);
    //}

    var forwardSelection = true;
    var positions = new Object();
    var BaseIdNull = 0;
    var BaseIdDocumentStart = 1;
    var BaseIdDocumentEnd = 2;
    var BaseIdSelectionStart = 3;
    var BaseIdSelectionEnd = 4;
    var firstDynamicPosId = 5;
    var nextPosId = firstDynamicPosId;

    function addPosition(pos)
    {
        if (pos == null)
            return 0;
        var copy = new Position(pos.node,pos.offset);
        copy.targetX = pos.targetX;
        pos = copy;
        pos.posId = nextPosId++;
        positions[pos.posId] = pos;
        Position_track(pos);
        return pos.posId;
    }

    Input_addPosition = addPosition;

    function getPosition(posId)
    {
        if (posId instanceof Position) // for tests
            return posId;
        if (posId < firstDynamicPosId) {
            switch (posId) {
            case BaseIdNull: {
                return null;
            }
            case BaseIdDocumentStart: {
                var pos = new Position(document.body,0);
                pos = Position_closestMatchForwards(pos,Position_okForMovement);
                return pos;
            }
            case BaseIdDocumentEnd: {
                var pos = new Position(document.body,document.body.childNodes.length);
                pos = Position_closestMatchBackwards(pos,Position_okForMovement);
                return pos;
            }
            case BaseIdSelectionStart: {
                var range = Selection_get();
                return (range != null) ? range.start : null;
            }
            case BaseIdSelectionEnd: {
                var range = Selection_get();
                return (range != null) ? range.end : null;
            }
            default:
                return null;
            }
        }
        if (positions[posId] == null)
            throw new Error("No position for pos id "+posId);
        return positions[posId];
    }

    Input_getPosition = getPosition;

    // void
    Input_removePosition = function(posId)
    {
        //idebug("Input_removePosition("+posId+")");
        var pos = positions[posId];
        if (pos == null) {
            throw new Error("no position for id "+posId);
        }
        Position_untrack(pos);
        delete positions[posId];
    }

    // string
    Input_textInRange = function(startId,startAdjust,endId,endAdjust)
    {
        var start = getPosition(startId);
        var end = getPosition(endId);
        start = positionRight(start,startAdjust);
        end = positionRight(end,endAdjust);
        if ((start == null) || (end == null))
            return "";

        var range = new Range(start.node,start.offset,end.node,end.offset);
        var result = Range_getText(range);
        //idebug("Input_textInRange("+startId+","+startAdjust+","+endId+","+endAdjust+") = "+
        //       JSON.stringify(result));
        return result;
    }

    // void
    Input_replaceRange = function(startId,endId,text)
    {
        //idebug("Input_replaceRange("+startId+","+endId+","+JSON.stringify(text)+")");
        var start = getPosition(startId);
        var end = getPosition(endId);
        if (start == null)
            throw new Error("start is null");
        if (end == null)
            throw new Error("end is null");

        var range = new Range(start.node,start.offset,end.node,end.offset);
        Range_trackWhileExecuting(range,function() {
            Selection_deleteRangeContents(range,true);
        });
        range.start = Position_preferTextPosition(range.start);
        var node = range.start.node;
        var offset = range.start.offset;

        if (node.nodeType == Node.TEXT_NODE) {
            DOM_insertCharacters(node,offset,text);
            Cursor_set(node,offset+text.length);
        }
        else if (node.nodeType == Node.ELEMENT_NODE) {
            var textNode = DOM_createTextNode(document,text);
            DOM_insertBefore(node,textNode,node.childNodes[offset]);
            Cursor_set(node,offset+1);
        }
    }

    // { startId, endId }
    Input_selectedTextRange = function()
    {
        var range = Selection_get();
        if (range == null) {
            //idebug("Input_selectedTextRange = null");
            return null;
        }
        else {
            var startId = addPosition(range.start);
            var endId = addPosition(range.end);
            //idebug("Input_selectedTextRange = "+startId+", "+endId);
            return { startId: startId,
                     endId: endId };
        }
    }

    // void
    Input_setSelectedTextRange = function(startId,endId)
    {
        //idebug("Input_setSelectedTextRange("+startId+","+endId+")");
        var start = getPosition(startId);
        var end = getPosition(endId);

        var oldSelection = Selection_get();
        var oldStart = (oldSelection != null) ? oldSelection.start : null;
        var oldEnd = (oldSelection != null) ? oldSelection.end : null;

        Selection_set(start.node,start.offset,end.node,end.offset);

        // The positions may have changed as a result of spans being added/removed
        var newRange = Selection_get();
        start = newRange.start;
        end = newRange.end;

        if (Position_equal(start,end))
            Cursor_ensurePositionVisible(end);
        else if (Position_equal(oldStart,start) && !Position_equal(oldEnd,end))
            Cursor_ensurePositionVisible(end);
        else if (Position_equal(oldEnd,end) && !Position_equal(oldStart,start))
            Cursor_ensurePositionVisible(start);
    }

    // { startId, endId }
    Input_markedTextRange = function()
    {
        //idebug("Input_markedTextRange");
        return null;
    }

    // void
    Input_setMarkedText = function(text,startOffset,endOffset)
    {
        Selection_deleteContents(true);
        var oldSel = Selection_get();
        Range_trackWhileExecuting(oldSel,function() {
            Cursor_insertCharacter(text,false,false,true);
        });
        var newSel = Selection_get();

        Selection_set(oldSel.start.node,oldSel.start.offset,
                      newSel.end.node,newSel.end.offset,false,true);
    }

    // void
    Input_unmarkText = function()
    {
        var range = Selection_get();
        Cursor_set(range.end.node,range.end.offset);
        //idebug("Input_unmarkText");
    }

    // boolean
    Input_forwardSelectionAffinity = function()
    {
        //idebug("Input_forwardSelectionAffinity");
        return forwardSelection;
    }

    // void
    Input_setForwardSelectionAffinity = function(value)
    {
        //idebug("Input_setForwardSelectionAffinity");
        forwardSelection = value;
    }

    function positionRight(pos,offset)
    {
        if (offset > 0) {
            for (; offset > 0; offset--) {
                var next = Position_nextMatch(pos,Position_okForMovement);
                if (next == null)
                    return pos;
                pos = next;
            }
        }
        else {
            for (; offset < 0; offset++) {
                var prev = Position_prevMatch(pos,Position_okForMovement);
                if (prev == null)
                    return pos;
                pos = prev;
            }
        }
        return pos;
    }

    function positionDown(pos,offset)
    {
        if (offset > 0) {
            for (; offset > 0; offset--) {
                var below = Text_posBelow(pos);
                if (below == null)
                    return pos;
                pos = below;
            }
        }
        else {
            for (; offset < 0; offset++) {
                var above = Text_posAbove(pos);
                if (above == null)
                    return pos;
                pos = above;
            }
        }
        return pos;
    }

    // posId
    Input_positionFromPositionOffset = function(posId,offset)
    {
        var pos = getPosition(posId);
        var res = addPosition(positionRight(pos,offset));
        //idebug("Input_positionFromPositionOffset("+posId+","+offset+") = "+res);
        return res;
    }

    // posId
    Input_positionFromPositionInDirectionOffset = function(posId,direction,offset)
    {
        //idebug("Input_positionFromPositionInDirectionOffset("+posId+","+direction+","+offset+")");
        var pos = getPosition(posId);
        if (direction == "left")
            return addPosition(positionRight(pos,-offset));
        else if (direction == "right")
            return addPosition(positionRight(pos,offset));
        else if (direction == "up")
            return addPosition(positionDown(pos,-offset));
        else if (direction == "down")
            return addPosition(positionDown(pos,offset));
        else
            throw new Error("unknown direction: "+direction);
    }

    // int
    Input_comparePositionToPosition = function(posId1,posId2)
    {
        //idebug("Input_comparePositionToPosition("+posId1+","+posId2+")");
        var pos1 = getPosition(posId1);
        var pos2 = getPosition(posId2);
        if (pos1 == null)
            throw new Error("pos1 is null");
        if (pos2 == null)
            throw new Error("pos2 is null");
        return Position_compare(pos1,pos2);
    }

    // int
    Input_offsetFromPositionToPosition = function(fromId,toId)
    {
        //idebug("Input_offsetFromPositionToPosition("+fromId+","+toId+")");
        throw new Error("offsetFromPositionToPosition: not implemented");
    }

    Input_positionWithinRangeFarthestInDirection = function(startId,endId,direction)
    {
        //idebug("Input_positionWithinRangeFarthestInDirection("+startId+","+endId+","+direction);
        throw new Error("positionWithinRangeFarthestInDirection: not implemented");
    }

    // { startId, endId }
    Input_characterRangeByExtendingPositionInDirection = function(posId,direction)
    {
        //idebug("Input_characterRangeByExtendingPositionInDirection("+posId+","+direction);
        throw new Error("characterRangeByExtendingPositionInDirection: not implemented");
    }

    Input_firstRectForRange = function(startId,endId)
    {
        //idebug("Input_firstRectForRange("+startId+","+endId+")");
        var start = getPosition(startId);
        var end = getPosition(endId);
        var range = new Range(start.node,start.offset,end.node,end.offset);
        var rects = Range_getClientRects(range);
        if (rects.length == 0)
            return { x: 0, y: 0, width: 0, height: 0 };
        else
            return { x: rects[0].left, y: rects[0].top,
                     width: rects[0].width, height: rects[0].height };
    }

    Input_caretRectForPosition = function(posId)
    {
        //idebug("Input_caretRectForPosition("+posId+")");
        var pos = getPosition(posId);
        var rect = Position_rectAtPos(pos);
        if (rect == null)
            return { x: 0, y: 0, width: 0, height: 0 };
        else
            return { x: rect.left, y: rect.top, width: rect.width, height: rect.height };
    }

    // posId
    Input_closestPositionToPoint = function(x,y)
    {
        //idebug("Input_closestPositionToPoint("+x+","+y+")");
        throw new Error("closestPositionToPoint: not implemented");
    }

    // posId
    Input_closestPositionToPointWithinRange = function(x,y,startId,endId)
    {
        //idebug("Input_closestPositionToPointWithinRange("+x+","+y+")");
        throw new Error("closestPositionToPointWithinRange: not implemented");
    }

    // { startId, endId }
    Input_characterRangeAtPoint = function(x,y)
    {
        //idebug("Input_characterRangeAtPoint("+x+","+y+")");
        throw new Error("characterRangeAtPoint: not implemented");
    }

    // posId
    Input_positionWithinRangeAtCharacterOffset = function(startId,endId,offset)
    {
        //idebug("Input_positionWithinRangeAtCharacterOffset("+startId+","+endId+","+offset+")");
        throw new Error("positionWithinRangeAtCharacterOffset: not implemented");
    }

    // int
    Input_characterOffsetOfPositionWithinRange = function(posId,startId,endId)
    {
        //idebug("Input_characterOffsetOfPositionWithinRange("+posId+","+startId+","+endId+")");
        throw new Error("characterOffsetOfPositionWithinRange: not implemented");
    }

    // UITextInputTokenizer methods

    var punctuation = "!\"#%&',-/:;<=>@`~\\^\\$\\\\\\.\\*\\+\\?\\(\\)\\[\\]\\{\\}\\|";
    var letterRE = new RegExp("[^\\s"+punctuation+"]");
    var wordAtStartRE = new RegExp("^[^\\s"+punctuation+"]+");
    var nonWordAtStartRE = new RegExp("^[\\s"+punctuation+"]+");
    var wordAtEndRE = new RegExp("[^\\s"+punctuation+"]+$");
    var nonWordAtEndRE = new RegExp("[\\s"+punctuation+"]+$");

    function isForward(direction)
    {
        return ((direction == "forward") ||
                (direction == "right") ||
                (direction == "down"));
    }

    Input_isAtWordBoundary = function(pos,direction)
    {
        if (pos.node.nodeType != Node.TEXT_NODE)
            return false;
        var paragraph = Text_analyseParagraph(pos);
        if (paragraph == null)
            return false;
        var offset = Paragraph_offsetAtPosition(paragraph,pos);
        var before = paragraph.text.substring(0,offset);
        var after = paragraph.text.substring(offset);
        var text = paragraph.text;

        var afterMatch = (offset < text.length) && (text.charAt(offset).match(letterRE));
        var beforeMatch = (offset > 0) && (text.charAt(offset-1).match(letterRE));

        // coerce to boolean
        afterMatch = !!afterMatch;
        beforeMatch = !!beforeMatch;

        if (isForward(direction))
            return beforeMatch && !afterMatch;
        else
            return !beforeMatch;
    }

    Input_isAtParagraphBoundary = function(pos,direction)
    {
    }

    Input_isPositionAtBoundaryGranularityInDirection = function(posId,granularity,direction)
    {
        //idebug("Input_isPositionAtBoundaryGranularityInDirection("+
        //       posId+","+granularity+","+direction+")");
        var pos = getPosition(posId);
        if (pos == null)
            return false;

        // FIXME: Temporary hack to avoid exceptions when running under iOS 8
        if ((granularity == "sentence") || (granularity == "document"))
            return false;

        if (granularity == "character") {
            return true;
        }
        else if (granularity == "word") {
            return Input_isAtWordBoundary(pos,direction);
        }
        else if ((granularity == "paragraph") || (granularity == "line")) {
            if (isForward(direction))
                return Position_equal(pos,Text_toEndOfBoundary(pos,granularity));
            else
                return Position_equal(pos,Text_toStartOfBoundary(pos,granularity));
        }
        else if (granularity == "sentence") {
        }
        else if (granularity == "document") {
        }
        throw new Error("unsupported granularity: "+granularity);
    }

    Input_isPositionWithinTextUnitInDirection = function(posId,granularity,direction)
    {
        //idebug("Input_isPositionWithinTextUnitInDirection("+
        //       posId+","+granularity+","+direction+")");
        var pos = getPosition(posId);
        if (pos == null)
            return false;

        // FIXME: Temporary hack to avoid exceptions when running under iOS 8
        if ((granularity == "sentence") || (granularity == "document"))
            return true;

        if (granularity == "character") {
            return true;
        }
        else if (granularity == "word") {
            pos = Text_closestPosInDirection(pos,direction);
            if (pos == null)
                return false;
            var paragraph = Text_analyseParagraph(pos);
            if (paragraph == null)
                return false;
            if ((pos != null) && (pos.node.nodeType == Node.TEXT_NODE)) {
                var offset = Paragraph_offsetAtPosition(paragraph,pos);
                var text = paragraph.text;
                if (isForward(direction))
                    return !!((offset < text.length) && (text.charAt(offset).match(letterRE)));
                else
                    return !!((offset > 0) && (text.charAt(offset-1).match(letterRE)));
            }
            else {
                return false;
            }
        }
        else if (granularity == "sentence") {
        }
        else if ((granularity == "paragraph") || (granularity == "line")) {
            var start = Text_toStartOfBoundary(pos,granularity);
            var end = Text_toEndOfBoundary(pos,granularity);
            start = start ? start : pos;
            end = end ? end : pos;
            if (isForward(direction)) {
                return ((Position_compare(start,pos) <= 0) &&
                        (Position_compare(pos,end) < 0));
            }
            else {
                return ((Position_compare(start,pos) < 0) &&
                        (Position_compare(pos,end) <= 0));
            }
        }
        else if (granularity == "document") {
        }
        throw new Error("unsupported granularity: "+granularity);
    }

    Input_toWordBoundary = function(pos,direction)
    {
        pos = Text_closestPosInDirection(pos,direction);
        if (pos == null)
            return null;
        var paragraph = Text_analyseParagraph(pos);
        if (paragraph == null)
            return null;
        var run = Paragraph_runFromNode(paragraph,pos.node);
        var offset = pos.offset + run.start;

        if (isForward(direction)) {
            var remaining = paragraph.text.substring(offset);
            var afterWord = remaining.replace(wordAtStartRE,"");
            var afterNonWord = remaining.replace(nonWordAtStartRE,"");

            if (remaining.length == 0) {
                return pos;
            }
            else if (afterWord.length < remaining.length) {
                var newOffset = offset + (remaining.length - afterWord.length);
                return Paragraph_positionAtOffset(paragraph,newOffset);
            }
            else {
                var newOffset = offset + (remaining.length - afterNonWord.length);
                return Paragraph_positionAtOffset(paragraph,newOffset);
            }
        }
        else {
            var remaining = paragraph.text.substring(0,offset);
            var beforeWord = remaining.replace(wordAtEndRE,"");
            var beforeNonWord = remaining.replace(nonWordAtEndRE,"");

            if (remaining.length == 0) {
                return pos;
            }
            else if (beforeWord.length < remaining.length) {
                var newOffset = offset - (remaining.length - beforeWord.length);
                return Paragraph_positionAtOffset(paragraph,newOffset);
            }
            else {
                var newOffset = offset - (remaining.length - beforeNonWord.length);
                return Paragraph_positionAtOffset(paragraph,newOffset);
            }
        }
    }

    Input_toParagraphBoundary = function(pos,direction)
    {
        if (isForward(direction)) {
            var end = Text_toEndOfBoundary(pos,"paragraph");
            if (Position_equal(pos,end)) {
                end = Position_nextMatch(end,Position_okForMovement);
                end = Text_toEndOfBoundary(end,"paragraph");
                end = Text_toStartOfBoundary(end,"paragraph");
            }
            return end ? end : pos;
        }
        else {
            var start = Text_toStartOfBoundary(pos,"paragraph");
            if (Position_equal(pos,start)) {
                start = Position_prevMatch(start,Position_okForMovement);
                start = Text_toStartOfBoundary(start,"paragraph");
                start = Text_toEndOfBoundary(start,"paragraph");
            }
            return start ? start : pos;
        }
    }

    Input_toLineBoundary = function(pos,direction)
    {
        if (isForward(direction)) {
            var end = Text_toEndOfBoundary(pos,"line");
            return end ? end : pos;
        }
        else {
            var start = Text_toStartOfBoundary(pos,"line");
            return start ? start : pos;
        }
    }

    Input_positionFromPositionToBoundaryInDirection = function(posId,granularity,direction)
    {
        //idebug("Input_positionFromPositionToBoundaryInDirection("+
        //       posId+","+granularity+","+direction+")");
        var pos = getPosition(posId);
        if (pos == null)
            return null;

        // FIXME: Temporary hack to avoid exceptions when running under iOS 8
        if (granularity == "sentence")
            granularity = "paragraph";

        if (granularity == "word")
            return addPosition(Input_toWordBoundary(pos,direction));
        else if (granularity == "paragraph")
            return addPosition(Input_toParagraphBoundary(pos,direction));
        else if (granularity == "line")
            return addPosition(Input_toLineBoundary(pos,direction));
        else if (granularity == "character")
            return Input_positionFromPositionInDirectionOffset(posId,direction,1);
        else if (granularity == "document")
            return isForward(direction) ? BaseIdDocumentEnd : BaseIdDocumentStart;
        else
            throw new Error("unsupported granularity: "+granularity);
    }

    Input_rangeEnclosingPositionWithGranularityInDirection = function(posId,granularity,direction)
    {
        //idebug("Input_rangeEnclosingPositionWithGranularityInDirection("+
        //       posId+","+granularity+","+direction);
        var pos = getPosition(posId);
        if (pos == null)
            return null;

        // FIXME: Temporary hack to avoid exceptions when running under iOS 8
        if (granularity == "sentence")
            granularity = "paragraph";

        if (granularity == "word") {
            pos = Text_closestPosInDirection(pos,direction);
            if (pos == null)
                return null;
            var paragraph = Text_analyseParagraph(pos);
            if (pos == null)
                return addPosition(null);
            if (paragraph == null)
                return addPosition(null);
            var run = Paragraph_runFromNode(paragraph,pos.node);
            var offset = pos.offset + run.start;

            var before = paragraph.text.substring(0,offset);
            var after = paragraph.text.substring(offset);
            var beforeWord = before.replace(wordAtEndRE,"");
            var afterWord = after.replace(wordAtStartRE,"");

            var ok;

            if (isForward(direction))
                ok = (afterWord.length < after.length);
            else
                ok = (beforeWord.length < before.length);

            if (ok) {
                var charsBefore = (before.length - beforeWord.length);
                var charsAfter = (after.length - afterWord.length);
                var startOffset = offset - charsBefore;
                var endOffset = offset + charsAfter;

                var startPos = Paragraph_positionAtOffset(paragraph,startOffset);
                var endPos = Paragraph_positionAtOffset(paragraph,endOffset);
                return { startId: addPosition(startPos),
                         endId: addPosition(endPos) };
            }
            else {
                return null;
            }
        }
        else if ((granularity == "paragraph") || (granularity == "line")) {
            var start = Text_toStartOfBoundary(pos,granularity);
            var end = Text_toEndOfBoundary(pos,granularity);
            start = start ? start : pos;
            end = end ? end : pos;

            if ((granularity == "paragraph") || !isForward(direction)) {
                if (isForward(direction)) {
                    if (Position_equal(pos,Text_toEndOfBoundary(pos,granularity)))
                        return null;
                }
                else {
                    if (Position_equal(pos,Text_toStartOfBoundary(pos,granularity)))
                        return null;
                }
            }
            return { startId: addPosition(start),
                     endId: addPosition(end) };
        }
        else {
            throw new Error("unsupported granularity: "+granularity);
        }
    }

})();
