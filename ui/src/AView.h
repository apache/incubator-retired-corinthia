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

#pragma once

#include "ARect.h"
#include "AColor.h"
#include "AEvent.h"
#include "AShared.h"

class AView : public AShared
{
public:
    AView() : _visible(true),
              _enabled(true),
              _parent(NULL),
              _nextSibling(NULL),
              _prevSibling(NULL),
              _firstChild(NULL),
              _lastChild(NULL) { }
    AView(const ARect &frame) : _frame(frame) { }
    virtual ~AView() { }

    ARect frame() const { return _frame; }
    void setFrame(const ARect &newFrame) { _frame = newFrame; }

    AColor backgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const AColor &newBackgroundColor) { _backgroundColor = newBackgroundColor; }

    bool visible() const { return _visible; }
    void setVisible(bool newVisible) { _visible = newVisible; }

    bool enabled() const { return _enabled; }
    void setEnabled(bool newEnabled) { _enabled = newEnabled; }

private:
    ARect _frame;
    AColor _backgroundColor;
    bool _visible;
    bool _enabled;
    AView *_parent;
    AView *_nextSibling;
    AView *_prevSibling;
    AView *_firstChild;
    AView *_lastChild;
};
