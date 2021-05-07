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
    inline constexpr char Latitude[] = "Plane Latitude";
    inline constexpr char Longitude[] = "Plane Longitude";
    inline constexpr char Altitude[] = "Plane Altitude";
    inline constexpr char Pitch[] = "Plane Pitch Degrees";
    inline constexpr char Bank[] = "Plane Bank Degrees";
    inline constexpr char Heading[] = "Plane Heading Degrees True";

    inline constexpr char VelocityBodyX[] = "Velocity Body X";
    inline constexpr char VelocityBodyY[] = "Velocity Body Y";
    inline constexpr char VelocityBodyZ[] = "Velocity Body Z";
    inline constexpr char RotationVelocityBodyX[] = "Rotation Velocity Body X";
    inline constexpr char RotationVelocityBodyY[] = "Rotation Velocity Body Y";
    inline constexpr char RotationVelocityBodyZ[] = "Rotation Velocity Body Z";

    inline constexpr char RudderPosition[] = "Rudder Position";
    inline constexpr char ElevatorPosition[] = "Elevator Position";
    inline constexpr char AileronPosition[] = "Aileron Position";

    inline constexpr char ThrottleLeverPosition1[] = "General Eng Throttle Lever Position:1";
    inline constexpr char ThrottleLeverPosition2[] = "General Eng Throttle Lever Position:2";
    inline constexpr char ThrottleLeverPosition3[] = "General Eng Throttle Lever Position:3";
    inline constexpr char ThrottleLeverPosition4[] = "General Eng Throttle Lever Position:4";
    inline constexpr char PropellerLeverPosition1[] = "General Eng Propeller Lever Position:1";
    inline constexpr char PropellerLeverPosition2[] = "General Eng Propeller Lever Position:2";
    inline constexpr char PropellerLeverPosition3[] = "General Eng Propeller Lever Position:3";
    inline constexpr char PropellerLeverPosition4[] = "General Eng Propeller Lever Position:4";
    inline constexpr char MixtureLeverPosition1[] = "General Eng Mixture Lever Position:1";
    inline constexpr char MixtureLeverPosition2[] = "General Eng Mixture Lever Position:2";
    inline constexpr char MixtureLeverPosition3[] = "General Eng Mixture Lever Position:3";
    inline constexpr char MixtureLeverPosition4[] = "General Eng Mixture Lever Position:4";
    inline constexpr char RecipEngineCowlFlapPosition1[] = "Recip Eng Cowl Flap Position:1";
    inline constexpr char RecipEngineCowlFlapPosition2[] = "Recip Eng Cowl Flap Position:2";
    inline constexpr char RecipEngineCowlFlapPosition3[] = "Recip Eng Cowl Flap Position:3";
    inline constexpr char RecipEngineCowlFlapPosition4[] = "Recip Eng Cowl Flap Position:4";
    inline constexpr char ElectricalMasterBattery1[] = "Electrical Master Battery:1";
    inline constexpr char ElectricalMasterBattery2[] = "Electrical Master Battery:2";
    inline constexpr char ElectricalMasterBattery3[] = "Electrical Master Battery:3";
    inline constexpr char ElectricalMasterBattery4[] = "Electrical Master Battery:4";
    inline constexpr char GeneralEngineStarter1[] = "General Eng Starter:1";
    inline constexpr char GeneralEngineStarter2[] = "General Eng Starter:2";
    inline constexpr char GeneralEngineStarter3[] = "General Eng Starter:3";
    inline constexpr char GeneralEngineStarter4[] = "General Eng Starter:4";

    inline constexpr char LeadingEdgeFlapsLeftPercent[] = "Leading Edge Flaps Left Percent";
    inline constexpr char LeadingEdgeFlapsRightPercent[] = "Leading Edge Flaps Right Percent";
    inline constexpr char TrailingEdgeFlapsLeftPercent[] = "Trailing Edge Flaps Left Percent";
    inline constexpr char TrailingEdgeFlapsRightPercent[] = "Trailing Edge Flaps Right Percent";
    inline constexpr char SpoilersHandlePosition[] = "Spoilers Handle Position";
    inline constexpr char FlapsHandleIndex[] = "Flaps Handle Index";

    inline constexpr char BrakeLeftPosition[] = "Brake Left Position";
    inline constexpr char BrakeRightPosition[] = "Brake Right Position";
    inline constexpr char WaterRudderHandlePosition[] = "Water Rudder Handle Position";
    inline constexpr char TailhookPosition[] = "Tailhook Position";
    inline constexpr char CanopyOpen[] = "Canopy Open";
    inline constexpr char GearHandlePosition[] = "Gear Handle Position";
    inline constexpr char FoldingWingLeftPercent[] = "Folding Wing Left Percent";
    inline constexpr char FoldingWingRightPercent[] = "Folding Wing Right Percent";

    inline constexpr char LightStates[] = "Light States";

    inline constexpr char Title[] = "Title";
    inline constexpr char ATCId[] = "ATC Id";
    inline constexpr char ATCAirline[] = "ATC Airline";
    inline constexpr char ATCFlightNumber[] = "ATC Flight Number";
    inline constexpr char Category[] = "Category";

    inline constexpr char SimOnGround[] = "Sim On Ground";
    inline constexpr char PlaneAltAboveGround[] = "Plane Alt Above Ground";
    inline constexpr char AirspeedTrue[] = "Airspeed True";
    inline constexpr char SurfaceType[] = "Surface Type";
    inline constexpr char WingSpan[] = "Wing Span";
    inline constexpr char NumberOfEngines[] = "Number Of Engines";
    inline constexpr char EngineType[] = "Engine Type";

    inline constexpr char GroundAltitude[] = "Ground Altitude";
    inline constexpr char AmbientTemperature[] = "Ambient Temperature";
    inline constexpr char TotalAirTemperature[] = "Total Air Temperature";
    inline constexpr char AmbientWindVelocity[] = "Ambient Wind Velocity";
    inline constexpr char AmbientWindDirection[] = "Ambient Wind Direction";
    inline constexpr char AmbientPrecipState[] = "Ambient Precip State";
    inline constexpr char AmbientInCloud[] = "Ambient In Cloud";
    inline constexpr char AmbientVisibility[] = "Ambient Visibility";
    inline constexpr char SeaLevelPressure[] = "Sea Level Pressure";
    inline constexpr char PitotIcePct[] = "Pitot Ice Pct";
    inline constexpr char StructuralIcePct[] = "Structural Ice Pct";

    inline constexpr char GpsWPNextId[] = "GPS WP Next Id";
    inline constexpr char GpsWPNextLat[] = "GPS WP Next Lat";
    inline constexpr char GpsWPNextLon[] = "GPS WP Next Lon";
    inline constexpr char GpsWPNextAlt[] = "GPS WP Next Alt";
    inline constexpr char GpsWPPrevId[] = "GPS WP Prev Id";
    inline constexpr char GpsWPPrevLat[] = "GPS WP Prev Lat";
    inline constexpr char GpsWPPrevLon[] = "GPS WP Prev Lon";
    inline constexpr char GpsWPPrevAlt[] = "GPS WP Prev Alt";

    inline constexpr char LocalTime[] = "Local Time";
    inline constexpr char LocalYear[] = "Local Year";
    inline constexpr char LocalMonthOfYear[] = "Local Month of Year";
    inline constexpr char LocalDayOfMonth[] = "Local Day of Month";

    inline constexpr char ZuluTime[] = "Zulu Time";
    inline constexpr char ZuluYear[] = "Zulu Year";
    inline constexpr char ZuluMonthOfYear[] = "Zulu Month of Year";
    inline constexpr char ZuluDayOfMonth[] = "Zulu Day of Month";

    inline constexpr char Timestamp[] = "Timestamp";
}

#endif // SIMVAR_H
