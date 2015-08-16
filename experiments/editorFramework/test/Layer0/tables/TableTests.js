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

function showSelectedTableRegion()
{
    var region = Tables_regionFromRange(Selection_get());
    for (var row = region.top; row <= region.bottom; row++) {
        for (var col = region.left; col <= region.right; col++) {
            var cell = Table_get(region.structure,row,col);
            DOM_setStyleProperties(cell.element,{"background-color": "silver"});
        }
    }
}

function getSelectedTableRegion()
{
    return Tables_regionFromRange(Selection_get());
}

function showTableStructure()
{
    var tableElement = document.getElementsByTagName("TABLE")[0];
    var table = Tables_analyseStructure(tableElement);
    var lines = new Array();
    lines.push(PrettyPrinter.getHTML(document.documentElement));

    for (var row = 0; row < table.numRows; row++) {
        for (var col = 0; col < table.numCols; col++) {
            var cell = Table_get(table,row,col);
            if (cell == null) {
                lines.push("Cell at ("+row+","+col+") = "+null);
            }
            else {
                lines.push("Cell at ("+row+","+col+") = "+
                           cell.rowspan+"x"+cell.colspan+" "+
                           JSON.stringify(getNodeText(cell.element)));
            }
        }
    }

    return lines.join("\n");
}
