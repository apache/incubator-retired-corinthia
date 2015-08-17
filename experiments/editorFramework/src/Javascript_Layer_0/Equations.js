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

var Equations_insertEquation;

(function() {

    Equations_insertEquation = function()
    {
        var math = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","math");
        var mrow = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mrow");
        var msup = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","msup");
        var mi = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mi");
        var mn = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mn");
        var mfrac = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mfrac");
        var mrow1 = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mrow");
        var mrow2 = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mrow");
        var mi1 = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mi");
        var mi2 = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mi");
        var mo = DOM_createElementNS(document,"http://www.w3.org/1998/Math/MathML","mo");

        DOM_appendChild(mi,DOM_createTextNode(document,"x"));
        DOM_appendChild(mn,DOM_createTextNode(document,"2"));
        DOM_appendChild(mo,DOM_createTextNode(document,"+"));
        DOM_appendChild(mi1,DOM_createTextNode(document,"a"));
        DOM_appendChild(mi2,DOM_createTextNode(document,"b"));
        DOM_appendChild(mrow1,mi1);
        DOM_appendChild(mrow2,mi2);
        DOM_appendChild(mfrac,mrow1);
        DOM_appendChild(mfrac,mrow2);
        DOM_appendChild(msup,mi);
        DOM_appendChild(msup,mn);
        DOM_appendChild(mrow,msup);
        DOM_appendChild(mrow,mo);
        DOM_appendChild(mrow,mfrac);
        DOM_appendChild(math,mrow);

        Clipboard_pasteNodes([math]);
    }

})();
