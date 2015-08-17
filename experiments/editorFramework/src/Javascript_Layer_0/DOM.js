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

// Helper functions
var DOM_assignNodeIds;

// Primitive node creation operations
var DOM_createElement;
var DOM_createElementNS;
var DOM_createTextNode;
var DOM_createComment;
var DOM_cloneNode;

// Primitive and high-level node mutation operations
var DOM_appendChild;
var DOM_insertBefore;
var DOM_deleteNode;
var DOM_setAttribute;
var DOM_setAttributeNS;
var DOM_removeAttribute;
var DOM_removeAttributeNS;
var DOM_setStyleProperties;
var DOM_insertCharacters;
var DOM_moveCharacters;
var DOM_deleteCharacters;
var DOM_setNodeValue;

// High-level DOM operations
var DOM_getAttribute;
var DOM_getAttributeNS;
var DOM_getStringAttribute;
var DOM_getStringAttributeNS;
var DOM_getStyleProperties;
var DOM_deleteAllChildren;
var DOM_shallowCopyElement;
var DOM_replaceElement;
var DOM_wrapNode;
var DOM_wrapSiblings;
var DOM_mergeWithNextSibling;
var DOM_nodesMergeable;
var DOM_replaceCharacters;
var DOM_addTrackedPosition;
var DOM_removeTrackedPosition;
var DOM_removeAdjacentWhitespace;
var DOM_documentHead;
var DOM_ensureUniqueIds;
var DOM_nodeOffset;
var DOM_maxChildOffset;
var DOM_ignoreMutationsWhileExecuting;
var DOM_getIgnoreMutations;
var DOM_addListener;
var DOM_removeListener;
var DOM_Listener;

(function() {

    var nextNodeId = 0;
    var nodeData = new Object();
    var ignoreMutations = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                            //
    //                                    DOM Helper Functions                                    //
    //                                                                                            //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    function addUndoAction()
    {
        if (window.undoSupported)
            UndoManager_addAction.apply(null,arrayCopy(arguments));
    }

    function assignNodeId(node)
    {
        if (node._nodeId != null)
            throw new Error(node+" already has id");
        node._nodeId = nextNodeId++;
        node._type = ElementTypes[node.nodeName];
        return node;
    }

    function checkNodeId(node)
    {
        if (node._nodeId == null)
            throw new Error(node.nodeName+" lacks _nodeId");
    }

    // public
    DOM_assignNodeIds = function(root)
    {
        if (root._nodeId != null)
            throw new Error(root+" already has id");
        recurse(root);
        return;

        function recurse(node) {
            node._nodeId = nextNodeId++;
            node._type = ElementTypes[node.nodeName];
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                recurse(child);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                            //
    //                                  Primitive DOM Operations                                  //
    //                                                                                            //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /*

      The following functions are considered "primitive", in that they are the core functions
      through which all manipulation of the DOM ultimately occurs. All other DOM functions call
      these, either directly or indirectly, instead of making direct method calls on node objects.
      These functions are divided into two categories: node creation and mode mutation.

      The creation functions are as follows:

      * createElement(document,elementName)
      * createElementNS(document,namespaceURI,qualifiedName)
      * createTextNode(document,data)
      * createComment(document,data)
      * cloneNode(original,deep,noIdAttr)

      The purpose of these is to ensure that a unique _nodeId value is assigned to each node object,
      which is needed for using the NodeSet and NodeMap classes. All nodes in a document must have
      this set; we use our own functions for this because DOM provides no other way of uniquely
      identifying nodes in a way that allows them to be stored in a hash table.

      The mutation functions are as follows:

      * insertBeforeInternal(parent,newChild,refChild)
      * deleteNodeInternal(node,deleteDescendantData)
      * setAttribute(element,name,value)
      * setAttributeNS(element,namespaceURI,qualifiedName,value)
      * setStyleProperties(element,properties)
      * insertCharacters(textNode,offset,characters)
      * deleteCharacters(textNode,startOffset,endOffset)
      * moveCharacters(srcTextNode,srcStartOffset,srcEndOffset,destTextNode,destOffset)
      * setNodeValue(textNode,value)

      These functions exist to allow us to record undo information. We can't use DOM mutation events
      for this purpose they're not fully supported in WebKit.

      Every time a mutation operation is performed on a node, we add an action to the undo stack
      corresponding to the inverse of that operaton, i.e. an action that undoes the operaton. It
      is absolutely critical that all changes to a DOM node go through these functions, regardless
      of whether or not the node currently resides in the tree. This ensures that the undo history
      is able to correctly revert the tree to the same state that it was in at the relevant point
      in time.

      By routing all DOM modifications through these few functions, virtually all of the other
      javascript code can be ignorant of the undo manager, provided the only state they change is
      in the DOM. Parts of the code which maintain their own state about the document, such as the
      style manager, must implement their own undo-compliant state manipulation logic.

      *** IMPORTANT ***

      Just in case it isn't already clear, you must *never* make direct calls to methods like
      appendChild() and createElement() on the node objects themselves. Doing so will result in
      subtle and probably hard-to-find bugs. As far as all javascript code for UX Write is
      concerned, consider the public functions defined in this file to be the DOM API. You can use
      check-dom-methods.sh to search for any cases where this rule has been violated.

      */

    // public
    DOM_createElement = function(document,elementName)
    {
        return assignNodeId(document.createElement(elementName)); // check-ok
    }

    // public
    DOM_createElementNS = function(document,namespaceURI,qualifiedName)
    {
        return assignNodeId(document.createElementNS(namespaceURI,qualifiedName)); // check-ok
    }

    // public
    DOM_createTextNode = function(document,data)
    {
        return assignNodeId(document.createTextNode(data)); // check-ok
    }

    // public
    DOM_createComment = function(document,data)
    {
        return assignNodeId(document.createComment(data)); // check-ok
    }

    // public
    DOM_cloneNode = function(original,deep,noIdAttr)
    {
        var clone = original.cloneNode(deep); // check-ok
        DOM_assignNodeIds(clone);
        if (noIdAttr)
            clone.removeAttribute("id"); // check-ok
        return clone;
    }

    function insertBeforeInternal(parent,newChild,refChild)
    {
        if (newChild.parentNode == null) {
            addUndoAction(deleteNodeInternal,newChild)
        }
        else {
            var oldParent = newChild.parentNode;
            var oldNext = newChild.nextSibling;
            addUndoAction(insertBeforeInternal,oldParent,newChild,oldNext);
        }

        parent.insertBefore(newChild,refChild); // check-ok
    }

    function deleteNodeInternal(node,deleteDescendantData)
    {
        checkNodeId(node);

        addUndoAction(insertBeforeInternal,node.parentNode,node,node.nextSibling);

        if (node.parentNode == null)
            throw new Error("Undo delete "+nodeString(node)+": parent is null");
        node.parentNode.removeChild(node); // check-ok

        // Delete all data associated with the node. This is not preserved across undo/redo;
        // currently the only thing we are using this data for is tracked positions, and we
        // are going to be recording undo information for the selection separately, so this is
        // not a problem.
        if (deleteDescendantData)
            deleteNodeDataRecursive(node);
        else
            deleteNodeData(node);

        return;

        function deleteNodeData(current)
        {
            delete nodeData[current._nodeId];
        }

        function deleteNodeDataRecursive(current)
        {
            deleteNodeData(current);
            for (var child = current.firstChild; child != null; child = child.nextSibling)
                deleteNodeDataRecursive(child);
        }
    }

    // public
    DOM_setAttribute = function(element,name,value)
    {
        if (element.hasAttribute(name))
            addUndoAction(DOM_setAttribute,element,name,element.getAttribute(name));
        else
            addUndoAction(DOM_setAttribute,element,name,null);

        if (value == null)
            element.removeAttribute(name); // check-ok
        else
            element.setAttribute(name,value); // check-ok
    }

    // public
    DOM_setAttributeNS = function(element,namespaceURI,qualifiedName,value)
    {
        var localName = qualifiedName.replace(/^.*:/,"");
        if (element.hasAttributeNS(namespaceURI,localName)) {
            var oldValue = element.getAttributeNS(namespaceURI,localName);
            var oldQName = element.getAttributeNodeNS(namespaceURI,localName).nodeName; // check-ok
            addUndoAction(DOM_setAttributeNS,element,namespaceURI,oldQName,oldValue)
        }
        else {
            addUndoAction(DOM_setAttributeNS,element,namespaceURI,localName,null);
        }

        if (value == null)
            element.removeAttributeNS(namespaceURI,localName); // check-ok
        else
            element.setAttributeNS(namespaceURI,qualifiedName,value); // check-ok
    }

    // public
    DOM_setStyleProperties = function(element,properties)
    {
        if (Object.getOwnPropertyNames(properties).length == 0)
            return;

        if (element.hasAttribute("style"))
            addUndoAction(DOM_setAttribute,element,"style",element.getAttribute("style"));
        else
            addUndoAction(DOM_setAttribute,element,"style",null);

        for (var name in properties)
            element.style.setProperty(name,properties[name]); // check-ok

        if (element.getAttribute("style") == "")
            element.removeAttribute("style"); // check-ok
    }

    // public
    DOM_insertCharacters = function(textNode,offset,characters)
    {
        if (textNode.nodeType != Node.TEXT_NODE)
            throw new Error("DOM_insertCharacters called on non-text node");
        if ((offset < 0) || (offset > textNode.nodeValue.length))
            throw new Error("DOM_insertCharacters called with invalid offset");
        trackedPositionsForNode(textNode).forEach(function (position) {
            if (position.offset > offset)
                position.offset += characters.length;
        });
        textNode.nodeValue = textNode.nodeValue.slice(0,offset) +
                             characters +
                             textNode.nodeValue.slice(offset);
        var startOffset = offset;
        var endOffset = offset + characters.length;
        addUndoAction(DOM_deleteCharacters,textNode,startOffset,endOffset);
    }

    // public
    DOM_deleteCharacters = function(textNode,startOffset,endOffset)
    {
        if (textNode.nodeType != Node.TEXT_NODE)
            throw new Error("DOM_deleteCharacters called on non-text node "+nodeString(textNode));
        if (endOffset == null)
            endOffset = textNode.nodeValue.length;
        if (endOffset < startOffset)
            throw new Error("DOM_deleteCharacters called with invalid start/end offset");
        trackedPositionsForNode(textNode).forEach(function (position) {
            var deleteCount = endOffset - startOffset;
            if ((position.offset > startOffset) && (position.offset < endOffset))
                position.offset = startOffset;
            else if (position.offset >= endOffset)
                position.offset -= deleteCount;
        });

        var removed = textNode.nodeValue.slice(startOffset,endOffset);
        addUndoAction(DOM_insertCharacters,textNode,startOffset,removed);

        textNode.nodeValue = textNode.nodeValue.slice(0,startOffset) +
                             textNode.nodeValue.slice(endOffset);
    }

    // public
    DOM_moveCharacters = function(srcTextNode,srcStartOffset,srcEndOffset,destTextNode,destOffset,
                                  excludeStartPos,excludeEndPos)
    {
        if (srcTextNode == destTextNode)
            throw new Error("src and dest text nodes cannot be the same");
        if (srcStartOffset > srcEndOffset)
            throw new Error("Invalid src range "+srcStartOffset+" - "+srcEndOffset);
        if (srcStartOffset < 0)
            throw new Error("srcStartOffset < 0");
        if (srcEndOffset > srcTextNode.nodeValue.length)
            throw new Error("srcEndOffset beyond end of src length");
        if (destOffset < 0)
            throw new Error("destOffset < 0");
        if (destOffset > destTextNode.nodeValue.length)
            throw new Error("destOffset beyond end of dest length");

        var length = srcEndOffset - srcStartOffset;

        addUndoAction(DOM_moveCharacters,destTextNode,destOffset,destOffset+length,
                      srcTextNode,srcStartOffset,excludeStartPos,excludeEndPos);

        trackedPositionsForNode(destTextNode).forEach(function (pos) {
            var startMatch = excludeStartPos ? (pos.offset > destOffset)
                                             : (pos.offset >= destOffset);
            if (startMatch)
                pos.offset += length;
        });
        trackedPositionsForNode(srcTextNode).forEach(function (pos) {

            var startMatch = excludeStartPos ? (pos.offset > srcStartOffset)
                                             : (pos.offset >= srcStartOffset);
            var endMatch = excludeEndPos ? (pos.offset < srcEndOffset)
                                         : (pos.offset <= srcEndOffset);

            if (startMatch && endMatch) {
                pos.node = destTextNode;
                pos.offset = destOffset + (pos.offset - srcStartOffset);
            }
            else if (pos.offset >= srcEndOffset) {
                pos.offset -= length;
            }
        });
        var extract = srcTextNode.nodeValue.substring(srcStartOffset,srcEndOffset);
        srcTextNode.nodeValue = srcTextNode.nodeValue.slice(0,srcStartOffset) +
                                srcTextNode.nodeValue.slice(srcEndOffset);
        destTextNode.nodeValue = destTextNode.nodeValue.slice(0,destOffset) +
                                 extract +
                                 destTextNode.nodeValue.slice(destOffset);
    }

    // public
    DOM_setNodeValue = function(textNode,value)
    {
        if (textNode.nodeType != Node.TEXT_NODE)
            throw new Error("DOM_setNodeValue called on non-text node");
        trackedPositionsForNode(textNode).forEach(function (position) {
            position.offset = 0;
        });
        var oldValue = textNode.nodeValue;
        addUndoAction(DOM_setNodeValue,textNode,oldValue);
        textNode.nodeValue = value;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                            //
    //                                  High-level DOM Operations                                 //
    //                                                                                            //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    function appendChildInternal(parent,newChild)
    {
        insertBeforeInternal(parent,newChild,null);
    }

    // public
    DOM_appendChild = function(node,child)
    {
        return DOM_insertBefore(node,child,null);
    }

    // public
    DOM_insertBefore = function(parent,child,nextSibling)
    {
        var newOffset;
        if (nextSibling != null)
            newOffset = DOM_nodeOffset(nextSibling);
        else
            newOffset = parent.childNodes.length;

        var oldParent = child.parentNode;
        if (oldParent != null) { // already in tree
            var oldOffset = DOM_nodeOffset(child);

            if ((oldParent == parent) && (newOffset > oldOffset))
                newOffset--;

            trackedPositionsForNode(oldParent).forEach(function (position) {
                if (position.offset > oldOffset) {
                    position.offset--;
                }
                else if (position.offset == oldOffset) {
                    position.node = parent;
                    position.offset = newOffset;
                }
            });
        }

        var result = insertBeforeInternal(parent,child,nextSibling);
        trackedPositionsForNode(parent).forEach(function (position) {
            if (position.offset > newOffset)
                position.offset++;
        });
        return result;
    }

    // public
    DOM_deleteNode = function(node)
    {
        if (node.parentNode == null) // already deleted
            return;
        adjustPositionsRecursive(node);
        deleteNodeInternal(node,true);

        function adjustPositionsRecursive(current)
        {
            for (var child = current.firstChild; child != null; child = child.nextSibling)
                adjustPositionsRecursive(child);

            trackedPositionsForNode(current.parentNode).forEach(function (position) {
                var offset = DOM_nodeOffset(current);
                if (offset < position.offset) {
                    position.offset--;
                }
            });
            trackedPositionsForNode(current).forEach(function (position) {
                var offset = DOM_nodeOffset(current);
                position.node = current.parentNode;
                position.offset = offset;
            });
        }
    }

    // public
    DOM_removeAttribute = function(element,name,value)
    {
        DOM_setAttribute(element,name,null);
    }

    // public
    DOM_removeAttributeNS = function(element,namespaceURI,localName)
    {
        DOM_setAttributeNS(element,namespaceURI,localName,null)
    }

    // public
    DOM_getAttribute = function(element,name)
    {
        if (element.hasAttribute(name))
            return element.getAttribute(name);
        else
            return null;
    }

    // public
    DOM_getAttributeNS = function(element,namespaceURI,localName)
    {
        if (element.hasAttributeNS(namespaceURI,localName))
            return element.getAttributeNS(namespaceURI,localName);
        else
            return null;
    }

    // public
    DOM_getStringAttribute = function(element,name)
    {
        var value = element.getAttribute(name);
        return (value == null) ? "" : value;
    }

    // public
    DOM_getStringAttributeNS = function(element,namespaceURI,localName)
    {
        var value = element.getAttributeNS(namespaceURI,localName);
        return (value == null) ? "" : value;
    }

    // public
    DOM_getStyleProperties = function(node)
    {
        var properties = new Object();
        if (node.nodeType == Node.ELEMENT_NODE) {
            for (var i = 0; i < node.style.length; i++) {
                var name = node.style[i];
                var value = node.style.getPropertyValue(name);
                properties[name] = value;
            }
        }
        return properties;
    }

    // public
    DOM_deleteAllChildren = function(parent)
    {
        while (parent.firstChild != null)
            DOM_deleteNode(parent.firstChild);
    }

    // public
    DOM_shallowCopyElement = function(element)
    {
        return DOM_cloneNode(element,false,true);
    }

    // public
    DOM_removeNodeButKeepChildren = function(node)
    {
        if (node.parentNode == null)
            throw new Error("Node "+nodeString(node)+" has no parent");
        var offset = DOM_nodeOffset(node);
        var childCount = node.childNodes.length;

        trackedPositionsForNode(node.parentNode).forEach(function (position) {
            if (position.offset > offset)
                position.offset += childCount-1;
        });

        trackedPositionsForNode(node).forEach(function (position) {
            position.node = node.parentNode;
            position.offset += offset;
        });

        var parent = node.parentNode;
        var nextSibling = node.nextSibling;
        deleteNodeInternal(node,false);

        while (node.firstChild != null) {
            var child = node.firstChild;
            insertBeforeInternal(parent,child,nextSibling);
        }
    }

    // public
    DOM_replaceElement = function(oldElement,newName)
    {
        var listeners = listenersForNode(oldElement);
        var newElement = DOM_createElement(document,newName);
        for (var i = 0; i < oldElement.attributes.length; i++) {
            var name = oldElement.attributes[i].nodeName; // check-ok
            var value = oldElement.getAttribute(name);
            DOM_setAttribute(newElement,name,value);
        }

        var positions = arrayCopy(trackedPositionsForNode(oldElement));
        if (positions != null) {
            for (var i = 0; i < positions.length; i++) {
                if (positions[i].node != oldElement)
                    throw new Error("replaceElement: position with wrong node");
                positions[i].node = newElement;
            }
        }

        var parent = oldElement.parentNode;
        var nextSibling = oldElement.nextSibling;
        while (oldElement.firstChild != null)
            appendChildInternal(newElement,oldElement.firstChild);
        // Deletion must be done first so if it's a heading, the outline code picks up the change
        // correctly. Otherwise, there could be two elements in the document with the same id at
        // the same time.
        deleteNodeInternal(oldElement,false);
        insertBeforeInternal(parent,newElement,nextSibling);

        for (var i = 0; i < listeners.length; i++)
            listeners[i].afterReplaceElement(oldElement,newElement);

        return newElement;
    }

    // public
    DOM_wrapNode = function(node,elementName)
    {
        return DOM_wrapSiblings(node,node,elementName);
    }

    DOM_wrapSiblings = function(first,last,elementName)
    {
        var parent = first.parentNode;
        var wrapper = DOM_createElement(document,elementName);

        if (first.parentNode != last.parentNode)
            throw new Error("first and last are not siblings");

        if (parent != null) {
            var firstOffset = DOM_nodeOffset(first);
            var lastOffset = DOM_nodeOffset(last);
            var nodeCount = lastOffset - firstOffset + 1;
            trackedPositionsForNode(parent).forEach(function (position) {
                if ((position.offset >= firstOffset) && (position.offset <= lastOffset+1)) {
                    position.node = wrapper;
                    position.offset -= firstOffset;
                }
                else if (position.offset > lastOffset+1) {
                    position.offset -= (nodeCount-1);
                }
            });

            insertBeforeInternal(parent,wrapper,first);
        }

        var end = last.nextSibling;
        var current = first;
        while (current != end) {
            var next = current.nextSibling;
            appendChildInternal(wrapper,current);
            current = next;
        }
        return wrapper;
    }

    // public
    DOM_mergeWithNextSibling = function(current,whiteList)
    {
        var parent = current.parentNode;
        var next = current.nextSibling;

        if ((next == null) || !DOM_nodesMergeable(current,next,whiteList))
            return;

        var currentLength = DOM_maxChildOffset(current);
        var nextOffset = DOM_nodeOffset(next);

        var lastChild = null;

        if (current.nodeType == Node.ELEMENT_NODE) {
            lastChild = current.lastChild;
            DOM_insertBefore(current,next,null);
            DOM_removeNodeButKeepChildren(next);
        }
        else {
            DOM_insertCharacters(current,current.nodeValue.length,next.nodeValue);

            trackedPositionsForNode(next).forEach(function (position) {
                position.node = current;
                position.offset = position.offset+currentLength;
            });

            trackedPositionsForNode(current.parentNode).forEach(function (position) {
                if (position.offset == nextOffset) {
                    position.node = current;
                    position.offset = currentLength;
                }
            });

            DOM_deleteNode(next);
        }

        if (lastChild != null)
            DOM_mergeWithNextSibling(lastChild,whiteList);
    }

    // public
    DOM_nodesMergeable = function(a,b,whiteList)
    {
        if ((a.nodeType == Node.TEXT_NODE) && (b.nodeType == Node.TEXT_NODE))
            return true;
        else if ((a.nodeType == Node.ELEMENT_NODE) && (b.nodeType == Node.ELEMENT_NODE))
            return elementsMergableTypes(a,b);
        else
            return false;

        function elementsMergableTypes(a,b)
        {
            if (whiteList["force"] && isParagraphNode(a) && isParagraphNode(b))
                return true;
            if ((a._type == b._type) &&
                whiteList[a._type] &&
                (a.attributes.length == b.attributes.length)) {
                for (var i = 0; i < a.attributes.length; i++) {
                    var attrName = a.attributes[i].nodeName; // check-ok
                    if (a.getAttribute(attrName) != b.getAttribute(attrName))
                        return false;
                }
                return true;
            }

            return false;
        }
    }

    function getDataForNode(node,create)
    {
        if (node._nodeId == null)
            throw new Error("getDataForNode: node "+node.nodeName+" has no _nodeId property");
        if ((nodeData[node._nodeId] == null) && create)
            nodeData[node._nodeId] = new Object();
        return nodeData[node._nodeId];
    }

    function trackedPositionsForNode(node)
    {
        var data = getDataForNode(node,false);
        if ((data != null) && (data.trackedPositions != null)) {
            // Sanity check
            for (var i = 0; i < data.trackedPositions.length; i++) {
                if (data.trackedPositions[i].node != node)
                    throw new Error("Position "+data.trackedPositions[i]+" has wrong node");
            }
            return arrayCopy(data.trackedPositions);
        }
        else {
            return [];
        }
    }

    function listenersForNode(node)
    {
        var data = getDataForNode(node,false);
        if ((data != null) && (data.listeners != null))
            return data.listeners;
        else
            return [];
    }

    // public
    DOM_replaceCharacters = function(textNode,startOffset,endOffset,replacement)
    {
        // Note that we do the insertion *before* the deletion so that the position is properly
        // maintained, and ends up at the end of the replacement (unless it was previously at
        // startOffset, in which case it will stay the same)
        DOM_insertCharacters(textNode,startOffset,replacement);
        DOM_deleteCharacters(textNode,startOffset+replacement.length,endOffset+replacement.length);
    }

    // public
    DOM_addTrackedPosition = function(position)
    {
        var data = getDataForNode(position.node,true);
        if (data.trackedPositions == null)
            data.trackedPositions = new Array();
        data.trackedPositions.push(position);
    }

    // public
    DOM_removeTrackedPosition = function(position)
    {
        var data = getDataForNode(position.node,false);
        if ((data == null) || (data.trackedPositions == null))
            throw new Error("DOM_removeTrackedPosition: no registered positions for this node "+
                            "("+position.node.nodeName+")");
        for (var i = 0; i < data.trackedPositions.length; i++) {
            if (data.trackedPositions[i] == position) {
                data.trackedPositions.splice(i,1);
                return;
            }
        }
        throw new Error("DOM_removeTrackedPosition: position is not registered ("+
                        data.trackedPositions.length+" others)");
    }

    // public
    DOM_removeAdjacentWhitespace = function(node)
    {
        while ((node.previousSibling != null) && (isWhitespaceTextNode(node.previousSibling)))
            DOM_deleteNode(node.previousSibling);
        while ((node.nextSibling != null) && (isWhitespaceTextNode(node.nextSibling)))
            DOM_deleteNode(node.nextSibling);
    }

    // public
    DOM_documentHead = function(document)
    {
        var html = document.documentElement;
        for (var child = html.firstChild; child != null; child = child.nextSibling) {
            if (child._type == HTML_HEAD)
                return child;
        }
        throw new Error("Document contains no HEAD element");
    }

    // public
    DOM_ensureUniqueIds = function(root)
    {
        var ids = new Object();
        var duplicates = new Array();

        discoverDuplicates(root);
        renameDuplicates();

        return;

        function discoverDuplicates(node)
        {
            if (node.nodeType != Node.ELEMENT_NODE)
                return;

            var id = node.getAttribute("id");
            if ((id != null) && (id != "")) {
                if (ids[id])
                    duplicates.push(node);
                else
                    ids[id] = true;
            }
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                discoverDuplicates(child);
        }

        function renameDuplicates()
        {
            var nextNumberForPrefix = new Object();
            for (var i = 0; i < duplicates.length; i++) {
                var id = duplicates[i].getAttribute("id");
                var prefix = id.replace(/[0-9]+$/,"");
                var num = nextNumberForPrefix[prefix] ? nextNumberForPrefix[prefix] : 1;

                var candidate;
                do {
                    candidate = prefix + num;
                    num++;
                } while (ids[candidate]);

                DOM_setAttribute(duplicates[i],"id",candidate);
                ids[candidate] = true;
                nextNumberForPrefix[prefix] = num;
            }
        }
    }

    // public
    DOM_nodeOffset = function(node,parent)
    {
        if ((node == null) && (parent != null))
            return DOM_maxChildOffset(parent);
        var offset = 0;
        for (var n = node.parentNode.firstChild; n != node; n = n.nextSibling)
            offset++;
        return offset;
    }

    // public
    DOM_maxChildOffset = function(node)
    {
        if (node.nodeType == Node.TEXT_NODE)
            return node.nodeValue.length;
        else if (node.nodeType == Node.ELEMENT_NODE)
            return node.childNodes.length;
        else
            throw new Error("maxOffset: invalid node type ("+node.nodeType+")");
    }

    function incIgnoreMutations()
    {
        UndoManager_addAction(decIgnoreMutations);
        ignoreMutations++;
    }

    function decIgnoreMutations()
    {
        UndoManager_addAction(incIgnoreMutations);
        ignoreMutations--;
        if (ignoreMutations < 0)
            throw new Error("ignoreMutations is now negative");
    }

    // public
    DOM_ignoreMutationsWhileExecuting = function(fun)
    {
        incIgnoreMutations();
        try {
            return fun();
        }
        finally {
            decIgnoreMutations();
        }
    }

    // public
    DOM_getIgnoreMutations = function()
    {
        return ignoreMutations;
    }

    // public
    DOM_addListener = function(node,listener)
    {
        var data = getDataForNode(node,true);
        if (data.listeners == null)
            data.listeners = [listener];
        else
            data.listeners.push(listener);
    }

    // public
    DOM_removeListener = function(node,listener)
    {
        var list = listenersForNode(node);
        var index = list.indexOf(listener);
        if (index >= 0)
            list.splice(index,1);
    }

    // public
    function Listener()
    {
    }

    Listener.prototype.afterReplaceElement = function(oldElement,newElement) {}

    DOM_Listener = Listener;

})();
