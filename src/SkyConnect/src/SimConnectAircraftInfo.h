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
#ifndef SIMCONNECTAIRCRAFTINFO_H
#define SIMCONNECTAIRCRAFTINFO_H

#include <windows.h>
#include <strsafe.h>

#include <QtGlobal>

#include "../../Kernel/src/SkyMath.h"
#include "../../Kernel/src/SimTypes.h"
#include "../../Kernel/src/AircraftInfo.h"

/*!
 * Information simulation variables about the aircraft and environment at start.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftInfo
{
    char title[256];
    char atcId[32];
    char atcAirline[64];
    char atcFlightNumber[8];
    char category[256];

    qint32 simOnGround;
    // Feet
    float planeAltAboveGround;
    // Knots
    qint32 airspeedTrue;
    qint32 surfaceType;
    // Feet
    qint32 wingSpan;
    qint32 numberOfEngines;
    qint32 engineType;

    float groundAltitude;
    // Celcius
    float ambientTemperature;
    float totalAirTemperature;
    float ambientWindVelocity;
    float ambientWindDirection;
    qint32 ambientPrecipState;
    qint32 ambientInCloud;
    float ambientVisibility;
    float seaLevelPressure;
    float pitotIcePct;
    float structuralIcePct;

    SimConnectAircraftInfo()
        : simOnGround(false),
          planeAltAboveGround(0.0f),
          airspeedTrue(0),
          surfaceType(0),
          wingSpan(0),
          numberOfEngines(0),
          engineType(0),
          groundAltitude(0.0f),
          ambientTemperature(0.0f),
          totalAirTemperature(0.0f),
          ambientWindVelocity(0.0f),
          ambientWindDirection(0.0f),
          ambientPrecipState(0),
          ambientInCloud(0),
          ambientVisibility(0.0f),
          seaLevelPressure(0.0f),
          pitotIcePct(0.0f),
          structuralIcePct(0.0f)
    {}

    inline AircraftInfo toAircraftInfo() const {
        AircraftInfo aircraftInfo;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&title[0], sizeof(title), nullptr))) {
            aircraftInfo.name = title;
        }
        if (SUCCEEDED(StringCbLengthA(&atcId[0], sizeof(atcId), nullptr))) {
            aircraftInfo.atcId = atcId;
        }
        if (SUCCEEDED(StringCbLengthA(&atcAirline[0], sizeof(atcAirline), nullptr))) {
            aircraftInfo.atcAirline = atcAirline;
        }
        if (SUCCEEDED(StringCbLengthA(&atcFlightNumber[0], sizeof(atcFlightNumber), nullptr))) {
            aircraftInfo.atcFlightNumber = atcFlightNumber;
        }
        if (SUCCEEDED(StringCbLengthA(&category[0], sizeof(category), nullptr))) {
            aircraftInfo.category = category;
        }
        aircraftInfo.startOnGround = (simOnGround != 0);
        aircraftInfo.aircraftAltitudeAboveGround = planeAltAboveGround;

        aircraftInfo.initialAirspeed = airspeedTrue;
        aircraftInfo.surfaceType = toSurfaceType(surfaceType);
        aircraftInfo.wingSpan = wingSpan;
        aircraftInfo.numberOfEngines = numberOfEngines;
        aircraftInfo.engineType = toEngineType(engineType);

        aircraftInfo.groundAltitude = groundAltitude;
        aircraftInfo.ambientTemperature = ambientTemperature;
        aircraftInfo.totalAirTemperature = totalAirTemperature;
        aircraftInfo.windVelocity = ambientWindVelocity;
        aircraftInfo.windDirection = ambientWindDirection;
        aircraftInfo.precipitationState = toPrecipitationState(ambientPrecipState);
        aircraftInfo.inClouds = (ambientInCloud != 0);
        aircraftInfo.visibility = ambientVisibility;
        aircraftInfo.seaLevelPressure = seaLevelPressure;
        aircraftInfo.pitotIcingPercent = SkyMath::fromPercent(pitotIcePct);
        aircraftInfo.structuralIcingPercent = SkyMath::fromPercent(structuralIcePct);

        return aircraftInfo;
    }

    static void addToDataDefinition(HANDLE simConnectHandle);

private:
    static inline SimTypes::SurfaceType toSurfaceType(qint32 surfaceType)
    {
        switch (surfaceType) {
        case 0:
            return SimTypes::SurfaceType::Concrete;
            break;
        case 1:
            return SimTypes::SurfaceType::Grass;
            break;
        case 2:
            return SimTypes::SurfaceType::Water;
            break;
        case 3:
            return SimTypes::SurfaceType::BumpyGrass;
            break;
        case 4:
            return SimTypes::SurfaceType::Asphalt;
            break;
        case 5:
            return SimTypes::SurfaceType::ShortGrass;
            break;
        case 6:
            return SimTypes::SurfaceType::LongGrass;
            break;
        case 7:
            return SimTypes::SurfaceType::HardTurf;
            break;
        case 8:
            return SimTypes::SurfaceType::Snow;
            break;
        case 9:
            return SimTypes::SurfaceType::Ice;
            break;
        case 10:
            return SimTypes::SurfaceType::Urban;
            break;
        case 11:
            return SimTypes::SurfaceType::Forest;
            break;
        case 12:
            return SimTypes::SurfaceType::Dirt;
            break;
        case 13:
            return SimTypes::SurfaceType::Coral;
            break;
        case 14:
            return SimTypes::SurfaceType::Gravel;
            break;
        case 15:
            return SimTypes::SurfaceType::OilTreated;
            break;
        case 16:
            return SimTypes::SurfaceType::SteelMats;
            break;
        case 17:
            return SimTypes::SurfaceType::Bituminus;
            break;
        case 18:
            return SimTypes::SurfaceType::Brick;
            break;
        case 19:
            return SimTypes::SurfaceType::Macadam;
            break;
        case 20:
            return SimTypes::SurfaceType::Planks;
            break;
        case 21:
            return SimTypes::SurfaceType::Sand;
            break;
        case 22:
            return SimTypes::SurfaceType::Shale;
            break;
        case 23:
            return SimTypes::SurfaceType::Tarmac;
            break;
        case 24:
            return SimTypes::SurfaceType::WrightFlyerTrack;
            break;
        default:
            return SimTypes::SurfaceType::Unknown;
            break;
        }
    }

    static inline SimTypes::EngineType toEngineType(qint32 engineType)
    {
        switch (engineType) {
        case 0:
            return SimTypes::EngineType::Piston;
            break;
        case 1:
            return SimTypes::EngineType::Jet;
            break;
        case 2:
            return SimTypes::EngineType::None;
            break;
        case 3:
            return SimTypes::EngineType::HeloBellTurbine;
            break;
        case 4:
            return SimTypes::EngineType::Unsupported;
            break;
        case 5:
            return SimTypes::EngineType::Turboprop;
            break;
        default:
            return SimTypes::EngineType::Unknown;
            break;
        }
    }

    static inline SimTypes::PrecipitationState toPrecipitationState(qint32 precipitationState)
    {
        switch (precipitationState) {
        case 0:
            return SimTypes::PrecipitationState::Unknown;
            break;
        case 2:
            return SimTypes::PrecipitationState::None;
            break;
        case 4:
            return SimTypes::PrecipitationState::Rain;
            break;
        case 8:
            return SimTypes::PrecipitationState::Snow;
            break;
        default:
            return SimTypes::PrecipitationState::Unknown;
            break;
        }
    }
} __attribute__ ((packed));
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTINFO_H
