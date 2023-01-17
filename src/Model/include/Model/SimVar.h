/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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

/*!
 * Constants related to simulation variable names.
 */
namespace SimVar
{
    // Simulation variables (SimConnect)
    constexpr const char *Latitude = "Plane Latitude";
    constexpr const char *Longitude = "Plane Longitude";
    constexpr const char *Altitude = "Plane Altitude";
    constexpr const char *IndicatedAltitude = "Indicated Altitude";
    constexpr const char *Pitch = "Plane Pitch Degrees";
    constexpr const char *Bank = "Plane Bank Degrees";
    constexpr const char *TrueHeading = "Plane Heading Degrees True";

    constexpr const char *VelocityBodyX = "Velocity Body X";
    constexpr const char *VelocityBodyY = "Velocity Body Y";
    constexpr const char *VelocityBodyZ = "Velocity Body Z";
    constexpr const char *RotationVelocityBodyX = "Rotation Velocity Body X";
    constexpr const char *RotationVelocityBodyY = "Rotation Velocity Body Y";
    constexpr const char *RotationVelocityBodyZ = "Rotation Velocity Body Z";

    constexpr const char *AileronLeftDeflection = "Aileron Left Deflection";
    constexpr const char *AileronRightDeflection = "Aileron Right Deflection";
    constexpr const char *ElevatorDeflection = "Elevator Deflection";
    constexpr const char *RudderDeflection = "Rudder Deflection";
    constexpr const char *RudderPosition = "Rudder Position";
    constexpr const char *ElevatorPosition = "Elevator Position";
    constexpr const char *AileronPosition = "Aileron Position";    

    constexpr const char *ThrottleLeverPosition1 = "General Eng Throttle Lever Position:1";
    constexpr const char *ThrottleLeverPosition2 = "General Eng Throttle Lever Position:2";
    constexpr const char *ThrottleLeverPosition3 = "General Eng Throttle Lever Position:3";
    constexpr const char *ThrottleLeverPosition4 = "General Eng Throttle Lever Position:4";
    constexpr const char *PropellerLeverPosition1 = "General Eng Propeller Lever Position:1";
    constexpr const char *PropellerLeverPosition2 = "General Eng Propeller Lever Position:2";
    constexpr const char *PropellerLeverPosition3 = "General Eng Propeller Lever Position:3";
    constexpr const char *PropellerLeverPosition4 = "General Eng Propeller Lever Position:4";
    constexpr const char *MixtureLeverPosition1 = "General Eng Mixture Lever Position:1";
    constexpr const char *MixtureLeverPosition2 = "General Eng Mixture Lever Position:2";
    constexpr const char *MixtureLeverPosition3 = "General Eng Mixture Lever Position:3";
    constexpr const char *MixtureLeverPosition4 = "General Eng Mixture Lever Position:4";
    constexpr const char *RecipEngineCowlFlapPosition1 = "Recip Eng Cowl Flap Position:1";
    constexpr const char *RecipEngineCowlFlapPosition2 = "Recip Eng Cowl Flap Position:2";
    constexpr const char *RecipEngineCowlFlapPosition3 = "Recip Eng Cowl Flap Position:3";
    constexpr const char *RecipEngineCowlFlapPosition4 = "Recip Eng Cowl Flap Position:4";
    constexpr const char *ElectricalMasterBattery1 = "Electrical Master Battery:1";
    constexpr const char *ElectricalMasterBattery2 = "Electrical Master Battery:2";
    constexpr const char *ElectricalMasterBattery3 = "Electrical Master Battery:3";
    constexpr const char *ElectricalMasterBattery4 = "Electrical Master Battery:4";
    constexpr const char *GeneralEngineStarter1 = "General Eng Starter:1";
    constexpr const char *GeneralEngineStarter2 = "General Eng Starter:2";
    constexpr const char *GeneralEngineStarter3 = "General Eng Starter:3";
    constexpr const char *GeneralEngineStarter4 = "General Eng Starter:4";
    constexpr const char *GeneralEngineCombustion1 = "General Eng Combustion:1";
    constexpr const char *GeneralEngineCombustion2 = "General Eng Combustion:2";
    constexpr const char *GeneralEngineCombustion3 = "General Eng Combustion:3";
    constexpr const char *GeneralEngineCombustion4 = "General Eng Combustion:4";

    constexpr const char *FlapsHandleIndex = "Flaps Handle Index";
    constexpr const char *LeadingEdgeFlapsLeftPercent = "Leading Edge Flaps Left Percent";
    constexpr const char *LeadingEdgeFlapsRightPercent = "Leading Edge Flaps Right Percent";
    constexpr const char *TrailingEdgeFlapsLeftPercent = "Trailing Edge Flaps Left Percent";
    constexpr const char *TrailingEdgeFlapsRightPercent = "Trailing Edge Flaps Right Percent";
    constexpr const char *SpoilersHandlePosition = "Spoilers Handle Position";
    constexpr const char *SpoilersArmed = "Spoilers Armed";
    constexpr const char *SpoilersLeftPosition = "Spoilers Left Position";
    constexpr const char *SpoilersRightPosition = "Spoilers Right Position";

    constexpr const char *BrakeLeftPosition = "Brake Left Position";
    constexpr const char *BrakeRightPosition = "Brake Right Position";
    constexpr const char *WaterRudderHandlePosition = "Water Rudder Handle Position";
    constexpr const char *TailhookHandle = "Tailhook Handle";
    constexpr const char *TailhookPosition = "Tailhook Position";
    constexpr const char *CanopyOpen = "Canopy Open";
    constexpr const char *GearHandlePosition = "Gear Handle Position";
    constexpr const char *SmokeEnable = "Smoke Enable";
    constexpr const char *FoldingWingHandlePosition = "Folding Wing Handle Position";
    constexpr const char *FoldingWingLeftPercent = "Folding Wing Left Percent";
    constexpr const char *FoldingWingRightPercent = "Folding Wing Right Percent";

    constexpr const char *LightStates = "Light States";
    constexpr const char *LightNav = "Light Nav";
    constexpr const char *LightBeacon = "Light Beacon";
    constexpr const char *LightLanding = "Light Landing";
    constexpr const char *LightTaxi = "Light Taxi";
    constexpr const char *LightStrobe = "Light Strobe";
    constexpr const char *LightPanel = "Light Panel";
    constexpr const char *LightRecognition = "Light Recognition";
    constexpr const char *LightWing = "Light Wing";
    constexpr const char *LightLogo = "Light Logo";
    constexpr const char *LightCabin = "Light Cabin";

    constexpr const char *Title = "Title";
    constexpr const char *ATCId = "ATC Id";
    constexpr const char *ATCAirline = "ATC Airline";
    constexpr const char *ATCFlightNumber = "ATC Flight Number";
    constexpr const char *Category = "Category";

    constexpr const char *SimOnGround = "Sim On Ground";
    constexpr const char *PlaneAltAboveGround = "Plane Alt Above Ground";
    constexpr const char *AirspeedTrue = "Airspeed True";
    constexpr const char *AirspeedIndicated = "Airspeed Indicated";
    constexpr const char *SurfaceType = "Surface Type";
    constexpr const char *WingSpan = "Wing Span";
    constexpr const char *NumberOfEngines = "Number Of Engines";
    constexpr const char *EngineType = "Engine Type";

    constexpr const char *GroundAltitude = "Ground Altitude";
    constexpr const char *AmbientTemperature = "Ambient Temperature";
    constexpr const char *TotalAirTemperature = "Total Air Temperature";
    constexpr const char *AmbientWindVelocity = "Ambient Wind Velocity";
    constexpr const char *AmbientWindDirection = "Ambient Wind Direction";
    constexpr const char *AmbientPrecipState = "Ambient Precip State";
    constexpr const char *AmbientInCloud = "Ambient In Cloud";
    constexpr const char *AmbientVisibility = "Ambient Visibility";
    constexpr const char *SeaLevelPressure = "Sea Level Pressure";
    constexpr const char *PitotIcePct = "Pitot Ice Pct";
    constexpr const char *StructuralIcePct = "Structural Ice Pct";

    constexpr const char *GpsWPNextId = "GPS WP Next Id";
    constexpr const char *GpsWPNextLat = "GPS WP Next Lat";
    constexpr const char *GpsWPNextLon = "GPS WP Next Lon";
    constexpr const char *GpsWPNextAlt = "GPS WP Next Alt";
    constexpr const char *GpsWPPrevId = "GPS WP Prev Id";
    constexpr const char *GpsWPPrevLat = "GPS WP Prev Lat";
    constexpr const char *GpsWPPrevLon = "GPS WP Prev Lon";
    constexpr const char *GpsWPPrevAlt = "GPS WP Prev Alt";

    constexpr const char *LocalTime = "Local Time";
    constexpr const char *LocalYear = "Local Year";
    constexpr const char *LocalMonthOfYear = "Local Month of Year";
    constexpr const char *LocalDayOfMonth = "Local Day of Month";

    constexpr const char *ZuluTime = "Zulu Time";
    constexpr const char *ZuluYear = "Zulu Year";
    constexpr const char *ZuluMonthOfYear = "Zulu Month of Year";
    constexpr const char *ZuluDayOfMonth = "Zulu Day of Month";

    constexpr const char *Timestamp = "Timestamp";
}

#endif // SIMVAR_H
