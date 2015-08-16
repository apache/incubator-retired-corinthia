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

function testHarnessSetup()
{
    DOM_assignNodeIds(document);

    var start;
    var track;
    var end;


    UndoManager_disableWhileExecuting(function() {
        start = extractPositionFromCharacter("[");
        track = (start == null) ? [] : [start];
        Position_trackWhileExecuting(track,function() {
            end = extractPositionFromCharacter("]");
        });
    });

    if ((start != null) && (end == null))
        throw new Error("Start of selection specified, but not end");
    if ((start == null) && (end != null))
        throw new Error("End of selection specified, but not start");

    if ((start != null) && (end != null)) {
        var range = new Range(start.node,start.offset,end.node,end.offset);

        UndoManager_disableWhileExecuting(function() {
            Range_trackWhileExecuting(range,function() {
                positionMergeWithNeighbours(start);
                positionMergeWithNeighbours(end);
            });
        });

        range.start = Position_preferTextPosition(range.start);
        range.end = Position_preferTextPosition(range.end);

        Selection_set(range.start.node,range.start.offset,range.end.node,range.end.offset);
    }

    return;

    function positionMergeWithNeighbours(pos)
    {
        var node = pos.node;
        var offset = pos.offset;
        if ((node.nodeType == Node.ELEMENT_NODE) && (offset < node.childNodes.length))
            Formatting_mergeWithNeighbours(node.childNodes[offset],Formatting_MERGEABLE_INLINE);
        else if ((node.nodeType == Node.ELEMENT_NODE) && (node.lastChild != null))
            Formatting_mergeWithNeighbours(node.lastChild,Formatting_MERGEABLE_INLINE);
        else
            Formatting_mergeWithNeighbours(node,Formatting_MERGEABLE_INLINE);
    }

    function extractPositionFromCharacter(c)
    {
        return recurse(document.body);

        function recurse(node)
        {
            if (node.nodeType == Node.TEXT_NODE) {
                var index = node.nodeValue.indexOf(c);
                if (index >= 0) {
                    var offsetInParent = DOM_nodeOffset(node);
                    if (index == 0) {
                        node.nodeValue = node.nodeValue.substring(1);
                        return new Position(node.parentNode,offsetInParent);
                    }
                    else if (index == node.nodeValue.length - 1) {
                        node.nodeValue = node.nodeValue.substring(0,node.nodeValue.length-1);
                        return new Position(node.parentNode,offsetInParent+1);
                    }
                    else {
                        var rest = node.nodeValue.substring(index+1);
                        node.nodeValue = node.nodeValue.substring(0,index);
                        var restNode = DOM_createTextNode(document,rest);
                        DOM_insertBefore(node.parentNode,restNode,node.nextSibling);
                        return new Position(node.parentNode,offsetInParent+1);
                    }
                }
            }
            else {
                for (var child = node.firstChild; child != null; child = child.nextSibling) {
                    var result = recurse(child);
                    if (result != null)
                        return result;
                }
            }
            return null;
        }
    }
}

function insertAtPosition(position,node)
{
    if (position.node.nodeType == Node.ELEMENT_NODE) {
        if (position.offset == position.node.childNodes.length)
            DOM_appendChild(position.node,node);
        else
            DOM_insertBefore(position.node,node,position.node.childNodes[position.offset]);
    }
    else if (position.node.nodeType == Node.TEXT_NODE) {
        var newText = DOM_createTextNode(document,position.node.nodeValue.slice(position.offset));
        position.node.nodeValue = position.node.nodeValue.slice(0,position.offset);
        DOM_insertBefore(position.node.parentNode,newText,position.node.nextSibling);
        DOM_insertBefore(position.node.parentNode,node,position.node.nextSibling);
    }
}

function insertTextAtPosition(position,str)
{
    if (position.node.nodeType == Node.ELEMENT_NODE) {
        var before = position.node.childNodes[position.offset-1];
        var after = position.node.childNodes[position.offset];
        if ((after != null) && (after.nodeType == Node.TEXT_NODE))
            position = new Position(after,0);
        else if ((before != null) && (before.nodeType == Node.TEXT_NODE))
            position = new Position(before,before.nodeValue.length);
    }
    if (position.node.nodeType == Node.ELEMENT_NODE) {
        insertAtPosition(position,DOM_createTextNode(document,str));
    }
    else if (position.node.nodeType == Node.TEXT_NODE) {
        position.node.nodeValue = position.node.nodeValue.slice(0,position.offset) + str +
                                  position.node.nodeValue.slice(position.offset);
    }
}

function showRangeAsBrackets(range)
{
    if (Range_isEmpty(range)) {
        insertTextAtPosition(range.end,"[]",true);
    }
    else {
        insertTextAtPosition(range.end,"]",true);
        insertTextAtPosition(range.start,"[",true);
    }
}

function showSelection()
{
    var range = Selection_get();
    if (range != null) {
        Range_assertValid(range,"Selection");
        showRangeAsBrackets(range);
    }
}

function removeIds()
{
    recurse(document.body);

    function recurse(node)
    {
        if (node.nodeType == Node.ELEMENT_NODE) {
            DOM_removeAttribute(node,"id");
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }
}

function selectNode(node)
{
    var offset = DOM_nodeOffset(node);
    Selection_set(node.parentNode,offset,node.parentNode,offset+1);
}

function removeWhitespaceAndCommentNodes(root)
{
    Selection_preserveWhileExecuting(function() {
        recurse(root);
    });

    function recurse(node)
    {
        if (isWhitespaceTextNode(node) || (node.nodeType == Node.COMMENT_NODE)) {
            DOM_deleteNode(node);
        }
        else {
            var next;
            for (var child = node.firstChild; child != null; child = next) {
                next = child.nextSibling;
                recurse(child);
            }
        }
    }
}

// selectionWrapElement() and selectionUnwrapElement() used to be in formatting.js but have
// now been made obselete by the addition of applyFormattingChanges(). However there are still
// a few tests which use them.
function selectionWrapElement(elementName)
{
    if (elementName == "B")
        Formatting_applyFormattingChanges(null,{"font-weight": "bold"});
    else if (elementName == "I")
        Formatting_applyFormattingChanges(null,{"font-style": "italic"});
    else if (elementName == "U")
        Formatting_applyFormattingChanges(null,{"text-decoration": "underline"});
}

function selectionUnwrapElement(elementName)
{
    if (elementName == "B")
        Formatting_applyFormattingChanges(null,{"font-weight": null});
    else if (elementName == "I")
        Formatting_applyFormattingChanges(null,{"font-style": null});
    else if (elementName == "U")
        Formatting_applyFormattingChanges(null,{"text-decoration": null});
}

function showEmptyTextNodes()
{
    recurse(document);

    function recurse(node)
    {
        if ((node.nodeType == Node.TEXT_NODE) && (node.nodeValue.length == 0))
            node.nodeValue = "*";
        for (var child = node.firstChild; child != null; child = child.nextSibling)
            recurse(child);
    }
}

function showClipboard(clipboard)
{
    var html = clipboard["text/html"];
    var text = clipboard["text/plain"];

    if ((html.length == 0) || (html.charAt(html.length-1) != "\n"))
        html += "\n";
    if ((text.length == 0) || (text.charAt(text.length-1) != "\n"))
        text += "\n";

    // Chrome and Safari behave differently when generating style attribute values for innerHTML.
    // Safari adds a space after the last property definition, while chrome doesn't. For consistent
    // results, we add the space if it is not already there (this was the original behaviour
    // of Chrome but it changed).
    html = html.replace(/(style="[^"]+;)"/g,"$1 \"");

    return "text/html\n"+
           "---------\n"+
           "\n"+
           html+
           "\n"+
           "text/plain\n"+
           "----------\n"+
           "\n"+
           text;
}

function setNumbering(enabled)
{
    if (enabled)
        setupOutlineNumbering();

    recurse(document.body,enabled);
    PostponedActions_perform();

    function recurse(node,enabled)
    {
        switch (node._type) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_FIGURE:
        case HTML_TABLE:
            if (!isInTOC(node)) {
                Outline_setNumbered(node.getAttribute("id"),enabled);
                return;
            }
            break;
        }

        for (var child = node.firstChild; child != null; child = child.nextSibling)
            recurse(child,enabled);
    }
}

function readXML(filename)
{
    var req = new XMLHttpRequest();
    req.open("GET",filename,false);
    req.send();
    var xml = req.responseXML;
    if (xml == null)
        return null;
    DOM_assignNodeIds(xml.documentElement);
    return xml;
}

function findTextMatchingRecursive(node,re)
{
    if (node.nodeType == Node.TEXT_NODE) {
        if (node.nodeValue.match(re))
            return node;
        else
            return null;
    }
    else {
        for (var child = node.firstChild; child != null; child = child.nextSibling) {
            var result = findTextMatchingRecursive(child,re);
            if (result != null)
                return result;
        }
        return null;
    }
}

function setupOutlineNumbering()
{
    Styles_setCSSText("",
    {
        "h1": {
            "counter-reset": "h2 h3 h4 h5 h6",
            "counter-increment": "h1"
        },
        "h2": {
            "counter-reset": "h3 h4 h5 h6",
            "counter-increment": "h2"
        },
        "h3": {
            "counter-reset": "h4 h5 h6",
            "counter-increment": "h3"
        },
        "h4": {
            "counter-reset": "h5 h6",
            "counter-increment": "h4"
        },
        "h5": {
            "counter-reset": "h6",
            "counter-increment": "h5"
        },
        "h6": {
            "counter-increment": "h6"
        },
        "h1::before": {
            "content": "counter(h1) ' '"
        },
        "h2::before": {
            "content": "counter(h1) '.' counter(h2) ' '"
        },
        "h3::before": {
            "content": "counter(h1) '.' counter(h2) '.' counter(h3) ' '"
        },
        "h4::before": {
            "content": "counter(h1) '.' counter(h2) '.' counter(h3) '.' counter(h4) ' '"
        },
        "h5::before": {
            "content": "counter(h1) '.' counter(h2) '.' counter(h3) '.' counter(h4) '.' counter(h5) ' '"
        },
        "h6::before": {
            "content": "counter(h1) '.' counter(h2) '.' counter(h3) '.' counter(h4) '.' counter(h5) '.' counter(h6) ' '"
        },
    });
}

function prependTableOfContents()
{
    var nav = DOM_createElement(document,"NAV");
    DOM_setAttribute(nav,"class","tableofcontents");
    DOM_insertBefore(document.body,nav,document.body.firstChild);
    PostponedActions_perform();
}

function simplifyTOCs()
{
    recurse(document.body);

    function recurse(node)
    {
        if ((node._type == HTML_NAV) &&
            ((DOM_getAttribute(node,"class") == "tableofcontents") ||
             (DOM_getAttribute(node,"class") == "listoffigures") ||
             (DOM_getAttribute(node,"class") == "listoftables"))) {
            mergeAdjacentTextNodes(node);
        }
        else {
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }

    function mergeAdjacentTextNodes(node)
    {
        var child = node.firstChild;
        while (child != null) {
            if ((child.nodeType == Node.TEXT_NODE) &&
                (child.nextSibling != null) &&
                (child.nextSibling.nodeType == Node.TEXT_NODE)) {
                DOM_insertCharacters(child,child.nodeValue.length,child.nextSibling.nodeValue);
                DOM_deleteNode(child.nextSibling);
            }
            else {
                child = child.nextSibling;
            }
        }

        for (var child = node.firstChild; child != null; child = child.nextSibling)
            mergeAdjacentTextNodes(child);
    }
}

function showNonEmptyTextNodes()
{
    recurse(document.body);

    function recurse(node)
    {
        if (node.nodeType == Node.TEXT_NODE) {
            if (!isWhitespaceTextNode(node))
                node.nodeValue = "{" + node.nodeValue + "}";
        }
        else {
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }
}
