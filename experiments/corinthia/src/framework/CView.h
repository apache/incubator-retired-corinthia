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

#include "CRect.h"
#include "CColor.h"
#include "CEvent.h"
#include "CShared.h"

class CView : public CShared
{
public:
    CView() : _visible(true),
              _enabled(true),
              _parent(NULL),
              _nextSibling(NULL),
              _prevSibling(NULL),
              _firstChild(NULL),
              _lastChild(NULL) { }
    CView(const CRect &frame) : _frame(frame) { }
    virtual ~CView() { }

    CRect frame() const { return _frame; }
    void setFrame(const CRect &newFrame) { _frame = newFrame; }

    CColor backgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const CColor &newBackgroundColor) { _backgroundColor = newBackgroundColor; }

    bool visible() const { return _visible; }
    void setVisible(bool newVisible) { _visible = newVisible; }

    bool enabled() const { return _enabled; }
    void setEnabled(bool newEnabled) { _enabled = newEnabled; }

private:
    CRect _frame;
    CColor _backgroundColor;
    bool _visible;
    bool _enabled;
    CView *_parent;
    CView *_nextSibling;
    CView *_prevSibling;
    CView *_firstChild;
    CView *_lastChild;
};
