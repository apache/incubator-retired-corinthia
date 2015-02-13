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

var Preview_showForStyle;

(function(){

    var previewText =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec in diam \n"+
        "mauris. Integer in lorem sit amet dolor lacinia aliquet. Cras vehicula odio \n"+
        "non enim euismod nec congue lorem varius. Sed eu libero arcu, eget tempus \n"+
        "augue. Vivamus varius risus ac libero sagittis eu ultricies lectus \n"+
        "consequat. Integer gravida accumsan fermentum. Morbi erat ligula, volutpat \n"+
        "non accumsan sed, pellentesque quis purus. Vestibulum vestibulum tincidunt \n"+
        "lectus non pellentesque. Quisque porttitor sollicitudin tellus, id porta \n"+
        "velit interdum sit amet. Cras quis sem orci, vel convallis magna. \n"+
        "Pellentesque congue, libero et iaculis volutpat, enim turpis sodales dui, \n"+
        "lobortis pharetra lectus dolor at sem. Nullam aliquam, odio ac laoreet \n"+
        "vulputate, ligula nunc euismod leo, vel bibendum magna leo ut orci. In \n"+
        "tortor turpis, pellentesque nec cursus ut, consequat non ipsum. Praesent \n"+
        "venenatis, leo in pulvinar pharetra, eros nisi convallis elit, vitae luctus \n"+
        "magna velit ut lorem."

    function setTableCellContents(node)
    {
        if (isTableCell(node)) {
            DOM_deleteAllChildren(node);
            DOM_appendChild(node,DOM_createTextNode(document,"Cell contents"));
        }
        else {
            for (var child = node.firstChild; child != null; child = child.nextSibling)
                setTableCellContents(child);
        }
    }

    function showForStyle(styleId,uiName,titleText)
    {
        var elementName = null;
        var className = null;

        var dotPos = styleId.indexOf(".");
        if (dotPos >= 0) {
            elementName = styleId.substring(0,dotPos);
            className = styleId.substring(dotPos+1);
        }
        else {
            elementName = styleId;
            className = null;
        }

        var title = DOM_createTextNode(document,titleText);
        var text = DOM_createTextNode(document,previewText);

        Selection_clear();
        DOM_deleteAllChildren(document.body);

        if (PARAGRAPH_ELEMENTS[ElementTypes[elementName]]) {
            var paragraph1 = createParagraphElement(elementName,className);
            var paragraph2 = createParagraphElement(elementName,className);
            DOM_appendChild(paragraph1,title);
            DOM_appendChild(paragraph2,text);
            DOM_appendChild(document.body,paragraph1);
            DOM_appendChild(document.body,paragraph2);

            if (className != null) {
                DOM_setAttribute(paragraph1,"class",className);
                DOM_setAttribute(paragraph2,"class",className);
            }
        }
        else if (elementName == "span") {
            var p1 = DOM_createElement(document,"P");
            var p2 = DOM_createElement(document,"P");
            var span1 = DOM_createElement(document,"SPAN");
            var span2 = DOM_createElement(document,"SPAN");

            if (className != null) {
                DOM_setAttribute(span1,"class",className);
                DOM_setAttribute(span2,"class",className);
            }

            DOM_appendChild(span1,title);
            DOM_appendChild(span2,text);

            DOM_appendChild(p1,span1);
            DOM_appendChild(p2,span2);

            DOM_appendChild(document.body,p1);
            DOM_appendChild(document.body,p2);
        }
        else if ((elementName == "table") || (elementName == "caption")) {
            // FIXME: cater for different table styles
            Selection_selectAll();
            Tables_insertTable(3,3,"66%",true,"Table caption");
            Selection_clear();
            var table = document.getElementsByTagName("TABLE")[0];
            setTableCellContents(table);
            if ((elementName == "table") && (className != null))
                DOM_setAttribute(table,"class",className);
        }
        else if ((elementName == "figure") || (elementName == "figcaption")) {
            Selection_selectAll();
            Figures_insertFigure("SampleFigure.svg","75%",true,"TCP 3-way handshake");
            Selection_clear();
        }
        else if (elementName == "body") {
            // We use BR here instead of separate paragraphs, since we don't want the properties
            // for the P element to be applied
            DOM_appendChild(document.body,title);
            DOM_appendChild(document.body,DOM_createElement(document,"BR"));
            DOM_appendChild(document.body,DOM_createElement(document,"BR"));
            DOM_appendChild(document.body,text);
        }

        function createParagraphElement(elementName,className)
        {
            var element = DOM_createElement(document,elementName);
            if (className != null)
                DOM_setAttribute(element,"class",className);
            return element;
        }
    }

    Preview_showForStyle = showForStyle;

})();
