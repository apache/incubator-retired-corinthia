// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
#pragma once
#include "qt_classes.hpp"


Toolbar::Toolbar() {
    saveButton.setText("Save");
    saveAsButton.setText("SaveAs");
    loadButton.setText("Load");
    this->setLayout(&layout);
    layout.addWidget(&saveButton);
    layout.addWidget(&saveAsButton);
    layout.addWidget(&loadButton);
    layout.setSpacing(4);
    layout.setContentsMargins(0, 0, 0, 0);
}
