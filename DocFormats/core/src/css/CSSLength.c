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

#include "CSSLength.h"
#include "DFCommon.h"
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                              Units                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define POINTS_PER_PC 12
#define POINTS_PER_PX 0.75
#define POINTS_PER_IN 72
#define POINTS_PER_CM (POINTS_PER_IN/2.54)
#define POINTS_PER_MM (POINTS_PER_CM/10)

double convertBetweenUnits(double value, Units from, Units to)
{
    if (from == to)
        return value;
    double valueInPoints = 0;
    switch (from) {
        case UnitsIn: valueInPoints = value * POINTS_PER_IN; break;
        case UnitsCm: valueInPoints = value * POINTS_PER_CM; break;
        case UnitsMm: valueInPoints = value * POINTS_PER_MM; break;
        case UnitsPt: valueInPoints = value; break;
        case UnitsPc: valueInPoints = value * POINTS_PER_PC; break;
        case UnitsPx: valueInPoints = value * POINTS_PER_PX; break;
        default: return 1;
    }
    switch (to) {
        case UnitsIn: return valueInPoints / POINTS_PER_IN;
        case UnitsCm: return valueInPoints / POINTS_PER_CM;
        case UnitsMm: return valueInPoints / POINTS_PER_MM;
        case UnitsPt: return valueInPoints;
        case UnitsPc: return valueInPoints / POINTS_PER_PC;
        case UnitsPx: return valueInPoints / POINTS_PER_PX;
        default: return 1;
    }
}

static Units stringToUnits(const char *str)
{
    if (str == NULL)
        return UnitsUnspecified;
    else if (!strcmp(str,"%"))
        return UnitsPct;
    else if (!strcmp(str,"in"))
        return UnitsIn;
    else if (!strcmp(str,"cm"))
        return UnitsCm;
    else if (!strcmp(str,"mm"))
        return UnitsMm;
    else if (!strcmp(str,"pt"))
        return UnitsPt;
    else if (!strcmp(str,"pc"))
        return UnitsPc;
    else if (!strcmp(str,"px"))
        return UnitsPx;
    else
        return UnitsUnspecified;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSLength                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

CSSLength CSSLengthNull = { 0, UnitsUnspecified, 0, 0 };

CSSLength CSSLengthFromValue(double value, Units units)
{
    CSSLength length;

    length.value = value;
    length.units = units;

    length.valueValid = 1;
    length.unitsValid = (units != UnitsUnspecified);

    return length;
}

CSSLength CSSLengthFromString(const char *str)
{
    if (str == NULL)
        return CSSLengthNull;

    double svalue;
    char sunits[11];

    if (sscanf(str,"%lf%10s",&svalue,sunits) != 2)
        return CSSLengthNull;;

    CSSLength length;
    length.valueValid = 1;
    length.value = svalue;
    length.units = stringToUnits(sunits);
    length.unitsValid = (length.units != UnitsUnspecified);
    return length;
}

int CSSLengthIsValid(CSSLength length)
{
    return (length.valueValid && length.unitsValid);
}

int CSSLengthIsPercentage(CSSLength length)
{
    return (CSSLengthIsValid(length) && (length.units == UnitsPct));
}

int CSSLengthIsAbsolute(CSSLength length)
{
    if (!CSSLengthIsValid(length))
        return 0;
    switch (length.units) {
        case UnitsIn:
        case UnitsCm:
        case UnitsMm:
        case UnitsPt:
        case UnitsPc:
        case UnitsPx:
            return 1;
        default:
            return 0;
    }
}

double CSSLengthToAbsolute(CSSLength length, double total, double valueUnits)
{
    switch (length.units) {
        case UnitsIn:
        case UnitsCm:
        case UnitsMm:
        case UnitsPt:
        case UnitsPc:
        case UnitsPx:
            return convertBetweenUnits(length.value,length.units,valueUnits);
        case UnitsPct:
            return (length.value/100.0)*total;
        default:
            return 0;
    }
}

double CSSLengthToPts(CSSLength length, double total)
{
    return CSSLengthToAbsolute(length,total,UnitsPt);
}
