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
#ifndef SIMCONNECTAIRCRAFTINFO_H
#define SIMCONNECTAIRCRAFTINFO_H

#include <windows.h>
#include <strsafe.h>

#include <QtGlobal>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftInfo.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Model/src/FlightCondition.h"

/*!
 * Information simulation variables about the aircraft and environment at start.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftInfo
{
    // Aircraft info
    char title[256];
    char atcId[32];
    char atcAirline[64];
    char atcFlightNumber[8];
    char category[256];
    // Feet
    float planeAltAboveGround;
    qint32 simOnGround;
    // Knots
    qint32 airspeedTrue;
    // Feet
    qint32 wingSpan;
    qint32 engineType;
    qint32 numberOfEngines;

    // Flight conditions
    qint32 surfaceType;
    float groundAltitude;
    // Celcius
    float ambientTemperature;
    float totalAirTemperature;
    float ambientWindVelocity;
    float ambientWindDirection;
    float ambientVisibility;
    float seaLevelPressure;
    float pitotIcePct;
    float structuralIcePct;
    qint32 ambientPrecipState;
    qint32 ambientInCloud;

    // Simulation time
    qint32 localTime;
    qint32 localYear;
    qint32 localMonth;
    qint32 localDay;
    qint32 zuluTime;
    qint32 zuluYear;
    qint32 zuluMonth;
    qint32 zuluDay;

    SimConnectAircraftInfo() noexcept
        : planeAltAboveGround(0.0f),
          simOnGround(false),
          airspeedTrue(0),
          wingSpan(0),
          engineType(0),
          numberOfEngines(0),
          surfaceType(0),
          groundAltitude(0.0f),
          ambientTemperature(0.0f),
          totalAirTemperature(0.0f),
          ambientWindVelocity(0.0f),
          ambientWindDirection(0.0f),
          ambientVisibility(0.0f),
          seaLevelPressure(0.0f),
          pitotIcePct(0.0f),
          structuralIcePct(0.0f),
          ambientPrecipState(0),
          ambientInCloud(0),
          localTime(0),
          localYear(0),
          localMonth(0),
          localDay(0),
          zuluTime(0),
          zuluYear(0),
          zuluMonth(0),
          zuluDay(0)
    {}

    inline AircraftInfo toAircraftInfo() const noexcept
    {
        AircraftInfo aircraftInfo(Aircraft::InvalidId);

        // Length check
        if (SUCCEEDED(StringCbLengthA(&title[0], sizeof(title), nullptr))) {
            aircraftInfo.aircraftType.type = QString(title);
        }
        aircraftInfo.aircraftType.wingSpan = wingSpan;
        aircraftInfo.aircraftType.engineType = toEngineType(engineType);
        aircraftInfo.aircraftType.numberOfEngines = numberOfEngines;

        if (SUCCEEDED(StringCbLengthA(&atcId[0], sizeof(atcId), nullptr))) {
            aircraftInfo.tailNumber = QString(atcId);
        }
        if (SUCCEEDED(StringCbLengthA(&atcAirline[0], sizeof(atcAirline), nullptr))) {
            aircraftInfo.airline = QString(atcAirline);
        }
        if (SUCCEEDED(StringCbLengthA(&atcFlightNumber[0], sizeof(atcFlightNumber), nullptr))) {
            aircraftInfo.flightNumber = QString(atcFlightNumber);
        }
        if (SUCCEEDED(StringCbLengthA(&category[0], sizeof(category), nullptr))) {
            aircraftInfo.aircraftType.category = QString(category);
        }

        aircraftInfo.initialAirspeed = airspeedTrue;
        aircraftInfo.altitudeAboveGround = planeAltAboveGround;
        aircraftInfo.startOnGround = (simOnGround != 0);

        return aircraftInfo;
    }

    inline FlightCondition toFlightCondition() const noexcept
    {
        FlightCondition flightCondition;

        flightCondition.surfaceType = toSurfaceType(surfaceType);
        flightCondition.groundAltitude = groundAltitude;
        flightCondition.ambientTemperature = ambientTemperature;
        flightCondition.totalAirTemperature = totalAirTemperature;
        flightCondition.windVelocity = ambientWindVelocity;
        flightCondition.windDirection = ambientWindDirection;
        flightCondition.visibility = ambientVisibility;
        flightCondition.seaLevelPressure = seaLevelPressure;
        flightCondition.pitotIcingPercent = SkyMath::fromPercent(pitotIcePct);
        flightCondition.structuralIcingPercent = SkyMath::fromPercent(structuralIcePct);
        flightCondition.precipitationState = toPrecipitationState(ambientPrecipState);
        flightCondition.inClouds = (ambientInCloud != 0);

        QTime time = QTime(0, 0).addSecs(localTime);
        flightCondition.startLocalTime.setTime(time);
        QDate date = QDate(localYear, localMonth, localDay);
        flightCondition.startLocalTime.setDate(date);

        time = QTime(0, 0).addSecs(zuluTime);
        flightCondition.startZuluTime.setTime(time);
        date = QDate(zuluYear, zuluMonth, zuluDay);
        flightCondition.startZuluTime.setDate(date);

        return flightCondition;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

private:
    static inline SimType::SurfaceType toSurfaceType(qint32 surfaceType) noexcept
    {
        switch (surfaceType) {
        case 0:
            return SimType::SurfaceType::Concrete;
            break;
        case 1:
            return SimType::SurfaceType::Grass;
            break;
        case 2:
            return SimType::SurfaceType::Water;
            break;
        case 3:
            return SimType::SurfaceType::BumpyGrass;
            break;
        case 4:
            return SimType::SurfaceType::Asphalt;
            break;
        case 5:
            return SimType::SurfaceType::ShortGrass;
            break;
        case 6:
            return SimType::SurfaceType::LongGrass;
            break;
        case 7:
            return SimType::SurfaceType::HardTurf;
            break;
        case 8:
            return SimType::SurfaceType::Snow;
            break;
        case 9:
            return SimType::SurfaceType::Ice;
            break;
        case 10:
            return SimType::SurfaceType::Urban;
            break;
        case 11:
            return SimType::SurfaceType::Forest;
            break;
        case 12:
            return SimType::SurfaceType::Dirt;
            break;
        case 13:
            return SimType::SurfaceType::Coral;
            break;
        case 14:
            return SimType::SurfaceType::Gravel;
            break;
        case 15:
            return SimType::SurfaceType::OilTreated;
            break;
        case 16:
            return SimType::SurfaceType::SteelMats;
            break;
        case 17:
            return SimType::SurfaceType::Bituminus;
            break;
        case 18:
            return SimType::SurfaceType::Brick;
            break;
        case 19:
            return SimType::SurfaceType::Macadam;
            break;
        case 20:
            return SimType::SurfaceType::Planks;
            break;
        case 21:
            return SimType::SurfaceType::Sand;
            break;
        case 22:
            return SimType::SurfaceType::Shale;
            break;
        case 23:
            return SimType::SurfaceType::Tarmac;
            break;
        case 24:
            return SimType::SurfaceType::WrightFlyerTrack;
            break;
        default:
            return SimType::SurfaceType::Unknown;
            break;
        }
    }

    static inline SimType::EngineType toEngineType(qint32 engineType) noexcept
    {
        switch (engineType) {
        case 0:
            return SimType::EngineType::Piston;
            break;
        case 1:
            return SimType::EngineType::Jet;
            break;
        case 2:
            return SimType::EngineType::None;
            break;
        case 3:
            return SimType::EngineType::HeloBellTurbine;
            break;
        case 4:
            return SimType::EngineType::Unsupported;
            break;
        case 5:
            return SimType::EngineType::Turboprop;
            break;
        default:
            return SimType::EngineType::Unknown;
            break;
        }
    }

    static inline SimType::PrecipitationState toPrecipitationState(qint32 precipitationState) noexcept
    {
        switch (precipitationState) {
        case 0:
            return SimType::PrecipitationState::Unknown;
            break;
        case 2:
            return SimType::PrecipitationState::None;
            break;
        case 4:
            return SimType::PrecipitationState::Rain;
            break;
        case 8:
            return SimType::PrecipitationState::Snow;
            break;
        default:
            return SimType::PrecipitationState::Unknown;
            break;
        }
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTINFO_H
