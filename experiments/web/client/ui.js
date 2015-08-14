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

function Button(imagePrefix)
{
    var self = this;
    var mouseInside = false;

    this.element = document.createElement("div");
    this.element.style.display = "inline-block";
    this.element.style.width = "20px";
    this.element.style.height = "20px";
    this.element.style.padding = "10px";
    this.element.style.margin = "8px";
    this.element.style.borderWidth = "4px";
    this.element.style.borderRadius = "4px";
    this.element.addEventListener("mousedown",function() {
        if (self.onMouseDown != null)
            self.onMouseDown();
    });
    this.element.addEventListener("mouseover",function() {
        mouseInside = true;
        updateBackgroundColor();
    });
    this.element.addEventListener("mouseout",function() {
        mouseInside = false;
        updateBackgroundColor();
    });


    this.img = document.createElement("img");
    this.img.style.width = "20px";
    this.img.style.height = "20px";
    this.element.appendChild(this.img);


    this.value = false;

    this.setValue = function(newValue)
    {
        this.value = newValue;
        if (this.value)
            this.img.setAttribute("src",imagePrefix+"-on.png");
        else
            this.img.setAttribute("src",imagePrefix+".png");
        updateBackgroundColor();
    }

    function updateBackgroundColor()
    {
        var color = "transparent";
        if (mouseInside)
            color = "#ccc";
        else if (self.value)
            color = "#000";
        self.element.style.backgroundColor = color;
        self.element.style.borderColor = color;
    }

    this.setValue(false);
}
