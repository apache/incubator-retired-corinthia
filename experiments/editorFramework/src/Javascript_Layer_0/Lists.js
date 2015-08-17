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

var Lists_increaseIndent;
var Lists_decreaseIndent;
var Lists_clearList;
var Lists_setUnorderedList;
var Lists_setOrderedList;

(function() {

    // private
    function findLIElements(range)
    {
        var listItems = new Array();

        var node = range.start.node;
        while (node != null) {

            addListItems(listItems,node);

            if (node == range.end.node)
                break;

            node = nextNode(node);
        }
        return listItems;

        function addListItems(array,node)
        {
            if (node == null)
                return;

            if (node._type == HTML_LI) {
                if (!arrayContains(array,node))
                    array.push(node);
                return;
            }

            if (!isWhitespaceTextNode(node))
                addListItems(array,node.parentNode);
        }
    }

    // public
    Lists_increaseIndent = function()
    {
        Selection_preferElementPositions();
        Selection_preserveWhileExecuting(function() {
            var range = Selection_get();
            if (range == null)
                return null;

            // Determine the set of LI nodes that are part of the selection
            // Note that these could be spread out all over the place, e.g. in different lists,
            // some in table cells etc
            var listItems = findLIElements(range);

            // For each LI node that is not the first in the list, move it to the child list of
            // its previous sibling (creating the child list if necessary)

            for (var i = 0; i < listItems.length; i++) {
                var li = listItems[i];
                var prevLi = li.previousSibling;
                while ((prevLi != null) && (prevLi._type != HTML_LI))
                    prevLi = prevLi.previousSibling;
                // We can only increase the indentation of the current list item C if there is
                // another list item P immediately preceding C. In this case, C becomes a child of
                // another list L, where L is inside P. L may already exist, or we may need to
                // create it.
                if (prevLi != null) {
                    var prevList = lastDescendentList(prevLi);
                    var childList = firstDescendentList(li);
                    var childListContainer = null;
                    if (childList != null) {
                        // childList may be contained inside one or more wrapper elements, in which
                        // case we set childListContainer to point to the wrapper element that is a
                        // child of li. Otherwise childListContainer will just be childList.
                        childListContainer = childList;
                        while (childListContainer.parentNode != li)
                            childListContainer = childListContainer.parentNode;
                    }

                    if (prevList != null) {
                        DOM_appendChild(prevList,li);
                        if (childList != null) {
                            while (childList.firstChild != null)
                                DOM_appendChild(prevList,childList.firstChild);
                            DOM_deleteNode(childListContainer);
                            // alert("Case 1: prevList and childList");
                        }
                        else {
                            // alert("Case 2: prevList and no childList");
                        }
                    }
                    else {
                        var newList;
                        if (childList != null) {
                            // alert("Case 3: no prevList but childList");
                            newList = childList;
                            DOM_appendChild(prevLi,childListContainer);
                        }
                        else {
                            // alert("Case 4: no prevList and no childList");
                            if (li.parentNode._type == HTML_UL)
                                newList = DOM_createElement(document,"UL");
                            else
                                newList = DOM_createElement(document,"OL");
                            DOM_appendChild(prevLi,newList);
                        }
                        DOM_insertBefore(newList,li,newList.firstChild);
                    }
                }
            }
        });

        function firstDescendentList(node)
        {
            while (true) {
                var node = firstChildElement(node);
                if (node == null)
                    return null;
                switch (node._type) {
                case HTML_UL:
                case HTML_OL:
                    return node;
                }
            }
        }

        function lastDescendentList(node)
        {
            while (true) {
                var node = lastChildElement(node);
                if (node == null)
                    return null;
                switch (node._type) {
                case HTML_UL:
                case HTML_OL:
                    return node;
                }
            }
        }
    }

    // public
    Lists_decreaseIndent = function()
    {
        Selection_preferElementPositions();
        Selection_preserveWhileExecuting(function() {
            var range = Selection_get();
            if (range == null)
                return null;

            // Determine the set of LI nodes that are part of the selection
            // Note that these could be spread out all over the place, e.g. in different lists,
            // some in table cells etc
            var listItems = findLIElements(range);

            // Remove from consideration any list items that have an ancestor that is going to
            // be moved
            var i = 0;
            var changed;
            while (i < listItems.length) {
                var node = listItems[i];

                var ancestorToBeRemoved = false;
                for (var ancestor = node.parentNode;
                     ancestor != null;
                     ancestor = ancestor.parentNode) {
                    if (arrayContains(listItems,ancestor))
                        ancestorToBeRemoved = true;
                }

                if (ancestorToBeRemoved)
                    listItems.splice(i,1);
                else
                    i++;
            }

            function haveContentAfter(node)
            {
                for (node = node.nextSibling; node != null; node = node.nextSibling) {
                    if (nodeHasContent(node))
                        return true;
                }
                return false;
            }

            // For LI nodes that are in a top-level list, change them to regular paragraphs
            // For LI nodes that are part of a nested list, move them to the parent (this requires
            // splitting the child list in two)
            for (var i = 0; i < listItems.length; i++) {
                var liNode = listItems[i];
                var listNode = liNode.parentNode;
                var containerChild = findContainerChild(listNode);

                if (haveContentAfter(liNode)) {
                    var secondHalf;
                    if (listNode._type == HTML_UL)
                        secondHalf = DOM_createElement(document,"UL");
                    else
                        secondHalf = DOM_createElement(document,"OL");

                    DOM_appendChild(liNode,secondHalf);

                    var following = liNode.nextSibling;
                    while (following != null) {
                        var next = following.nextSibling;
                        DOM_appendChild(secondHalf,following);
                        following = next;
                    }
                }

                DOM_insertBefore(containerChild.parentNode,liNode,containerChild.nextSibling);
                if (!isListNode(liNode.parentNode)) {
                    Hierarchy_avoidInlineChildren(liNode);
                    DOM_removeNodeButKeepChildren(liNode);
                }

                if (!nodeHasContent(listNode))
                    DOM_deleteNode(listNode);
            }
        });

        function findContainerChild(node)
        {
            while (node.parentNode != null) {
                if (isContainerNode(node.parentNode) && (node.parentNode._type != HTML_LI))
                    return node;
                node = node.parentNode;
            }
        }
    }

    // private
    function getListOperationNodes(range)
    {
        var detail = Range_detail(range);
        var dca = detail.commonAncestor;
        var ds = detail.startAncestor;
        var de = detail.endAncestor;

        while (isInlineNode(dca)) {
            ds = dca;
            de = dca;
            dca = dca.parentNode;
        }

        var nodes = new Array();
        var nodeSet = new NodeSet();

        if (dca._type == HTML_LI)
            return [dca];

        // If, after moving up the tree until dca is a container node, a single node is selected,
        // check if it is wholly contained within a single list item. If so, select just that
        // list item.
        var isStartLI = ((ds != null) && (ds._type == HTML_LI));
        var isEndLI = ((de != null) && (de._type == HTML_LI));
        if (!isStartLI && !isEndLI) {
            for (var ancestor = dca; ancestor.parentNode != null; ancestor = ancestor.parentNode) {
                if (ancestor.parentNode._type == HTML_LI) {
                    var firstElement = true;

                    for (var p = ancestor.previousSibling; p != null; p = p.previousSibling) {
                        if (p.nodeType == Node.ELEMENT_NODE) {
                            firstElement = false;
                            break;
                        }
                    }

                    if (firstElement)
                        return [ancestor.parentNode];
                }
            }
        }

        var end = (de == null) ? null : de.nextSibling;

        for (var child = ds; child != end; child = child.nextSibling) {
            switch (child._type) {
            case HTML_UL:
            case HTML_OL:
                for (var gc = child.firstChild; gc != null; gc = gc.nextSibling) {
                    if (!isWhitespaceTextNode(gc))
                        addNode(gc);
                }
                break;
            default:
                if ((child._type == HTML_DIV) &&
                     child.getAttribute("class") == Keys.SELECTION_HIGHLIGHT) {
                    // skip
                }
                else if (!isWhitespaceTextNode(child)) {
                    addNode(child);
                }
                break;
            }
        }
        if ((nodes.length == 0) && isParagraphNode(dca))
            nodes.push(dca);
        return nodes;

        function addNode(node)
        {
            while (isInlineNode(node) && node.parentNode != document.body)
                node = node.parentNode;
            if (!nodeSet.contains(node)) {
                nodeSet.add(node);
                nodes.push(node);
            }
        }
    }

    // public
    Lists_clearList = function()
    {
        Selection_preferElementPositions();
        Selection_preserveWhileExecuting(function() {
            var range = Selection_get();
            if (range == null)
                return;
            Range_ensureInlineNodesInParagraph(range);

            var nodes = getListOperationNodes(range);

            for (var i = 0; i < nodes.length; i++) {
                var node = nodes[i];
                if (node._type == HTML_LI) {
                    var li = node;
                    var list = li.parentNode;
                    var insertionPoint = null;

                    DOM_removeAdjacentWhitespace(li);

                    if (li.previousSibling == null) {
                        insertionPoint = list;
                    }
                    else if (li.nextSibling == null) {
                        insertionPoint = list.nextSibling;
                    }
                    else {
                        var secondList = DOM_shallowCopyElement(list);
                        DOM_insertBefore(list.parentNode,secondList,list.nextSibling);
                        while (li.nextSibling != null) {
                            DOM_appendChild(secondList,li.nextSibling);
                            DOM_removeAdjacentWhitespace(li);
                        }

                        insertionPoint = secondList;
                    }

                    var parent = null;
                    var child = li.firstChild;
                    while (child != null) {
                        var next = child.nextSibling;
                        if (isInlineNode(child) && !isWhitespaceTextNode(child)) {
                            child = Hierarchy_wrapInlineNodesInParagraph(child);
                            next = child.nextSibling;
                        }
                        child = next;
                    }
                    DOM_insertBefore(list.parentNode,li,insertionPoint);
                    DOM_removeNodeButKeepChildren(li);

                    if (list.firstChild == null)
                        DOM_deleteNode(list);
                }
            }
        });

        var range = Selection_get();
        if (range == null)
            return;
        if (Range_isEmpty(range) &&
            (range.start.node.nodeType == Node.ELEMENT_NODE) &&
            (isContainerNode(range.start.node))) {

            var p = DOM_createElement(document,"P");

            var next = range.start.node.childNodes[range.start.offset+1];
            DOM_insertBefore(range.start.node,p,next);

            Cursor_updateBRAtEndOfParagraph(p);
            Selection_set(p,0,p,0);
        }
    }

    // private
    function setList(type)
    {
        var range = Selection_get();
        if (range == null)
            return;

        var nodes = getListOperationNodes(range);

        if (nodes.length == 0) {
            var text;
            if (range.start.node.nodeType == Node.TEXT_NODE) {
                text = range.start.node;
            }
            else if (range.start.node.nodeType == Node.ELEMENT_NODE) {
                text = DOM_createTextNode(document,"");
                DOM_insertBefore(range.start.node,
                                 text,
                                 range.start.node[range.start.offset+1]);
            }
            nodes = [text];

            var offset = DOM_nodeOffset(text);
            Selection_set(text,0,text,0);
            range = Selection_get();
        }

        Range_trackWhileExecuting(range,function () {
            // Set list to UL or OL

            for (var i = 0; i < nodes.length; i++) {
                var node = nodes[i];
                var next;
                var prev;
                var li = null;
                var oldList = null;
                var listInsertionPoint;

                if ((node._type == HTML_LI) && (node.parentNode._type == type)) {
                    // Already in the correct type of list; don't need to do anything
                    continue;
                }

                if (node._type == HTML_LI) {
                    li = node;
                    var list = li.parentNode;

                    DOM_removeAdjacentWhitespace(list);
                    prev = list.previousSibling;
                    next = list.nextSibling;


                    DOM_removeAdjacentWhitespace(li);

                    if (li.previousSibling == null) {
                        listInsertionPoint = list;
                        next = null;
                    }
                    else if (li.nextSibling == null) {
                        listInsertionPoint = list.nextSibling;
                        prev = null;
                    }
                    else {
                        var secondList = DOM_shallowCopyElement(list);
                        DOM_insertBefore(list.parentNode,secondList,list.nextSibling);
                        while (li.nextSibling != null) {
                            DOM_insertBefore(secondList,li.nextSibling,null);
                            DOM_removeAdjacentWhitespace(li);
                        }

                        listInsertionPoint = secondList;

                        prev = null;
                        next = null;
                    }

                    node = list;
                    oldList = list;
                }
                else {
                    DOM_removeAdjacentWhitespace(node);
                    prev = node.previousSibling;
                    next = node.nextSibling;
                    listInsertionPoint = node;
                }

                var list;
                var itemInsertionPoint;

                if ((prev != null) && (prev._type == type)) {
                    list = prev;
                    itemInsertionPoint = null;
                }
                else if ((next != null) && (next._type == type)) {
                    list = next;
                    itemInsertionPoint = list.firstChild;
                }
                else {
                    if (type == HTML_UL)
                        list = DOM_createElement(document,"UL");
                    else
                        list = DOM_createElement(document,"OL");
                    DOM_insertBefore(node.parentNode,list,listInsertionPoint);
                    itemInsertionPoint = null;
                }

                if (li != null) {
                    DOM_insertBefore(list,li,itemInsertionPoint);
                }
                else {
                    var li = DOM_createElement(document,"LI");
                    DOM_insertBefore(list,li,itemInsertionPoint);
                    DOM_insertBefore(li,node,null);
                }


                if ((oldList != null) && (oldList.firstChild == null))
                    DOM_deleteNode(oldList);

                // Merge with adjacent list
                DOM_removeAdjacentWhitespace(list);
                if ((list.nextSibling != null) && (list.nextSibling._type == type)) {
                    var followingList = list.nextSibling;
                    while (followingList.firstChild != null) {
                        if (isWhitespaceTextNode(followingList.firstChild))
                            DOM_deleteNode(followingList.firstChild);
                        else
                            DOM_insertBefore(list,followingList.firstChild,null);
                    }
                    DOM_deleteNode(followingList);
                }
            }
        });
        Range_ensureValidHierarchy(range);
        Selection_set(range.start.node,range.start.offset,range.end.node,range.end.offset);
    }

    // public
    Lists_setUnorderedList = function()
    {
        setList(HTML_UL);
    }

    // public
    Lists_setOrderedList = function()
    {
        setList(HTML_OL);
    }

})();
