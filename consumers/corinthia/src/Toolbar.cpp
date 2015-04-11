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

#include "Toolbar.h"
#include <QPushButton>
#include <QHBoxLayout>

Toolbar::Toolbar(QWidget *parent) : QWidget(parent)
{
    _tableButton = new QPushButton("Insert table",0);
    _linkButton = new QPushButton("Insert link",0);
    _characterButton = new QPushButton("Insert character",0);
    _backspaceButton = new QPushButton("Backspace",0);
    _leftButton = new QPushButton("Move left",0);
    _rightButton = new QPushButton("Move right",0);
    _undoButton = new QPushButton("Undo",0);
    _redoButton = new QPushButton("Redo",0);

    _layout = new QHBoxLayout();
    this->setLayout(_layout);

    _layout->addWidget(_tableButton);
    _layout->addWidget(_linkButton);
    _layout->addWidget(_characterButton);
    _layout->addWidget(_backspaceButton);
    _layout->addWidget(_leftButton);
    _layout->addWidget(_rightButton);
    _layout->addWidget(_undoButton);
    _layout->addWidget(_redoButton);
    _layout->setSpacing(4);
    _layout->setContentsMargins(0,0,0,0);
}

Toolbar::~Toolbar()
{
}
