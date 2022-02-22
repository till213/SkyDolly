/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
    constexpr char Latitude[] = "Plane Latitude";
    constexpr char Longitude[] = "Plane Longitude";
    constexpr char Altitude[] = "Plane Altitude";
    constexpr char Pitch[] = "Plane Pitch Degrees";
    constexpr char Bank[] = "Plane Bank Degrees";
    constexpr char Heading[] = "Plane Heading Degrees True";

    constexpr char VelocityBodyX[] = "Velocity Body X";
    constexpr char VelocityBodyY[] = "Velocity Body Y";
    constexpr char VelocityBodyZ[] = "Velocity Body Z";
    constexpr char RotationVelocityBodyX[] = "Rotation Velocity Body X";
    constexpr char RotationVelocityBodyY[] = "Rotation Velocity Body Y";
    constexpr char RotationVelocityBodyZ[] = "Rotation Velocity Body Z";

    constexpr char RudderPosition[] = "Rudder Position";
    constexpr char ElevatorPosition[] = "Elevator Position";
    constexpr char AileronPosition[] = "Aileron Position";

    constexpr char ThrottleLeverPosition1[] = "General Eng Throttle Lever Position:1";
    constexpr char ThrottleLeverPosition2[] = "General Eng Throttle Lever Position:2";
    constexpr char ThrottleLeverPosition3[] = "General Eng Throttle Lever Position:3";
    constexpr char ThrottleLeverPosition4[] = "General Eng Throttle Lever Position:4";
    constexpr char PropellerLeverPosition1[] = "General Eng Propeller Lever Position:1";
    constexpr char PropellerLeverPosition2[] = "General Eng Propeller Lever Position:2";
    constexpr char PropellerLeverPosition3[] = "General Eng Propeller Lever Position:3";
    constexpr char PropellerLeverPosition4[] = "General Eng Propeller Lever Position:4";
    constexpr char MixtureLeverPosition1[] = "General Eng Mixture Lever Position:1";
    constexpr char MixtureLeverPosition2[] = "General Eng Mixture Lever Position:2";
    constexpr char MixtureLeverPosition3[] = "General Eng Mixture Lever Position:3";
    constexpr char MixtureLeverPosition4[] = "General Eng Mixture Lever Position:4";
    constexpr char RecipEngineCowlFlapPosition1[] = "Recip Eng Cowl Flap Position:1";
    constexpr char RecipEngineCowlFlapPosition2[] = "Recip Eng Cowl Flap Position:2";
    constexpr char RecipEngineCowlFlapPosition3[] = "Recip Eng Cowl Flap Position:3";
    constexpr char RecipEngineCowlFlapPosition4[] = "Recip Eng Cowl Flap Position:4";
    constexpr char ElectricalMasterBattery1[] = "Electrical Master Battery:1";
    constexpr char ElectricalMasterBattery2[] = "Electrical Master Battery:2";
    constexpr char ElectricalMasterBattery3[] = "Electrical Master Battery:3";
    constexpr char ElectricalMasterBattery4[] = "Electrical Master Battery:4";
    constexpr char GeneralEngineStarter1[] = "General Eng Starter:1";
    constexpr char GeneralEngineStarter2[] = "General Eng Starter:2";
    constexpr char GeneralEngineStarter3[] = "General Eng Starter:3";
    constexpr char GeneralEngineStarter4[] = "General Eng Starter:4";
    constexpr char GeneralEngineCombustion1[] = "General Eng Combustion:1";
    constexpr char GeneralEngineCombustion2[] = "General Eng Combustion:2";
    constexpr char GeneralEngineCombustion3[] = "General Eng Combustion:3";
    constexpr char GeneralEngineCombustion4[] = "General Eng Combustion:4";

    constexpr char LeadingEdgeFlapsLeftPercent[] = "Leading Edge Flaps Left Percent";
    constexpr char LeadingEdgeFlapsRightPercent[] = "Leading Edge Flaps Right Percent";
    constexpr char TrailingEdgeFlapsLeftPercent[] = "Trailing Edge Flaps Left Percent";
    constexpr char TrailingEdgeFlapsRightPercent[] = "Trailing Edge Flaps Right Percent";
    constexpr char SpoilersHandlePosition[] = "Spoilers Handle Position";
    constexpr char FlapsHandleIndex[] = "Flaps Handle Index";

    constexpr char BrakeLeftPosition[] = "Brake Left Position";
    constexpr char BrakeRightPosition[] = "Brake Right Position";
    constexpr char WaterRudderHandlePosition[] = "Water Rudder Handle Position";
    constexpr char TailhookPosition[] = "Tailhook Position";
    constexpr char CanopyOpen[] = "Canopy Open";
    constexpr char GearHandlePosition[] = "Gear Handle Position";
    constexpr char SmokeEnable[] = "Smoke Enable";
    constexpr char FoldingWingLeftPercent[] = "Folding Wing Left Percent";
    constexpr char FoldingWingRightPercent[] = "Folding Wing Right Percent";

    constexpr char LightStates[] = "Light States";

    constexpr char Title[] = "Title";
    constexpr char ATCId[] = "ATC Id";
    constexpr char ATCAirline[] = "ATC Airline";
    constexpr char ATCFlightNumber[] = "ATC Flight Number";
    constexpr char Category[] = "Category";

    constexpr char SimOnGround[] = "Sim On Ground";
    constexpr char PlaneAltAboveGround[] = "Plane Alt Above Ground";
    constexpr char AirspeedTrue[] = "Airspeed True";
    constexpr char SurfaceType[] = "Surface Type";
    constexpr char WingSpan[] = "Wing Span";
    constexpr char NumberOfEngines[] = "Number Of Engines";
    constexpr char EngineType[] = "Engine Type";

    constexpr char GroundAltitude[] = "Ground Altitude";
    constexpr char AmbientTemperature[] = "Ambient Temperature";
    constexpr char TotalAirTemperature[] = "Total Air Temperature";
    constexpr char AmbientWindVelocity[] = "Ambient Wind Velocity";
    constexpr char AmbientWindDirection[] = "Ambient Wind Direction";
    constexpr char AmbientPrecipState[] = "Ambient Precip State";
    constexpr char AmbientInCloud[] = "Ambient In Cloud";
    constexpr char AmbientVisibility[] = "Ambient Visibility";
    constexpr char SeaLevelPressure[] = "Sea Level Pressure";
    constexpr char PitotIcePct[] = "Pitot Ice Pct";
    constexpr char StructuralIcePct[] = "Structural Ice Pct";

    constexpr char GpsWPNextId[] = "GPS WP Next Id";
    constexpr char GpsWPNextLat[] = "GPS WP Next Lat";
    constexpr char GpsWPNextLon[] = "GPS WP Next Lon";
    constexpr char GpsWPNextAlt[] = "GPS WP Next Alt";
    constexpr char GpsWPPrevId[] = "GPS WP Prev Id";
    constexpr char GpsWPPrevLat[] = "GPS WP Prev Lat";
    constexpr char GpsWPPrevLon[] = "GPS WP Prev Lon";
    constexpr char GpsWPPrevAlt[] = "GPS WP Prev Alt";

    constexpr char LocalTime[] = "Local Time";
    constexpr char LocalYear[] = "Local Year";
    constexpr char LocalMonthOfYear[] = "Local Month of Year";
    constexpr char LocalDayOfMonth[] = "Local Day of Month";

    constexpr char ZuluTime[] = "Zulu Time";
    constexpr char ZuluYear[] = "Zulu Year";
    constexpr char ZuluMonthOfYear[] = "Zulu Month of Year";
    constexpr char ZuluDayOfMonth[] = "Zulu Day of Month";

    constexpr char Timestamp[] = "Timestamp";
}

#endif // SIMVAR_H
