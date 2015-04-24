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

function oldInsertCharacter(character)
{
    var selectionRange = Selection_get();
    if (selectionRange == null)
        return;

    if (!Range_isEmpty(selectionRange))
        Selection_deleteContents();
    var pos = selectionRange.start;
    var node = pos.node;
    var offset = pos.offset;

    if (node.nodeType == Node.ELEMENT_NODE) {
        var prev = node.childNodes[offset-1];
        var next = node.childNodes[offset];
        var emptyTextNode = DOM_createTextNode(document,"");
        if (offset >= node.childNodes.length)
            DOM_appendChild(node,emptyTextNode);
        else
            DOM_insertBefore(node,emptyTextNode,node.childNodes[offset]);
        node = emptyTextNode;
        offset = 0;
    }

    DOM_insertCharacters(node,offset,character);
    Selection_set(node,offset+1,node,offset+1);
}

function showValidPositions()
{
    var validPositions = new Array();
    var pos = new Position(document.body,0);
    while (pos != null) {
        if (Position_okForMovement(pos)) {
//            debug("Valid position: "+pos);
            validPositions.push(pos);
        }
        pos = Position_next(pos);
    }

    Position_trackWhileExecuting(validPositions,function() {
//        for (var i = 0; i < validPositions.length; i++) {
        for (var i = validPositions.length-1; i >= 0; i--) {
            var pos = validPositions[i];
            Selection_setEmptySelectionAt(pos.node,pos.offset);
            oldInsertCharacter('.');
        }
    });
}

function flattenTreeToString(node)
{
    var result = new Array();
    recurse(node);
    return result.join("").replace(/\n/g," ");

    function recurse(node)
    {
        switch (node._type) {
        case HTML_TEXT:
            result.push(node.nodeValue);
            break;
        case HTML_IMG:
            result.push("I");
            break;
        default:
            if (isOpaqueNode(node)) {
                result.push("O");
            }
            else if (node.nodeType == Node.ELEMENT_NODE) {
                for (var child = node.firstChild; child != null; child = child.nextSibling) {
                    recurse(child);
                }
            }
            break;
        }
    }
}

function findCursorPositionErrors(text)
{
    var detail = "";
    for (var i = 0; i < text.length; i++) {
        var prevChar = (i > 0) ? text.charAt(i-1) : null;
        var nextChar = (i < text.length-1) ? text.charAt(i+1) : null;
        var curChar = text.charAt(i);

        if (curChar == '.') {
            if ((prevChar == '.') || (nextChar == '.')) {
                // Two positions not separated by a space or character
                detail += "^";
            }
            else if ((prevChar != null) && (nextChar != null) &&
                     isWhitespaceString(prevChar) && isWhitespaceString(nextChar)) {
                // A position between two spaces
                detail += "^";
            }
            else {
                // OK
                detail += " ";
            }
        }
        else if (!isWhitespaceString(curChar)) {
            if ((prevChar != '.') || (nextChar != '.'))
                detail += "^";
            else
                detail += " ";
        }
    }
    return detail;
}

function checkCursorPositions(node)
{
    var text = flattenTreeToString(document.body);
    var detail = findCursorPositionErrors(text);
    return text+"\n"+detail;
}

function addEmptyTextNode(parent)
{
    var text = DOM_createTextNode(document,"");
    DOM_appendChild(parent,text);
}
