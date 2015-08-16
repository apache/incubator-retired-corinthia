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

function figurePropertiesString(index)
{
    var figure = document.getElementsByTagName("FIGURE")[0];
    var parent = figure.parentNode;
    var offset = DOM_nodeOffset(figure);
    Selection_set(parent,offset,parent,offset+1);
    var properties = Figures_getProperties(Figures_getSelectedFigureId());
    var strings = new Array();
    var names = Object.getOwnPropertyNames(properties).sort();
    for (var i = 0; i < names.length; i++) {
        var name = names[i];
        if (properties[name] == null)
            strings.push(name+" = null");
        else
            strings.push(name+" = "+properties[name]);
    }
    return strings.join("\n");
}
