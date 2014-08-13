//
//  CSSLength.h
//  DocFormats
//
//  Created by Peter Kelly on 18/04/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

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
