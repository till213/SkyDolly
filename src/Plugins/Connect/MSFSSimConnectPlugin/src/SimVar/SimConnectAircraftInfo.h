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

#include <cstdint>

#include <windows.h>
#include <strsafe.h>

#include <QtGlobal>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include <Kernel/Const.h>
#include <Kernel/SkyMath.h>
#include <Model/SimType.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/FlightCondition.h>

/*!
 * Information simulation variables about the aircraft and environment at start.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftInfo
{
    // Aircraft info
    char title[256] {'\0'};
    char atcId[32] {'\0'};
    char atcAirline[64] {'\0'};
    char atcFlightNumber[8] {'\0'};
    char category[256] {'\0'};
    // Feet
    float planeAltAboveGround {0.0f};
    std::int32_t simOnGround {0};
    // Knots
    std::int32_t airspeedTrue {0};
    // Feet
    std::int32_t wingSpan {0};
    std::int32_t engineType {0};
    std::int32_t numberOfEngines {0};

    // Flight conditions
    std::int32_t surfaceType {0};
    std::int32_t onAnyRunway {0};
    std::int32_t atcOnParkingSpot {0};
    std::int32_t surfaceCondition {0};

    float groundAltitude {0.0f};
    // Celcius
    float ambientTemperature {0.0f};
    float totalAirTemperature {0.0f};
    float ambientWindVelocity {0.0f};
    float ambientWindDirection {0.0f};
    float ambientVisibility {0.0f};
    float seaLevelPressure {0.0f};
    float pitotIcePct {0.0f};
    float structuralIcePct {0.0f};
    std::int32_t ambientPrecipState {0};
    std::int32_t ambientInCloud {0};

    // Simulation time
    // [seconds]
    std::int32_t localTime {0};
    std::int32_t localYear {0};
    std::int32_t localMonth {0};
    std::int32_t localDay {0};
    // [seconds]
    std::int32_t zuluTime {0};
    std::int32_t zuluYear {0};
    std::int32_t zuluMonth {0};
    std::int32_t zuluDay {0};

    inline AircraftInfo toAircraftInfo() const noexcept
    {
        AircraftInfo aircraftInfo(Const::InvalidId);

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
        flightCondition.onAnyRunway = (onAnyRunway != 0);
        flightCondition.onParkingSpot = (atcOnParkingSpot != 0);
        flightCondition.surfaceCondition = toSurfaceCondition(surfaceCondition, simOnGround);

        flightCondition.groundAltitude = groundAltitude;
        flightCondition.ambientTemperature = ambientTemperature;
        flightCondition.totalAirTemperature = totalAirTemperature;
        flightCondition.windSpeed = ambientWindVelocity;
        flightCondition.windDirection = ambientWindDirection;
        flightCondition.visibility = ambientVisibility;
        flightCondition.seaLevelPressure = seaLevelPressure;
        flightCondition.pitotIcingPercent = SkyMath::fromPercent(pitotIcePct);
        flightCondition.structuralIcingPercent = SkyMath::fromPercent(structuralIcePct);
        flightCondition.precipitationState = toPrecipitationState(ambientPrecipState);
        flightCondition.inClouds = (ambientInCloud != 0);

        QTime time = QTime::fromMSecsSinceStartOfDay(localTime * 1000);
        flightCondition.startLocalTime.setTime(time);
        QDate date = QDate(localYear, localMonth, localDay);
        flightCondition.startLocalTime.setDate(date);

        time = QTime::fromMSecsSinceStartOfDay(zuluTime * 1000);
        flightCondition.startZuluTime.setTime(time);
        date = QDate(zuluYear, zuluMonth, zuluDay);
        flightCondition.startZuluTime.setDate(date);

        return flightCondition;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

private:
    static inline SimType::SurfaceType toSurfaceType(std::int32_t type) noexcept
    {
        SimType::SurfaceType surfaceType;
        switch (type) {
        case 0:
            surfaceType = SimType::SurfaceType::Concrete;
            break;
        case 1:
            surfaceType = SimType::SurfaceType::Grass;
            break;
        case 2:
            surfaceType = SimType::SurfaceType::Water;
            break;
        case 3:
            surfaceType = SimType::SurfaceType::BumpyGrass;
            break;
        case 4:
            surfaceType = SimType::SurfaceType::Asphalt;
            break;
        case 5:
            surfaceType = SimType::SurfaceType::ShortGrass;
            break;
        case 6:
            surfaceType = SimType::SurfaceType::LongGrass;
            break;
        case 7:
            surfaceType = SimType::SurfaceType::HardTurf;
            break;
        case 8:
            surfaceType = SimType::SurfaceType::Snow;
            break;
        case 9:
            surfaceType = SimType::SurfaceType::Ice;
            break;
        case 10:
            surfaceType = SimType::SurfaceType::Urban;
            break;
        case 11:
            surfaceType = SimType::SurfaceType::Forest;
            break;
        case 12:
            surfaceType = SimType::SurfaceType::Dirt;
            break;
        case 13:
            surfaceType = SimType::SurfaceType::Coral;
            break;
        case 14:
            surfaceType = SimType::SurfaceType::Gravel;
            break;
        case 15:
            surfaceType = SimType::SurfaceType::OilTreated;
            break;
        case 16:
            surfaceType = SimType::SurfaceType::SteelMats;
            break;
        case 17:
            surfaceType = SimType::SurfaceType::Bituminus;
            break;
        case 18:
            surfaceType = SimType::SurfaceType::Brick;
            break;
        case 19:
            surfaceType = SimType::SurfaceType::Macadam;
            break;
        case 20:
            surfaceType = SimType::SurfaceType::Planks;
            break;
        case 21:
            surfaceType = SimType::SurfaceType::Sand;
            break;
        case 22:
            surfaceType = SimType::SurfaceType::Shale;
            break;
        case 23:
            surfaceType = SimType::SurfaceType::Tarmac;
            break;
        case 24:
            surfaceType = SimType::SurfaceType::WrightFlyerTrack;
            break;
        default:
            surfaceType = SimType::SurfaceType::Unknown;
            break;
        }
        return surfaceType;
    }

    static inline SimType::SurfaceCondition toSurfaceCondition(std::int32_t condition, bool valid) noexcept
    {
        SimType::SurfaceCondition surfaceCondition;
        if (valid) {
            switch (condition) {
            case 0:
                surfaceCondition = SimType::SurfaceCondition::Normal;
                break;
            case 1:
                surfaceCondition = SimType::SurfaceCondition::Wet;
                break;
            case 2:
                surfaceCondition = SimType::SurfaceCondition::Icy;
                break;
            case 3:
                surfaceCondition = SimType::SurfaceCondition::Snow;
                break;
            default:
                surfaceCondition = SimType::SurfaceCondition::Unknown;
                break;
            }
        } else {
            // Surface condition info invalid
            surfaceCondition = SimType::SurfaceCondition::Unknown;
        }
        return surfaceCondition;
    }

    static inline SimType::EngineType toEngineType(std::int32_t type) noexcept
    {
        SimType::EngineType engineType;
        switch (type) {
        case 0:
            engineType = SimType::EngineType::Piston;
            break;
        case 1:
            engineType = SimType::EngineType::Jet;
            break;
        case 2:
            engineType = SimType::EngineType::None;
            break;
        case 3:
            engineType = SimType::EngineType::HeloBellTurbine;
            break;
        case 4:
            engineType = SimType::EngineType::Unsupported;
            break;
        case 5:
            engineType = SimType::EngineType::Turboprop;
            break;
        default:
            engineType = SimType::EngineType::Unknown;
            break;
        }
        return engineType;
    }

    static inline SimType::PrecipitationState toPrecipitationState(std::int32_t state) noexcept
    {
        SimType::PrecipitationState precipitationState;
        switch (state) {
        case 0:
            precipitationState = SimType::PrecipitationState::Unknown;
            break;
        case 2:
            precipitationState = SimType::PrecipitationState::None;
            break;
        case 4:
            precipitationState = SimType::PrecipitationState::Rain;
            break;
        case 8:
            precipitationState = SimType::PrecipitationState::Snow;
            break;
        default:
            precipitationState = SimType::PrecipitationState::Unknown;
            break;
        }
        return precipitationState;
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTINFO_H
