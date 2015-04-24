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

var Main_getLanguage;
var Main_setLanguage;
var Main_setGenerator;
var Main_isEmptyDocument;
var Main_prepareForSave;
var Main_getHTML;
var Main_getErrorReportingInfo;
var Main_removeUnsupportedInput;
var Main_removeSpecial;
var Main_execute;
var Main_init;

var Main_clientRectsBug;

(function() {

    // public
    Main_getLanguage = function()
    {
        var lang = document.documentElement.getAttribute("lang");
        if (lang != null)
            lang = lang.replace(/-/g,"_");
        return lang;
    }

    // public
    Main_setLanguage = function(lang)
    {
        if ((lang == null) || (lang == "")) {
            DOM_removeAttribute(document.documentElement,"lang");
        }
        else {
            lang = lang.replace(/_/g,"-");
            DOM_setAttribute(document.documentElement,"lang",lang);
        }
    }

    // public
    Main_removeUnsupportedInput = function()
    {
        recurse(document.documentElement);

        function recurse(node)
        {
            // Delete comments and processing instructions
            if ((node.nodeType != Node.TEXT_NODE) &&
                (node.nodeType != Node.ELEMENT_NODE)) {
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

    // private
    function addMetaCharset()
    {
        var head = DOM_documentHead(document);
        var next;
        for (var child = head.firstChild; child != null; child = next) {
            next = child.nextSibling;
            if ((child._type == HTML_META) && (child.hasAttribute("charset"))) {
                DOM_deleteNode(child);
            }
            else if ((child._type == HTML_META) && child.hasAttribute("http-equiv") &&
                     (child.getAttribute("http-equiv").toLowerCase() == "content-type")) {
                DOM_deleteNode(child);
            }
        }

        var meta = DOM_createElement(document,"META");
        DOM_setAttribute(meta,"charset","utf-8");
        DOM_insertBefore(head,meta,head.firstChild);
    }

    // public
    Main_setGenerator = function(generator)
    {
        return UndoManager_disableWhileExecuting(function() {
            var head = DOM_documentHead(document);
            for (var child = head.firstChild; child != null; child = child.nextSibling) {
                if ((child._type == HTML_META) &&
                    child.hasAttribute("name") &&
                    (child.getAttribute("name").toLowerCase() == "generator")) {
                    var origGenerator = DOM_getAttribute(child,"content");
                    DOM_setAttribute(child,"content",generator);

                    if (origGenerator == null)
                        return "";
                    else
                        return origGenerator;
                }
            }

            var meta = DOM_createElement(document,"META");
            DOM_setAttribute(meta,"name","generator");
            DOM_setAttribute(meta,"content",generator);
            DOM_insertBefore(head,meta,head.firstChild);

            return "";
        });
    }

    // public
    Main_isEmptyDocument = function()
    {
        return !nodeHasContent(document.body);
    }

    // public
    Main_prepareForSave = function()
    {
        // Force any end-of-group actions to be performed
        UndoManager_newGroup();
        return true;
    }

    // public
    Main_getHTML = function()
    {
        return document.documentElement.outerHTML;
    }

    // public
    Main_getErrorReportingInfo = function()
    {
        if (document.documentElement == null)
            return "(document.documentElement is null)";
        try {
            var html = htmlWithSelection();
            cleanse(html);
            return html.outerHTML;
        }
        catch (e) {
            try {
                var html = DOM_cloneNode(document.documentElement,true);
                cleanse(html);
                return html.outerHTML+"\n[Error getting selection: "+e+"]";
            }
            catch (e2) {
                return "[Error getting HTML: "+e2+"]";
            }
        }

        function cleanse(node)
        {
            switch (node._type) {
            case HTML_TEXT:
            case HTML_COMMENT:
                DOM_setNodeValue(node,cleanseString(node.nodeValue));
                break;
            case HTML_STYLE:
            case HTML_SCRIPT:
                return;
            default:
                if (node.nodeType == Node.ELEMENT_NODE) {
                    cleanseAttribute(node,"original");
                    if (node.hasAttribute("href") && !node.getAttribute("href").match(/^#/))
                        cleanseAttribute(node,"href");
                    for (var child = node.firstChild; child != null; child = child.nextSibling)
                        cleanse(child);
                }
                break;
            }
        }

        function cleanseAttribute(node,name)
        {
            if (node.hasAttribute(name)) {
                var value = node.getAttribute(name);
                value = cleanseString(value);
                DOM_setAttribute(node,name,value);
            }
        }

        function cleanseString(str)
        {
            return str.replace(/[^\s\.\@\^]/g,"X");
        }

        function htmlWithSelection()
        {
            var selectionRange = Selection_get();
            if (selectionRange != null) {
                selectionRange = Range_forwards(selectionRange);
                var startSave = new Object();
                var endSave = new Object();

                var html = null;

                Range_trackWhileExecuting(selectionRange,function() {
                    // We use the strings @@^^ and ^^@@ to represent the selection
                    // start and end, respectively. The reason for this is that after we have
                    // cloned the tree, all text will be removed. We keeping the @ and ^
                    // characters so we have some way to identifiy the selection markers;
                    // leaving these in is not going to reveal any confidential information.

                    addPositionMarker(selectionRange.end,"^^@@",endSave);
                    addPositionMarker(selectionRange.start,"@@^^",startSave);

                    html = DOM_cloneNode(document.documentElement,true);

                    removePositionMarker(selectionRange.start,startSave);
                    removePositionMarker(selectionRange.end,endSave);
                });

                return html;
            }
            else {
                return DOM_cloneNode(document.documentElement,true);
            }
        }

        function addPositionMarker(pos,name,save)
        {
            var node = pos.node;
            var offset = pos.offset;
            if (node.nodeType == Node.ELEMENT_NODE) {
                save.tempNode = DOM_createTextNode(document,name);
                DOM_insertBefore(node,save.tempNode,node.childNodes[offset]);
            }
            else if (node.nodeType == Node.TEXT_NODE) {
                save.originalNodeValue = node.nodeValue;
                node.nodeValue = node.nodeValue.slice(0,offset) + name + node.nodeValue.slice(offset);
            }
        }

        function removePositionMarker(pos,save)
        {
            var node = pos.node;
            var offset = pos.offset;
            if (pos.node.nodeType == Node.ELEMENT_NODE) {
                DOM_deleteNode(save.tempNode);
            }
            else if (pos.node.nodeType == Node.TEXT_NODE) {
                node.nodeValue = save.originalNodeValue;
            }
        }
    }

    // public
    Main_removeSpecial = function(node)
    {
        // We process the children first, so that if there are any nested removable elements (e.g.
        // a selection span inside of an autocorrect span), all levels of nesting are taken care of
        var next;
        for (var child = node.firstChild; child != null; child = next) {
            next = child.nextSibling;
            Main_removeSpecial(child);
        }

        var cssClass = null;
        if ((node.nodeType == Node.ELEMENT_NODE) && node.hasAttribute("class"))
            cssClass = node.getAttribute("class");

        if ((cssClass == Keys.HEADING_NUMBER) ||
            (cssClass == Keys.FIGURE_NUMBER) ||
            (cssClass == Keys.TABLE_NUMBER) ||
            (cssClass == Keys.AUTOCORRECT_CLASS) ||
            (cssClass == Keys.SELECTION_CLASS) ||
            (cssClass == Keys.SELECTION_HIGHLIGHT)) {
            DOM_removeNodeButKeepChildren(node);
        }
        else if ((node._type == HTML_META) &&
                 node.hasAttribute("name") &&
                 (node.getAttribute("name").toLowerCase() == "viewport")) {
            DOM_deleteNode(node);
        }
        else if (node._type == HTML_LINK) {
            if ((node.getAttribute("rel") == "stylesheet") &&
                (node.getAttribute("href") == Styles_getBuiltinCSSURL())) {
                DOM_deleteNode(node);
            }
        }
    }

    function simplifyStackString(e)
    {
        if (e.stack == null)
            return "";
        var lines = e.stack.toString().split(/\n/);
        for (var i = 0; i < lines.length; i++) {
            var nameMatch = lines[i].match(/^(.*)@/);
            var name = (nameMatch != null) ? nameMatch[1] : "(anonymous function)";
            var locMatch = lines[i].match(/:([0-9]+:[0-9]+)$/);
            var loc = (locMatch != null) ? locMatch[1] : "?";
            lines[i] = "stack["+(lines.length-i-1)+"] = "+name+"@"+loc;
        }
        return lines.join("\n");
    }

    // public
    Main_execute = function(fun)
    {
        try {
            var res = fun();
            PostponedActions_perform();
            return res;
        }
        catch (e) {
            var message = (e.message != null) ? e.message : e.toString();
            var stack = simplifyStackString(e);
            Editor_error(message+"\n"+stack);
        }
    }

    function fixEmptyBody()
    {
        for (var child = document.body.firstChild; child != null; child = child.nextSibling) {
            if (nodeHasContent(child))
                return;
        }

        for (var child = document.body.firstChild; child != null; child = child.nextSibling) {
            if (child._type == HTML_P) {
                Cursor_updateBRAtEndOfParagraph(child);
                return;
            }
        }

        var p = DOM_createElement(document,"P");
        var br = DOM_createElement(document,"BR");
        DOM_appendChild(p,br);
        DOM_appendChild(document.body,p);
    }

    // public
    Main_init = function(width,textScale,cssURL,clientRectsBug)
    {
        try {
            Main_clientRectsBug = clientRectsBug;
            if (document.documentElement == null)
                throw new Error("document.documentElement is null");
            if (document.body == null)
                throw new Error("document.body is null");
            var timing = new TimingInfo();
            timing.start();
            DOM_assignNodeIds(document);
            timing.addEntry("DOM_assignNodeIds");
            Main_removeUnsupportedInput();
            timing.addEntry("Main_removeUnsupportedInput");
            addMetaCharset();
            timing.addEntry("addMetaCharset");
            fixEmptyBody();
            timing.addEntry("fixEmptyBody");
            Outline_init();
            timing.addEntry("Outline_init");
            Styles_init(cssURL);
            timing.addEntry("Styles_init");
            Viewport_init(width,textScale);
            timing.addEntry("Viewport_init");
            AutoCorrect_init();
            timing.addEntry("AutoCorrect_init");

            PostponedActions_perform();
            timing.addEntry("PostponedActions_perform");
            Cursor_moveToStartOfDocument();
            timing.addEntry("Cursor_moveToStartOfDocument");

            UndoManager_clear();
            timing.addEntry("UndoManager_clear");
//            timing.print();

            return true;
        }
        catch (e) {
            return e.toString();
        }
    }

})();
