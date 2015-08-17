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

var Editor_getBackMessages;
var Editor_debug;
var Editor_addOutlineItem;
var Editor_updateOutlineItem;
var Editor_removeOutlineItem;
var Editor_outlineUpdated;
var Editor_setCursor;
var Editor_setSelectionHandles;
var Editor_clearSelectionHandlesAndCursor;
var Editor_setSelectionBounds;
var Editor_updateAutoCorrect;
var Editor_error;
var debug;

(function(){

    var backMessages = new Array();

    function addBackMessage()
    {
        backMessages.push(arrayCopy(arguments));
        return null;
    }

    Editor_getBackMessages = function()
    {
        var result = JSON.stringify(backMessages);
        backMessages = new Array();
        return result;
    };

    Editor_debug = function(str)
    {
        addBackMessage("debug",str);
    };

    Editor_error = function(error,type)
    {
        if (type == null)
            type = "";
        addBackMessage("error",error.toString(),type);
    };

    Editor_addOutlineItem = function(itemId,type,title)
    {
        addBackMessage("addOutlineItem",itemId,type,title);
    };

    Editor_updateOutlineItem = function(itemId,title)
    {
        addBackMessage("updateOutlineItem",itemId,title);
    };

    Editor_removeOutlineItem = function(itemId)
    {
        addBackMessage("removeOutlineItem",itemId);
    };

    Editor_outlineUpdated = function()
    {
        addBackMessage("outlineUpdated");
    };

    Editor_setCursor = function(x,y,width,height)
    {
        addBackMessage("setCursor",x,y,width,height);
    };

    Editor_setSelectionHandles = function(x1,y1,height1,x2,y2,height2)
    {
        addBackMessage("setSelectionHandles",x1,y1,height1,x2,y2,height2);
    };

    Editor_setTableSelection = function(x,y,width,height)
    {
        addBackMessage("setTableSelection",x,y,width,height);
    };

    Editor_setSelectionBounds = function(left,top,right,bottom)
    {
        addBackMessage("setSelectionBounds",left,top,right,bottom);
    };

    Editor_clearSelectionHandlesAndCursor = function()
    {
        addBackMessage("clearSelectionHandlesAndCursor");
    };

    Editor_updateAutoCorrect = function()
    {
        addBackMessage("updateAutoCorrect");
    };

    debug = Editor_debug;

})();
