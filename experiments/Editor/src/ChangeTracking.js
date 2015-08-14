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

var ChangeTracking_showChanges;
var ChangeTracking_trackChanges;
var ChangeTracking_setShowChanges;
var ChangeTracking_setTrackChanges;
var ChangeTracking_acceptSelectedChanges;

(function() {

    var showChangesEnabled = false;
    var trackChangesEnabled = false;

    ChangeTracking_showChanges = function()
    {
        return showChangesEnabled;
    }

    ChangeTracking_trackChanges = function()
    {
        return trackChangesEnabled;
    }

    ChangeTracking_setShowChanges = function(enabled)
    {
        showChangesEnabled = enabled;
    }

    ChangeTracking_setTrackChanges = function(enabled)
    {
        trackChangesEnabled = enabled;
    }

    ChangeTracking_acceptSelectedChanges = function()
    {
        var selRange = Selection_get();
        if (selRange == null)
            return;

        var outermost = Range_getOutermostNodes(selRange,true);
        var checkEmpty = new Array();

        Selection_preserveWhileExecuting(function() {
            for (var i = 0; i < outermost.length; i++) {
                recurse(outermost[i]);

                var next;
                for (ancestor = outermost[i].parentNode; ancestor != null; ancestor = next) {
                    next = ancestor.parentNode;
                    if (ancestor._type == HTML_DEL) {
                        checkEmpty.push(ancestor.parentNode);
                        DOM_deleteNode(ancestor);
                    }
                    else if (ancestor._type == HTML_INS)
                        DOM_removeNodeButKeepChildren(ancestor);
                }
            }

            for (var i = 0; i < checkEmpty.length; i++) {
                var node = checkEmpty[i];
                if (node == null)
                    continue;
                var empty = true;
                for (var child = node.firstChild; child != null; child = child.nextSibling) {
                    if (!isWhitespaceTextNode(child)) {
                        empty = false;
                        break;
                    }
                }
                if (empty) {
                    switch (node._type) {
                    case HTML_LI:
                    case HTML_UL:
                    case HTML_OL:
                        checkEmpty.push(node.parentNode);
                        DOM_deleteNode(node);
                        break;
                    }
                }
            }
        });

        var selRange = Selection_get();
        if (selRange != null) {
            var start = Position_closestMatchForwards(selRange.start,Position_okForInsertion);
            var end = Position_closestMatchBackwards(selRange.end,Position_okForInsertion);
            if (!Range_isForwards(new Range(start.node,start.offset,end.node,end.offset)))
                end = Position_closestMatchForwards(selRange.end,Position_okForInsertion);
            Selection_set(start.node,start.offset,end.node,end.offset);
        }

        function recurse(node)
        {
            if (node._type == HTML_DEL) {
                checkEmpty.push(node.parentNode);
                DOM_deleteNode(node);
                return;
            }

            var next;
            for (var child = node.firstChild; child != null; child = next) {
                next = child.nextSibling;
                recurse(child);
            }

            if (node._type == HTML_INS) {
                DOM_removeNodeButKeepChildren(node);
            }
        }
    }

})();
