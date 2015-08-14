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

var Viewport_init;
var Viewport_setViewportWidth;
var Viewport_setTextScale;

(function() {

    var viewportMetaElement = null;

    // public
    Viewport_init = function(width,textScale)
    {
        var head = DOM_documentHead(document);
        for (var child = head.firstChild; child != null; child = child.nextSibling) {
            if ((child._type == HTML_META) && (child.getAttribute("name") == "viewport")) {
                viewportMetaElement = child;
                break;
            }
        }

        if (viewportMetaElement == null) {
            viewportMetaElement = DOM_createElement(document,"META");
            DOM_setAttribute(viewportMetaElement,"name","viewport");
            DOM_appendChild(head,viewportMetaElement);
        }

        if (width != 0) {
            // Only set the width and text scale if they are not already set, to avoid triggering
            // an extra layout at load time
            var contentValue = "width = "+width+", user-scalable = no";
            if (viewportMetaElement.getAttribute("content") != contentValue)
                DOM_setAttribute(viewportMetaElement,"content",contentValue);
        }

        if (textScale != 0) {
            var pct = textScale+"%";
            if (document.documentElement.style.getPropertyValue("-webkit-text-size-adjust") != pct)
                DOM_setStyleProperties(document.documentElement,{"-webkit-text-size-adjust": pct});
        }
    }

    // public
    Viewport_setViewportWidth = function(width)
    {
        var contentValue = "width = "+width+", user-scalable = no";
        if (viewportMetaElement.getAttribute("content") != contentValue)
            DOM_setAttribute(viewportMetaElement,"content",contentValue);

        Selection_update();
        Cursor_ensureCursorVisible();
    }

    // public
    Viewport_setTextScale = function(textScale)
    {
        var pct = textScale+"%";
        if (document.documentElement.style.getPropertyValue("-webkit-text-size-adjust") != pct)
            DOM_setStyleProperties(document.documentElement,{"-webkit-text-size-adjust": pct});

        Selection_update();
        Cursor_ensureCursorVisible();
    }

})();
