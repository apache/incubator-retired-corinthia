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

var Figures_insertFigure;
var Figures_getSelectedFigureId;
var Figures_getProperties;
var Figures_setProperties;
var Figures_getGeometry;

(function() {

    // public
    Figures_insertFigure = function(filename,width,numbered,caption)
    {
        UndoManager_newGroup("Insert figure");

        var figure = DOM_createElement(document,"FIGURE");
        var img = DOM_createElement(document,"IMG");
        DOM_setAttribute(img,"src",encodeURI(filename));
        DOM_setStyleProperties(img,{"width": width});
        DOM_appendChild(figure,img);

        if ((caption != null) && (caption != "")) {
            var figcaption = DOM_createElement(document,"FIGCAPTION");
            DOM_appendChild(figcaption,DOM_createTextNode(document,caption));
            DOM_appendChild(figure,figcaption);
        }

        Clipboard_pasteNodes([figure]);

        // Now that the figure has been inserted into the DOM tree, the outline code will
        // have noticed it and added an id attribute, as well as a caption giving the
        // table number.
        Outline_setNumbered(figure.getAttribute("id"),numbered);

        // Place the cursor directly after the figure
        var offset = DOM_nodeOffset(figure);
        var pos = new Position(figure.parentNode,offset);
        pos = Position_closestMatchForwards(pos,Position_okForMovement);
        Selection_set(pos.node,pos.offset,pos.node,pos.offset);

        PostponedActions_add(UndoManager_newGroup);
    }

    Figures_getSelectedFigureId = function()
    {
        var element = Cursor_getAdjacentNodeWithType(HTML_FIGURE);
        return element ? element.getAttribute("id") : null;
    }

    // public
    Figures_getProperties = function(itemId)
    {
        var figure = document.getElementById(itemId);
        if (figure == null)
            return null;
        var rect = figure.getBoundingClientRect();
        var result = { width: null, src: null };

        var img = firstDescendantOfType(figure,HTML_IMG);
        if (img != null) {
            result.src = decodeURI(img.getAttribute("src"));
            result.width = img.style.width;

            if ((result.width == null) || (result.width == ""))
                result.width = DOM_getAttribute(img,"width");
        }
        return result;
    }

    // public
    Figures_setProperties = function(itemId,width,src)
    {
        var figure = document.getElementById(itemId);
        if (figure == null)
            return null;
        var img = firstDescendantOfType(figure,HTML_IMG);
        if (img != null) {
            if (src == null)
                DOM_removeAttribute(img,"src");
            else
                DOM_setAttribute(img,"src",encodeURI(src));

            DOM_setStyleProperties(img,{"width": width});
            if (img.getAttribute("style") == "")
                DOM_removeAttribute(img,"style");
            Selection_update();
        }
    }

    // public
    Figures_getGeometry = function(itemId)
    {
        var figure = document.getElementById(itemId);
        if ((figure == null) || (figure.parentNode == null))
            return null;
        var img = firstDescendantOfType(figure,HTML_IMG);
        if (img == null)
            return null;

        var figcaption = firstChildOfType(figure,HTML_FIGCAPTION);

        var result = new Object();
        result.contentRect = xywhAbsElementRect(img);
        result.fullRect = xywhAbsElementRect(figure);
        result.parentRect = xywhAbsElementRect(figure.parentNode);
        result.hasCaption = (figcaption != null);
        return result;
    }

})();
