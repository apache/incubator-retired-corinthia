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

var PostponedActions_add;
var PostponedActions_perform;

(function() {

    function PostponedAction(fun,undoDisabled)
    {
        this.fun = fun;
        this.undoDisabled = undoDisabled;
    }

    var actions = new Array();

    PostponedActions_add = function(action)
    {
        actions.push(new PostponedAction(action,UndoManager_isDisabled()));
    }

    PostponedActions_perform = function()
    {
        var count = 0;
        while (actions.length > 0) {
            if (count >= 10)
                throw new Error("Too many postponed actions");
            var actionsToPerform = actions;
            actions = new Array();
            for (var i = 0; i < actionsToPerform.length; i++) {
                var action = actionsToPerform[i];
                if (action.undoDisabled)
                    UndoManager_disableWhileExecuting(action.fun);
                else
                    action.fun();
            }
            count++;
        }
    }

})();
