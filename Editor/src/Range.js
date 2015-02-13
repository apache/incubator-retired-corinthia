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

var Range;

var Range_assertValid;
var Range_isEmpty;
var Range_trackWhileExecuting;
var Range_expand;
var Range_isForwards;
var Range_getAllNodes;
var Range_singleNode;
var Range_ensureInlineNodesInParagraph;
var Range_ensureValidHierarchy;
var Range_forwards;
var Range_detail;
var Range_getOutermostNodes;
var Range_getClientRects;
var Range_cloneContents;
var Range_hasContent;
var Range_getText;

(function() {

    Range = function(startNode,startOffset,endNode,endOffset)
    {
        this.start = new Position(startNode,startOffset);
        this.end = new Position(endNode,endOffset);
    }

    Range_assertValid = function(range,description)
    {
        if (description == null)
            description = "Range";
        if (range == null)
            throw new Error(description+" is null");
        Position_assertValid(range.start,description+" start");
        Position_assertValid(range.end,description+" end");
    }

    Range_isEmpty = function(range)
    {
        return ((range.start.node == range.end.node) &&
                (range.start.offset == range.end.offset));
    }

    Range.prototype.toString = function()
    {
        return this.start.toString() + " - " + this.end.toString();
    }

    Range_trackWhileExecuting = function(range,fun)
    {
        if (range == null)
            return fun();
        else
            return Position_trackWhileExecuting([range.start,range.end],fun);
    }

    Range_expand = function(range)
    {
        var doc = range.start.node.ownerDocument;
        while ((range.start.offset == 0) && (range.start.node != doc.body)) {
            var offset = DOM_nodeOffset(range.start.node);
            range.start.node = range.start.node.parentNode;
            range.start.offset = offset;
        }

        while ((range.end.offset == DOM_maxChildOffset(range.end.node)) &&
               (range.end.node != doc.body)) {
            var offset = DOM_nodeOffset(range.end.node);
            range.end.node = range.end.node.parentNode;
            range.end.offset = offset+1;
        }
    }

    Range_isForwards = function(range)
    {
        return (Position_compare(range.start,range.end) <= 0);
    }

    Range_getAllNodes = function(range,atLeastOne)
    {
        var result = new Array();
        var outermost = Range_getOutermostNodes(range,atLeastOne);
        for (var i = 0; i < outermost.length; i++)
            addRecursive(outermost[i]);
        return result;

        function addRecursive(node)
        {
            result.push(node);
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                addRecursive(child);
        }
    }

    Range_singleNode = function(range)
    {
        return Position_closestActualNode(range.start,true);
    }

    Range_ensureInlineNodesInParagraph = function(range)
    {
        Range_trackWhileExecuting(range,function() {
            var nodes = Range_getAllNodes(range,true);
            for (var i = 0; i < nodes.length; i++)
                Hierarchy_ensureInlineNodesInParagraph(nodes[i]);
        });
    }

    Range_ensureValidHierarchy = function(range,allowDirectInline)
    {
        Range_trackWhileExecuting(range,function() {
            var nodes = Range_getAllNodes(range,true);
            for (var i = nodes.length-1; i >= 0; i--)
                Hierarchy_ensureValidHierarchy(nodes[i],true,allowDirectInline);
        });
    }

    Range_forwards = function(range)
    {
        if (Range_isForwards(range)) {
            return range;
        }
        else {
            var reverse = new Range(range.end.node,range.end.offset,
                                    range.start.node,range.start.offset);
            if (!Range_isForwards(reverse))
                throw new Error("Both range "+range+" and its reverse are not forwards");
            return reverse;
        }
    }

    Range_detail = function(range)
    {
        if (!Range_isForwards(range)) {
            var reverse = new Range(range.end.node,range.end.offset,
                                    range.start.node,range.start.offset);
            if (!Range_isForwards(reverse))
                throw new Error("Both range "+range+" and its reverse are not forwards");
            return Range_detail(reverse);
        }

        var detail = new Object();
        var start = range.start;
        var end = range.end;

        // Start location
        if (start.node.nodeType == Node.ELEMENT_NODE) {
            detail.startParent = start.node;
            detail.startChild = start.node.childNodes[start.offset];
        }
        else {
            detail.startParent = start.node.parentNode;
            detail.startChild = start.node;
        }

        // End location
        if (end.node.nodeType == Node.ELEMENT_NODE) {
            detail.endParent = end.node;
            detail.endChild = end.node.childNodes[end.offset];
        }
        else if (end.offset == 0) {
            detail.endParent = end.node.parentNode;
            detail.endChild = end.node;
        }
        else {
            detail.endParent = end.node.parentNode;
            detail.endChild = end.node.nextSibling;
        }

        // Common ancestor
        var startP = detail.startParent;
        var startC = detail.startChild;
        while (startP != null) {
            var endP = detail.endParent;
            var endC = detail.endChild
            while (endP != null) {
                if (startP == endP) {
                    detail.commonAncestor = startP;
                    detail.startAncestor = startC;
                    detail.endAncestor = endC;
                    // Found it
                    return detail;
                }
                endC = endP;
                endP = endP.parentNode;
            }
            startC = startP;
            startP = startP.parentNode;
        }
        throw new Error("Start and end of range have no common ancestor");
    }

    Range_getOutermostNodes = function(range,atLeastOne,info)
    {
        var beforeNodes = new Array();
        var middleNodes = new Array();
        var afterNodes = new Array();

        if (info != null) {
            info.beginning = beforeNodes;
            info.middle = middleNodes;
            info.end = afterNodes;
        }

        if (Range_isEmpty(range))
            return atLeastOne ? [Range_singleNode(range)] : [];

        // Note: start and end are *points* - they are always *in between* nodes or characters, never
        // *at* a node or character.
        // Everything after the end point is excluded from the selection
        // Everything after the start point, but before the end point, is included in the selection

        // We use (parent,child) pairs so that we have a way to represent a point that comes after all
        // the child nodes in a container - in which case the child is null. The parent, however, is
        // always non-null;

        var detail = Range_detail(range);
        if (detail.commonAncestor == null)
            return atLeastOne ? [Range_singleNode(range)] : [];
        var startParent = detail.startParent;
        var startChild = detail.startChild;
        var endParent = detail.endParent;
        var endChild = detail.endChild;
        var commonParent = detail.commonAncestor;
        var startAncestor = detail.startAncestor;
        var endAncestor = detail.endAncestor;

        // Add start nodes
        var topParent = startParent;
        var topChild = startChild;
        while (topParent != commonParent) {
            if (topChild != null)
                beforeNodes.push(topChild);

            while (((topChild == null) || (topChild.nextSibling == null)) &&
                   (topParent != commonParent)) {
                topChild = topParent;
                topParent = topParent.parentNode;
            }
            if (topParent != commonParent)
                topChild = topChild.nextSibling;
        }

        // Add middle nodes
        if (startAncestor != endAncestor) {
            var c = startAncestor;
            if ((c != null) && (c != startChild))
                c = c.nextSibling;
            for (; c != endAncestor; c = c.nextSibling)
                middleNodes.push(c);
        }

        // Add end nodes
        var bottomParent = endParent;
        var bottomChild = endChild;
        while (true) {

            while ((getPreviousSibling(bottomParent,bottomChild) == null) &&
                   (bottomParent != commonParent)) {
                bottomChild = bottomParent;
                bottomParent = bottomParent.parentNode;
            }
            if (bottomParent != commonParent)
                bottomChild = getPreviousSibling(bottomParent,bottomChild);

            if (bottomParent == commonParent)
                break;

            afterNodes.push(bottomChild);
        }
        afterNodes = afterNodes.reverse();

        var result = new Array();

        Array.prototype.push.apply(result,beforeNodes);
        Array.prototype.push.apply(result,middleNodes);
        Array.prototype.push.apply(result,afterNodes);

        if (result.length == 0)
            return atLeastOne ? [Range_singleNode(range)] : [];
        else
            return result;

        function getPreviousSibling(parent,child)
        {
            if (child != null)
                return child.previousSibling;
            else if (parent.lastChild != null)
                return parent.lastChild;
            else
                return null;
        }

        function isAncestorLocation(ancestorParent,ancestorChild,
                                    descendantParent,descendantChild)
        {
            while ((descendantParent != null) &&
                   ((descendantParent != ancestorParent) || (descendantChild != ancestorChild))) {
                descendantChild = descendantParent;
                descendantParent = descendantParent.parentNode;
            }

            return ((descendantParent == ancestorParent) &&
                    (descendantChild == ancestorChild));
        }
    }

    Range_getClientRects = function(range)
    {
        var nodes = Range_getOutermostNodes(range,true);

        // WebKit in iOS 5.0 and 5.1 has a bug where if the selection spans multiple paragraphs,
        // the complete rect for paragraphs other than the first is returned, instead of just the
        // portions of it that are actually in the range. To get around this problem, we go through
        // each text node individually and collect all the rects.
        var result = new Array();
        var doc = range.start.node.ownerDocument;
        var domRange = doc.createRange();
        for (var nodeIndex = 0; nodeIndex < nodes.length; nodeIndex++) {
            var node = nodes[nodeIndex];
            if (node.nodeType == Node.TEXT_NODE) {
                var startOffset = (node == range.start.node) ? range.start.offset : 0;
                var endOffset = (node == range.end.node) ? range.end.offset : node.nodeValue.length;
                domRange.setStart(node,startOffset);
                domRange.setEnd(node,endOffset);
                var rects = domRange.getClientRects();
                for (var rectIndex = 0; rectIndex < rects.length; rectIndex++) {
                    var rect = rects[rectIndex];
                    if (Main_clientRectsBug) {
                        // Apple Bug ID 14682166 - getClientRects() returns coordinates relative
                        // to top of document, when it should instead return coordinates relative
                        // to the current client view (that is, taking into account scroll offsets)
                        result.push({ left: rect.left - window.scrollX,
                                      right: rect.right - window.scrollX,
                                      top: rect.top - window.scrollY,
                                      bottom: rect.bottom - window.scrollY,
                                      width: rect.width,
                                      height: rect.height });
                    }
                    else {
                        result.push(rect);
                    }
                }
            }
            else if (node.nodeType == Node.ELEMENT_NODE) {
                result.push(node.getBoundingClientRect());
            }
        }
        return result;
    }

    Range_cloneContents = function(range)
    {
        var nodeSet = new NodeSet();
        var ancestorSet = new NodeSet();
        var detail = Range_detail(range);
        var outermost = Range_getOutermostNodes(range);

        var haveContent = false;
        for (var i = 0; i < outermost.length; i++) {
            if (!isWhitespaceTextNode(outermost[i]))
                haveContent = true;
            nodeSet.add(outermost[i]);
            for (var node = outermost[i]; node != null; node = node.parentNode)
                ancestorSet.add(node);
        }

        if (!haveContent)
            return new Array();

        var clone = recurse(detail.commonAncestor);

        var ancestor = detail.commonAncestor;
        while (isInlineNode(ancestor)) {
            var ancestorClone = DOM_cloneNode(ancestor.parentNode,false);
            DOM_appendChild(ancestorClone,clone);
            ancestor = ancestor.parentNode;
            clone = ancestorClone;
        }

        var childArray = new Array();
        switch (clone._type) {
        case HTML_UL:
        case HTML_OL:
            childArray.push(clone);
            break;
        default:
            for (var child = clone.firstChild; child != null; child = child.nextSibling)
                childArray.push(child);
            Formatting_pushDownInlineProperties(childArray);
            break;
        }

        return childArray;

        function recurse(parent)
        {
            var clone = DOM_cloneNode(parent,false);
            for (var child = parent.firstChild; child != null; child = child.nextSibling) {
                if (nodeSet.contains(child)) {
                    if ((child.nodeType == Node.TEXT_NODE) &&
                        (child == range.start.node) &&
                        (child == range.end.node)) {
                        var substring = child.nodeValue.substring(range.start.offset,
                                                                  range.end.offset);
                        DOM_appendChild(clone,DOM_createTextNode(document,substring));
                    }
                    else if ((child.nodeType == Node.TEXT_NODE) &&
                             (child == range.start.node)) {
                        var substring = child.nodeValue.substring(range.start.offset);
                        DOM_appendChild(clone,DOM_createTextNode(document,substring));
                    }
                    else if ((child.nodeType == Node.TEXT_NODE) &&
                             (child == range.end.node)) {
                        var substring = child.nodeValue.substring(0,range.end.offset);
                        DOM_appendChild(clone,DOM_createTextNode(document,substring));
                    }
                    else {
                        DOM_appendChild(clone,DOM_cloneNode(child,true));
                    }
                }
                else if (ancestorSet.contains(child)) {
                    DOM_appendChild(clone,recurse(child));
                }
            }
            return clone;
        }
    }

    Range_hasContent = function(range)
    {
        var outermost = Range_getOutermostNodes(range);
        for (var i = 0; i < outermost.length; i++) {
            var node = outermost[i];
            if (node.nodeType == Node.TEXT_NODE) {
                var value = node.nodeValue;
                if ((node == range.start.node) && (node == range.end.node)) {
                    if (!isWhitespaceString(value.substring(range.start.offset,range.end.offset)))
                        return true;
                }
                else if (node == range.start.node) {
                    if (!isWhitespaceString(value.substring(range.start.offset)))
                        return true;
                }
                else if (node == range.end.node) {
                    if (!isWhitespaceString(value.substring(0,range.end.offset)))
                        return true;
                }
                else {
                    if (!isWhitespaceString(value))
                        return true;
                }
            }
            else if (node.nodeType == Node.ELEMENT_NODE) {
                if (nodeHasContent(node))
                    return true;
            }
        }
        return false;
    }

    Range_getText = function(range)
    {
        range = Range_forwards(range);

        var start = range.start;
        var end = range.end;

        var startNode = start.node;
        var startOffset = start.offset;

        if (start.node.nodeType == Node.ELEMENT_NODE) {
            if ((start.node.offset == start.node.childNodes.length) &&
                (start.node.offset > 0))
                startNode = nextNodeAfter(start.node);
            else
                startNode = start.node.childNodes[start.offset];
            startOffset = 0;
        }

        var endNode = end.node;
        var endOffset = end.offset;

        if (end.node.nodeType == Node.ELEMENT_NODE) {
            if ((end.node.offset == end.node.childNodes.length) &&
                (end.node.offset > 0))
                endNode = nextNodeAfter(end.node);
            else
                endNode = end.node.childNodes[end.offset];
            endOffset = 0;
        }

        if ((startNode == null) || (endNode == null))
            return "";

        var components = new Array();
        var node = startNode;
        var significantParagraph = true;
        while (true) {
            if (node == null)
                throw new Error("Cannot find end node");

            if (node.nodeType == Node.TEXT_NODE) {

                if (!significantParagraph && !isWhitespaceString(node.nodeValue)) {
                    significantParagraph = true;
                    components.push("\n");
                }

                if (significantParagraph) {
                    var str;
                    if ((node == startNode) && (node == endNode))
                        str = node.nodeValue.substring(startOffset,endOffset);
                    else if (node == startNode)
                        str = node.nodeValue.substring(startOffset);
                    else if (node == endNode)
                        str = node.nodeValue.substring(0,endOffset);
                    else
                        str = node.nodeValue;
                    str = str.replace(/\s+/g," ");
                    components.push(str);
                }
            }

            if (node == endNode)
                break;


            var next = nextNode(node,entering,exiting);
            node = next;
        }
        return components.join("");

        function entering(n)
        {
            if (isParagraphNode(n)) {
                significantParagraph = true;
                components.push("\n");
            }
        }

        function exiting(n)
        {
            if (isParagraphNode(n))
                significantParagraph = false;
        }
    }

})();
