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

var topArea;
var leftArea;
var rightArea;
var leftLoadedContinuation = null;
var results = new Object();
var allCode = null;

function Result(actual,expected)
{
    this.actual = actual;
    this.expected = expected;
}

function readFile(filename)
{
    var req = new XMLHttpRequest();
    req.open("GET",filename,false);
    req.send();
    return req.responseText;
}

function loadCode()
{
    // Sync with Editor.m
    var javascriptFiles = ["../src/first.js", // must be first
                           "../src/ElementTypes.js", // must be second
                           "../src/AutoCorrect.js",
                           "../src/ChangeTracking.js",
                           "../src/Clipboard.js",
                           "../src/Cursor.js",
                           "../src/DOM.js",
                           "../src/Editor.js",
                           "../src/Equations.js",
                           "../src/Figures.js",
                           "../src/Formatting.js",
                           "../src/Hierarchy.js",
                           "../src/Input.js",
                           "../src/Lists.js",
                           "../src/Main.js",
                           "../src/Metadata.js",
                           "../src/NodeSet.js",
                           "../src/Outline.js",
                           "../src/Position.js",
                           "../src/PostponedActions.js",
                           "../src/Range.js",
                           "../src/Selection.js",
                           "../src/3rdparty/showdown/showdown.js",
                           "../src/Scan.js",
                           "../src/StringBuilder.js",
                           "../src/Styles.js",
                           "../src/Tables.js",
                           "../src/Text.js",
                           "../src/traversal.js",
                           "../src/types.js",
                           "../src/UndoManager.js",
                           "../src/util.js",
                           "../src/Viewport.js",
                           "PrettyPrinter.js", // only used for test harness
                           "testlib.js"]; // must be last
    var allCodeArray = new Array();
    for (var i = 0; i < javascriptFiles.length; i++)
        allCodeArray.push(readJSCode(javascriptFiles[i]));
    allCode = allCodeArray.join("\n");
}

function loadTestIndex()
{
    window.eval(readFile("index.js"));
}

function doPerformTest()
{
    var testDocument = leftArea.contentDocument;
    var w = leftArea.contentWindow;
    w.outputOptions = new Object();
    w.disableOutlineRedoHack = true;
    var resultText = w.performTest();
    if (!w.outputOptions.keepSelectionHighlights)
        w.Selection_clearSelection();
    if (resultText == null)
        resultText = w.PrettyPrinter.getHTML(testDocument.documentElement,w.outputOptions)
    var messages = JSON.parse(w.Editor_getBackMessages());
    for (var i = 0; i < messages.length; i++) {
        var message = messages[i];
        if (message[0] == "error")
            throw new Error(message[1]);
    }

    return resultText;
}

function showTest(dir,name)
{
    leftLoadedContinuation = function() {
        setLeftTitle("Working area");
        setRightTitle("Result");
        var resultText = doPerformTest();
        setPanelText(rightArea,resultText);
    }
    leftArea.src = dir+"/"+name+"-input.html";
}

function showResult(dirname,filename)
{
    var fullname = dirname+"-"+filename;
    setLeftTitle("Actual result for "+dirname+"/"+filename);
    setRightTitle("Expected result for "+dirname+"/"+filename);
    leftLoadedContinuation = null;
    setPanelText(leftArea,results[fullname].actual);
    setPanelText(rightArea,results[fullname].expected);
}

function setLeftTitle(title)
{
    document.getElementById("leftTitle").firstChild.nodeValue = title;
}

function setRightTitle(title)
{
    document.getElementById("rightTitle").firstChild.nodeValue = title;
}

function clearPanel(panel)
{
    panel.contentDocument.open();
    panel.contentDocument.close();
}

function setPanelText(panel,text)
{
    clearPanel(panel);
    var pre = panel.contentDocument.createElement("PRE");
    panel.contentDocument.body.appendChild(pre);
    pre.appendChild(panel.contentDocument.createTextNode(text));
}



function readJSCode(filename)
{
    var req = new XMLHttpRequest();
    req.open("GET",filename,false);
    req.send();
    return req.responseText;
}

function leftLoaded()
{
    if (leftLoadedContinuation == null)
        return;
    var continuation = leftLoadedContinuation;
    leftLoadedContinuation = null;

    var w = leftArea.contentWindow;
    w.eval(allCode);
    w.debug = function(str) { console.log(str); };

    w.testHarnessSetup();
    continuation();

    return;
}

function runAllTests()
{
    var dirno = 0;
    var fileno = 0;
    var haveTest = false;
    var dirname;
    var filename;

    var passes = 0;
    var failures = 0;
    var startTime = new Date();

    setLeftTitle("Working area");
    setRightTitle("");

    clearPanel(rightArea);
    results = new Object();

    runNextTest();
    return;

    function updateStatistics()
    {
        var statistics = document.getElementById("statistics");
        while (statistics.firstChild != null)
            statistics.removeChild(statistics.firstChild);
        var now = new Date();
        var elapsed = now - startTime;
        var str = "Passes: "+passes+", Failures: "+failures+
            ", Elapsed time "+(elapsed/1000)+" seconds";
        statistics.appendChild(document.createTextNode(str));
    }

    function runNextTest()
    {
        if (haveTest) {
            var expected = readFile(dirname+"/"+filename+"-expected.html");

            var actual;
            try {
                actual = doPerformTest();
            }
            catch (e) {
                actual = e.toString();
            }

            actual = actual.trim();
            expected = expected.trim();

            var fullname = dirname+"-"+filename;
            var resultElement = document.getElementById("result-"+fullname);
            while (resultElement.firstChild != null)
                resultElement.removeChild(resultElement);
            var a = document.createElement("a");
            a.href = "javascript:showResult('"+dirname+"','"+filename+"')";
            resultElement.appendChild(a);
            results[fullname] = new Result(actual,expected);
            if (actual == expected) {
                resultElement.setAttribute("class","pass");
                a.appendChild(document.createTextNode("PASS"));
                passes++;
            }
            else {
                resultElement.setAttribute("class","fail");
                a.appendChild(document.createTextNode("FAIL"));
                failures++;
            }
            updateStatistics();
        }
        if (dirno < tests.length) {
            var dir = tests[dirno];
            dirname = dir.dir;
            filename = dir.files[fileno];
            incrementPosition();
            leftLoadedContinuation = runNextTest;
            haveTest = true;
            leftArea.src = dirname+"/"+filename+"-input.html";
        }
    }

    function incrementPosition()
    {
        fileno++;
        if (fileno == tests[dirno].files.length) {
            dirno++;
            fileno = 0;
        }
    }
}

function loaded()
{
    topArea = document.getElementById("topInner");
    leftArea = document.getElementById("leftInner");
    rightArea = document.getElementById("rightInner");
    loadCode();
    loadTestIndex();

    var table = document.createElement("table");
    topArea.appendChild(table);

    for (var dirno = 0; dirno < tests.length; dirno++) {
        var dir = tests[dirno];

        var tr = document.createElement("tr");
        table.appendChild(tr);
        tr.setAttribute("class","dirrow");
        table.setAttribute("width","100%");

        var td = document.createElement("td");
        tr.appendChild(td);
        td.setAttribute("colspan","2");
        td.appendChild(document.createTextNode(dir.dir));

        for (var fileno = 0; fileno < dir.files.length; fileno++) {
            var filename = dir.files[fileno];

            tr = document.createElement("tr");
            table.appendChild(tr);
            tr.setAttribute("class","testrow");

            td = document.createElement("td");
            tr.appendChild(td);
            td.setAttribute("width","50%");

            var a = document.createElement("a");
            td.appendChild(a);
            a.href = "javascript:showTest('"+dir.dir+"','"+filename+"')";
            a.appendChild(document.createTextNode(filename));

            td = document.createElement("td");
            tr.appendChild(td);
            td.setAttribute("width","50%");
            td.id = "result-"+dir.dir+"-"+filename;
        }
    }
}
