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

function prevNode(node)
{
    if (node.previousSibling != null) {
        node = node.previousSibling;
        while (node.lastChild != null)
            node = node.lastChild;
        return node;
    }
    else {
        return node.parentNode;
    }
}

function nextNodeAfter(node,entering,exiting)
{
    while (node != null) {
        if (node.nextSibling != null) {
            if (exiting != null)
                exiting(node);
            node = node.nextSibling;
            if (entering != null)
                entering(node);
            break;
        }

        if (exiting != null)
            exiting(node);
        node = node.parentNode;
    }
    return node;
}

function nextNode(node,entering,exiting)
{
    if (node.firstChild) {
        node = node.firstChild;
        if (entering != null)
            entering(node);
        return node;
    }
    else {
        return nextNodeAfter(node,entering,exiting);
    }
}

function prevTextNode(node)
{
    do {
        node = prevNode(node);
    } while ((node != null) && (node.nodeType != Node.TEXT_NODE));
    return node;
}

function nextTextNode(node)
{
    do {
        node = nextNode(node);
    } while ((node != null) && (node.nodeType != Node.TEXT_NODE));
    return node;
}

function firstChildElement(node)
{
    var first = node.firstChild;
    while ((first != null) && (first.nodeType != Node.ELEMENT_NODE))
        first = first.nextSibling;
    return first;
}

function lastChildElement(node)
{
    var last = node.lastChild;
    while ((last != null) && (last.nodeType != Node.ELEMENT_NODE))
        last = last.previousSibling;
    return last;
}

function firstDescendant(node)
{
    while (node.firstChild != null)
        node = node.firstChild;
    return node;
}

function lastDescendant(node)
{
    while (node.lastChild != null)
        node = node.lastChild;
    return node;
}

function firstDescendantOfType(node,type)
{
    if (node._type == type)
        return node;

    for (var child = node.firstChild; child != null; child = child.nextSibling) {
        var result = firstDescendantOfType(child,type);
        if (result != null)
            return result;
    }
    return null;
}

function firstChildOfType(node,type)
{
    for (var child = node.firstChild; child != null; child = child.nextSibling) {
        if (child._type == type)
            return child;
    }
    return null;
}

function getNodeDepth(node)
{
    var depth = 0;
    for (; node != null; node = node.parentNode)
        depth++;
    return depth;
}

function getNodeText(node)
{
    var strings = new Array();
    recurse(node);
    return strings.join("").replace(/\s+/g," ");

    function recurse(node)
    {
        if (node.nodeType == Node.TEXT_NODE)
            strings.push(node.nodeValue);

        for (var child = node.firstChild; child != null; child = child.nextSibling)
            recurse(child);
    }
}

function isWhitespaceTextNode(node)
{
    if (node.nodeType != Node.TEXT_NODE)
        return false;
    return isWhitespaceString(node.nodeValue);
}

function isNonWhitespaceTextNode(node)
{
    if (node.nodeType != Node.TEXT_NODE)
        return false;
    return !isWhitespaceString(node.nodeValue);
}

function printTree(node,indent,offset)
{
    if (indent == null)
        indent = "";
    if (offset == null)
        offset = "";
    if ((node.nodeType == Node.ELEMENT_NODE) && node.hasAttribute("class"))
        debug(indent+offset+nodeString(node)+"."+node.getAttribute("class"));
    else
        debug(indent+offset+nodeString(node));
    var childOffset = 0;
    for (var child = node.firstChild; child != null; child = child.nextSibling) {
        printTree(child,indent+"    ",childOffset+" ");
        childOffset++;
    }
}
