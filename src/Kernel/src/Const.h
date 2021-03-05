/**
 * Sky Dolly - The black sheep for your flight recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef CONST_H
#define CONST_H

#include "KernelLib.h"

namespace Const
{
    constexpr char Sep = '\t';
    constexpr char Ln = '\n';

    // Simulation variables (SimConnect)
    KERNEL_API extern const char *Latitude;
    KERNEL_API extern const char *Longitude;
    KERNEL_API extern const char *Altitude;
    KERNEL_API extern const char *Pitch;
    KERNEL_API extern const char *Bank;
    KERNEL_API extern const char *Heading;

    KERNEL_API extern const char *YokeXPosition;
    KERNEL_API extern const char *YokeYPosition;
    KERNEL_API extern const char *RudderPosition;
    KERNEL_API extern const char *ElevatorPosition;
    KERNEL_API extern const char *AileronPosition;

    KERNEL_API extern const char *ThrottleLeverPosition1;
    KERNEL_API extern const char *ThrottleLeverPosition2;
    KERNEL_API extern const char *ThrottleLeverPosition3;
    KERNEL_API extern const char *ThrottleLeverPosition4;
    KERNEL_API extern const char *ThrottleLeverPosition4;
    KERNEL_API extern const char *PropellerLeverPosition1;
    KERNEL_API extern const char *PropellerLeverPosition2;
    KERNEL_API extern const char *PropellerLeverPosition3;
    KERNEL_API extern const char *PropellerLeverPosition4;
    KERNEL_API extern const char *MixtureLeverPosition1;
    KERNEL_API extern const char *MixtureLeverPosition2;
    KERNEL_API extern const char *MixtureLeverPosition3;
    KERNEL_API extern const char *MixtureLeverPosition4;

    KERNEL_API extern const char *LeadingEdgeFlapsLeftPercent;
    KERNEL_API extern const char *LeadingEdgeFlapsRightPercent;
    KERNEL_API extern const char *TrailingEdgeFlapsLeftPercent;
    KERNEL_API extern const char *TrailingEdgeFlapsRightPercent;
    KERNEL_API extern const char *SpoilersHandlePosition;
    KERNEL_API extern const char *FlapsHandleIndex;

    KERNEL_API extern const char *GearHandlePosition;
    KERNEL_API extern const char *WaterRudderHandlePosition;
    KERNEL_API extern const char *BrakeLeftPosition;
    KERNEL_API extern const char *BrakeRightPosition;

    extern const char *Timestamp;
}

#endif // CONST_H
