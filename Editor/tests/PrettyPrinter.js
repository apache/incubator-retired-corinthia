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

(function() {

    // Applicable options:
    // keepSelectionHighlights (boolean)
    // preserveCase (boolean)
    // showNamespaceDetails (boolean)
    // separateLines (boolean)

    function getHTML(root,options)
    {
        var copy;
        UndoManager_disableWhileExecuting(function() {
            if (options == null)
                options = new Object();
            copy = DOM_cloneNode(root,true);
            if (!options.keepSelectionHighlights)
                removeSelectionSpans(copy);
            for (var body = copy.firstChild; body != null; body = body.nextSibling) {
                if (body.nodeName == "BODY") {
                    DOM_removeAttribute(body,"style");
                    DOM_removeAttribute(body,"contentEditable");
                }
            }
        });

        var output = new Array();
        prettyPrint(output,options,copy,"");
        return output.join("");
    }

    function removeSelectionSpans(root)
    {
        var checkMerge = new Array();
        recurse(root);

        for (var i = 0; i < checkMerge.length; i++) {
            if (checkMerge[i].parentNode != null) { // if not already merged
                Formatting_mergeWithNeighbours(checkMerge[i],{});
            }
        }

        function recurse(node) {
            if (isSelectionHighlight(node)) {
                checkMerge.push(node.firstChild);
                checkMerge.push(node.lastChild);
                DOM_removeNodeButKeepChildren(node);
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

    function entityFix(str)
    {
        return str.replace(/\u00a0/g,"&nbsp;");
    }

    function singleDescendents(node)
    {
        var count = 0;
        for (var child = node.firstChild; child != null; child = child.nextSibling) {
            if ((child.nodeType == Node.TEXT_NODE) && (textNodeDisplayValue(child).length == 0))
                continue;
            count++;
            if (count > 1)
                return false;
            if (!singleDescendents(child))
                return false;
        }
        return true;
    }

    function sortCSSProperties(value)
    {
        // Make sure the CSS properties on the "style" attribute appear in a consistent order
        var items = value.trim().split(/\s*;\s*/);
        if ((items.length > 0) && (items[items.length-1] == ""))
            items.length--;
        items.sort();
        return items.join("; ");
    }

    function attributeString(options,node)
    {
        // Make sure the attributes appear in a consistent order
        var names = new Array();
        for (var i = 0; i < node.attributes.length; i++) {
            names.push(node.attributes[i].nodeName);
        }
        names.sort();
        var str = "";
        for (var i = 0; i < names.length; i++) {
            var name = names[i];

            var value = node.getAttribute(name);
            if (name == "style")
                value = sortCSSProperties(value);
            var attr = node.getAttributeNode(name);
            if (options.showNamespaceDetails) {
                if ((attr.namespaceURI != null) || (attr.prefix != null))
                    name = "{"+attr.namespaceURI+","+attr.prefix+","+attr.localName+"}"+name;
            }
            str += " "+name+"=\""+value+"\"";
        }
        return str;
    }

    function textNodeDisplayValue(node)
    {
        var value = entityFix(node.nodeValue);
        if ((node.parentNode != null) &&
            (node.parentNode.getAttribute("xml:space") != "preserve"))
            value = value.trim();
        return value;
    }

    function prettyPrintOneLine(output,options,node)
    {
        if ((node.nodeType == Node.ELEMENT_NODE) && (node.nodeName != "SCRIPT")) {
            var name = options.preserveCase ? node.nodeName : node.nodeName.toLowerCase();
            if (node.firstChild == null) {
                output.push("<" + name + attributeString(options,node) + "/>");
            }
            else {
                output.push("<" + name + attributeString(options,node) + ">");
                for (var child = node.firstChild; child != null; child = child.nextSibling)
                    prettyPrintOneLine(output,options,child);
                output.push("</" + name + ">");
            }
        }
        else if (node.nodeType == Node.TEXT_NODE) {
            var value = textNodeDisplayValue(node);
            if (value.length > 0)
                output.push(value);
        }
        else if (node.nodeType == Node.COMMENT_NODE) {
            output.push("<!--" + entityFix(node.nodeValue) + "-->\n");
        }
    }

    function isContainer(node)
    {
        switch (node._type) {
        case HTML_BODY:
        case HTML_SECTION:
        case HTML_FIGURE:
        case HTML_TABLE:
        case HTML_TBODY:
        case HTML_THEAD:
        case HTML_TFOOT:
        case HTML_TR:
        case HTML_DIV:
        case HTML_UL:
        case HTML_OL:
        case HTML_NAV:
        case HTML_COLGROUP:
            return true;
        default:
            return false;
        }
    }

    function prettyPrint(output,options,node,indent)
    {
        if ((node.nodeType == Node.ELEMENT_NODE) && (node.nodeName != "SCRIPT")) {
            var name = options.preserveCase ? node.nodeName : node.nodeName.toLowerCase();
            if (node.firstChild == null) {
                output.push(indent + "<" + name + attributeString(options,node) + "/>\n");
            }
            else {
                if (node._type == HTML_STYLE) {
                    output.push(indent + "<" + name + attributeString(options,node) + ">\n");
                    for (var child = node.firstChild; child != null; child = child.nextSibling)
                        prettyPrint(output,options,child,"");
                    output.push(indent + "</" + name + ">\n");
                }
                else if (!options.separateLines && singleDescendents(node) && !isContainer(node)) {
                    output.push(indent);
                    prettyPrintOneLine(output,options,node);
                    output.push("\n");
                }
                else {
                    output.push(indent + "<" + name + attributeString(options,node) + ">\n");
                    for (var child = node.firstChild; child != null; child = child.nextSibling)
                        prettyPrint(output,options,child,indent+"  ");
                    output.push(indent + "</" + name + ">\n");
                }
            }
        }
        else if (node.nodeType == Node.TEXT_NODE) {
            var value = textNodeDisplayValue(node);
//            var value = JSON.stringify(node.nodeValue);
            if (value.length > 0)
                output.push(indent + value + "\n");
        }
        else if (node.nodeType == Node.COMMENT_NODE) {
            output.push(indent + "<!--" + entityFix(node.nodeValue) + "-->\n");
        }
    }

    window.PrettyPrinter = new Object();
    window.PrettyPrinter.getHTML = getHTML;

})();
