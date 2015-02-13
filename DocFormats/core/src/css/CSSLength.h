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

#ifndef DocFormats_CSSLength_h
#define DocFormats_CSSLength_h

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                              Units                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    UnitsUnspecified = -1,
    UnitsIn = 0,
    UnitsCm = 1,
    UnitsMm = 2,
    UnitsPt = 3,
    UnitsPc = 4,
    UnitsPx = 5,
    UnitsPct = 6
} Units;

double convertBetweenUnits(double value, Units from, Units to);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSLength                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CSSLength CSSLength;

struct CSSLength {
    double value;
    Units units;
    int valueValid;
    int unitsValid;
};

extern CSSLength CSSLengthNull;

CSSLength CSSLengthFromValue(double value, Units units);
CSSLength CSSLengthFromString(const char *str);

int CSSLengthIsValid(CSSLength length);
int CSSLengthIsPercentage(CSSLength length);
int CSSLengthIsAbsolute(CSSLength length);

double CSSLengthToAbsolute(CSSLength length, double total, double valueUnits);
double CSSLengthToPts(CSSLength length, double total);

#endif
