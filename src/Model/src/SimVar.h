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
#ifndef SIMVAR_H
#define SIMVAR_H

#include "ModelLib.h"

/*!
 * Constants related to simulation variable names.
 */
namespace SimVar
{
    // Simulation variables (SimConnect)
    MODEL_API extern const char *Latitude;
    MODEL_API extern const char *Longitude;
    MODEL_API extern const char *Altitude;
    MODEL_API extern const char *Pitch;
    MODEL_API extern const char *Bank;
    MODEL_API extern const char *Heading;

    MODEL_API extern const char *VelocityBodyX;
    MODEL_API extern const char *VelocityBodyY;
    MODEL_API extern const char *VelocityBodyZ;
    MODEL_API extern const char *RotationVelocityBodyX;
    MODEL_API extern const char *RotationVelocityBodyY;
    MODEL_API extern const char *RotationVelocityBodyZ;

    MODEL_API extern const char *RudderPosition;
    MODEL_API extern const char *ElevatorPosition;
    MODEL_API extern const char *AileronPosition;

    MODEL_API extern const char *ThrottleLeverPosition1;
    MODEL_API extern const char *ThrottleLeverPosition2;
    MODEL_API extern const char *ThrottleLeverPosition3;
    MODEL_API extern const char *ThrottleLeverPosition4;
    MODEL_API extern const char *PropellerLeverPosition1;
    MODEL_API extern const char *PropellerLeverPosition2;
    MODEL_API extern const char *PropellerLeverPosition3;
    MODEL_API extern const char *PropellerLeverPosition4;
    MODEL_API extern const char *MixtureLeverPosition1;
    MODEL_API extern const char *MixtureLeverPosition2;
    MODEL_API extern const char *MixtureLeverPosition3;
    MODEL_API extern const char *MixtureLeverPosition4;
    MODEL_API extern const char *RecipEngineCowlFlapPosition1;
    MODEL_API extern const char *RecipEngineCowlFlapPosition2;
    MODEL_API extern const char *RecipEngineCowlFlapPosition3;
    MODEL_API extern const char *RecipEngineCowlFlapPosition4;
    MODEL_API extern const char *ElectricalMasterBattery1;
    MODEL_API extern const char *ElectricalMasterBattery2;
    MODEL_API extern const char *ElectricalMasterBattery3;
    MODEL_API extern const char *ElectricalMasterBattery4;
    MODEL_API extern const char *GeneralEngineStarter1;
    MODEL_API extern const char *GeneralEngineStarter2;
    MODEL_API extern const char *GeneralEngineStarter3;
    MODEL_API extern const char *GeneralEngineStarter4;

    MODEL_API extern const char *LeadingEdgeFlapsLeftPercent;
    MODEL_API extern const char *LeadingEdgeFlapsRightPercent;
    MODEL_API extern const char *TrailingEdgeFlapsLeftPercent;
    MODEL_API extern const char *TrailingEdgeFlapsRightPercent;
    MODEL_API extern const char *SpoilersHandlePosition;
    MODEL_API extern const char *FlapsHandleIndex;

    MODEL_API extern const char *BrakeLeftPosition;
    MODEL_API extern const char *BrakeRightPosition;
    MODEL_API extern const char *WaterRudderHandlePosition;
    MODEL_API extern const char *TailhookPosition;
    MODEL_API extern const char *CanopyOpen;
    MODEL_API extern const char *GearHandlePosition;
    MODEL_API extern const char *FoldingWingHandlePosition;

    MODEL_API extern const char *LightStates;

    // Information simulation variables (SimConnect)
    MODEL_API extern const char *Title;
    MODEL_API extern const char *ATCId;
    MODEL_API extern const char *ATCAirline;
    MODEL_API extern const char *ATCFlightNumber;
    MODEL_API extern const char *Category;

    MODEL_API extern const char *SimOnGround;
    MODEL_API extern const char *PlaneAltAboveGround;
    MODEL_API extern const char *AirspeedTrue;
    MODEL_API extern const char *SurfaceType;
    MODEL_API extern const char *WingSpan;
    MODEL_API extern const char *NumberOfEngines;
    MODEL_API extern const char *EngineType;

    MODEL_API extern const char *GroundAltitude;
    MODEL_API extern const char *AmbientTemperature;
    MODEL_API extern const char *TotalAirTemperature;
    MODEL_API extern const char *AmbientWindVelocity;
    MODEL_API extern const char *AmbientWindDirection;
    MODEL_API extern const char *AmbientPrecipState;
    MODEL_API extern const char *AmbientInCloud;
    MODEL_API extern const char *AmbientVisibility;
    MODEL_API extern const char *SeaLevelPressure;
    MODEL_API extern const char *PitotIcePct;
    MODEL_API extern const char *StructuralIcePct;

    MODEL_API extern const char *GpsWPNextId;
    MODEL_API extern const char *GpsWPNextLat;
    MODEL_API extern const char *GpsWPNextLon;
    MODEL_API extern const char *GpsWPNextAlt;
    MODEL_API extern const char *GpsWPPrevId;
    MODEL_API extern const char *GpsWPPrevLat;
    MODEL_API extern const char *GpsWPPrevLon;
    MODEL_API extern const char *GpsWPPrevAlt;

    inline constexpr char LocalTime[] = "Local Time";
    inline constexpr char LocalYear[] = "Local Year";
    inline constexpr char LocalMonthOfYear[] = "Local Month of Year";
    inline constexpr char LocalDayOfMonth[] = "Local Day of Month";

    inline constexpr char ZuluTime[] = "Zulu Time";
    inline constexpr char ZuluYear[] = "Zulu Year";
    inline constexpr char ZuluMonthOfYear[] = "Zulu Month of Year";
    inline constexpr char ZuluDayOfMonth[] = "Zulu Day of Month";

    MODEL_API extern const char *Timestamp;
}

#endif // SIMVAR_H
