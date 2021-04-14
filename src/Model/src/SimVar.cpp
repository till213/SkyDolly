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
#include "SimVar.h"

const char *SimVar::Latitude = "Plane Latitude";
const char *SimVar::Longitude = "Plane Longitude";
const char *SimVar::Altitude = "Plane Altitude";
const char *SimVar::Pitch = "Plane Pitch Degrees";
const char *SimVar::Bank = "Plane Bank Degrees";
const char *SimVar::Heading = "Plane Heading Degrees True";

const char *SimVar::VelocityBodyX = "Velocity Body X";
const char *SimVar::VelocityBodyY = "Velocity Body Y";
const char *SimVar::VelocityBodyZ = "Velocity Body Z";
const char *SimVar::RotationVelocityBodyX = "Rotation Velocity Body X";
const char *SimVar::RotationVelocityBodyY = "Rotation Velocity Body Y";
const char *SimVar::RotationVelocityBodyZ = "Rotation Velocity Body Z";

const char *SimVar::AccelerationBodyX = "Acceleration Body X";
const char *SimVar::AccelerationBodyY = "Acceleration Body Y";
const char *SimVar::AccelerationBodyZ = "Acceleration Body Z";
const char *SimVar::RotationAccelerationBodyX = "Rotation Acceleration Body X";
const char *SimVar::RotationAccelerationBodyY = "Rotation Acceleration Body Y";
const char *SimVar::RotationAccelerationBodyZ = "Rotation Acceleration Body Z";

const char *SimVar::YokeXPosition = "Yoke Y Position";
const char *SimVar::YokeYPosition = "Yoke Y Position";
const char *SimVar::RudderPosition = "Rudder Position";
const char *SimVar::ElevatorPosition = "Elevator Position";
const char *SimVar::AileronPosition = "Aileron Position";

const char *SimVar::ThrottleLeverPosition1 = "General Eng Throttle Lever Position:1";
const char *SimVar::ThrottleLeverPosition2 = "General Eng Throttle Lever Position:2";
const char *SimVar::ThrottleLeverPosition3 = "General Eng Throttle Lever Position:3";
const char *SimVar::ThrottleLeverPosition4 = "General Eng Throttle Lever Position:4";
const char *SimVar::PropellerLeverPosition1 = "General Eng Propeller Lever Position:1";
const char *SimVar::PropellerLeverPosition2 = "General Eng Propeller Lever Position:2";
const char *SimVar::PropellerLeverPosition3 = "General Eng Propeller Lever Position:3";
const char *SimVar::PropellerLeverPosition4 = "General Eng Propeller Lever Position:4";
const char *SimVar::MixtureLeverPosition1 = "General Eng Mixture Lever Position:1";
const char *SimVar::MixtureLeverPosition2 = "General Eng Mixture Lever Position:2";
const char *SimVar::MixtureLeverPosition3 = "General Eng Mixture Lever Position:3";
const char *SimVar::MixtureLeverPosition4 = "General Eng Mixture Lever Position:4";
const char *SimVar::RecipEngineCowlFlapPosition1 = "Recip Eng Cowl Flap Position:1";
const char *SimVar::RecipEngineCowlFlapPosition2 = "Recip Eng Cowl Flap Position:2";
const char *SimVar::RecipEngineCowlFlapPosition3 = "Recip Eng Cowl Flap Position:3";
const char *SimVar::RecipEngineCowlFlapPosition4 = "Recip Eng Cowl Flap Position:4";

const char *SimVar::LeadingEdgeFlapsLeftPercent = "Leading Edge Flaps Left Percent";
const char *SimVar::LeadingEdgeFlapsRightPercent = "Leading Edge Flaps Right Percent";
const char *SimVar::TrailingEdgeFlapsLeftPercent = "Trailing Edge Flaps Left Percent";
const char *SimVar::TrailingEdgeFlapsRightPercent = "Trailing Edge Flaps Right Percent";
const char *SimVar::SpoilersHandlePosition = "Spoilers Handle Position";
const char *SimVar::FlapsHandleIndex = "Flaps Handle Index";

const char *SimVar::GearHandlePosition = "Gear Handle Position";
const char *SimVar::BrakeLeftPosition = "Brake Left Position";
const char *SimVar::BrakeRightPosition = "Brake Right Position";
const char *SimVar::WaterRudderHandlePosition = "Water Rudder Handle Position";
const char *SimVar::TailhookPosition = "Tailhook Position";
const char *SimVar::CanopyOpen = "Canopy Open";

const char *SimVar::LightStates = "Light States";

const char *SimVar::Title= "Title";
const char *SimVar::ATCId= "ATC Id";
const char *SimVar::ATCAirline= "ATC Airline";
const char *SimVar::ATCFlightNumber= "ATC Flight Number";
const char *SimVar::Category= "Category";

const char *SimVar::SimOnGround= "Sim On Ground";
const char *SimVar::PlaneAltAboveGround= "Plane Alt Above Ground";
const char *SimVar::AirspeedTrue= "Airspeed True";
const char *SimVar::SurfaceType= "Surface Type";
const char *SimVar::WingSpan= "Wing Span";
const char *SimVar::NumberOfEngines= "Number Of Engines";
const char *SimVar::EngineType= "Engine Type";

const char *SimVar::GroundAltitude= "Ground Altitude";
const char *SimVar::AmbientTemperature= "Ambient Temperature";
const char *SimVar::TotalAirTemperature= "Total Air Temperature";
const char *SimVar::AmbientWindVelocity= "Ambient Wind Velocity";
const char *SimVar::AmbientWindDirection= "Ambient Wind Direction";
const char *SimVar::AmbientPrecipState= "Ambient Precip State";
const char *SimVar::AmbientInCloud= "Ambient In Cloud";
const char *SimVar::AmbientVisibility= "Ambient Visibility";
const char *SimVar::SeaLevelPressure= "Sea Level Pressure";
const char *SimVar::PitotIcePct= "Pitot Ice Pct";
const char *SimVar::StructuralIcePct= "Structural Ice Pct";

const char *SimVar::Timestamp = "Timestamp";
