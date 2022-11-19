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
#include <memory>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <type_traits>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>

#include <Kernel/Convert.h>
#include <Kernel/SkyMath.h>
#include <Model/SimType.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include "FlightRecorderCsvParser.h"

namespace
{
    constexpr char Separator = ',';

    // Column names

    // Position
    constexpr const char *Milliseconds {"Milliseconds"};
    constexpr const char *Latitude {"Latitude"};
    constexpr const char *Longitude {"Longitude"};
    constexpr const char *Altitude {"Altitude"};
    constexpr const char *Pitch {"Pitch"};
    constexpr const char *Bank {"Bank"};
    constexpr const char *TrueHeading {"TrueHeading"};
    constexpr const char *VelocityBodyX {"VelocityBodyX"};
    constexpr const char *VelocityBodyY {"VelocityBodyY"};
    constexpr const char *VelocityBodyZ {"VelocityBodyZ"};
    constexpr const char *RotationVelocityBodyX {"RotationVelocityBodyX"};
    constexpr const char *RotationVelocityBodyY {"RotationVelocityBodyY"};
    constexpr const char *RotationVelocityBodyZ {"RotationVelocityBodyZ"};

    // Engine
    constexpr const char *ThrottleLeverPosition1 {"ThrottleLeverPosition1"};
    constexpr const char *ThrottleLeverPosition2 {"ThrottleLeverPosition2"};
    constexpr const char *ThrottleLeverPosition3 {"ThrottleLeverPosition3"};
    constexpr const char *ThrottleLeverPosition4 {"ThrottleLeverPosition4"};

    // Primary flight controls
    constexpr const char *RudderPosition {"RudderPosition"};
    constexpr const char *ElevatorPosition {"ElevatorPosition"};
    constexpr const char *AileronPosition {"AileronPosition"};

    // Secondary flight controls
    constexpr const char *LeadingEdgeFlapsLeftPercent {"LeadingEdgeFlapsLeftPercent"};
    constexpr const char *LeadingEdgeFlapsRightPercent {"LeadingEdgeFlapsRightPercent"};
    constexpr const char *TrailingEdgeFlapsLeftPercent {"TrailingEdgeFlapsLeftPercent"};
    constexpr const char *TrailingEdgeFlapsRightPercent {"TrailingEdgeFlapsRightPercent"};
    constexpr const char *SpoilerHandlePosition {"SpoilerHandlePosition"};
    constexpr const char *FlapsHandleIndex {"FlapsHandleIndex"};

    // Aircraft handles
    constexpr const char *BrakeLeftPosition {"BrakeLeftPosition"};
    constexpr const char *BrakeRightPosition {"BrakeRightPosition"};
    constexpr const char *WaterRudderHandlePosition {"WaterRudderHandlePosition"};
    constexpr const char *GearHandlePosition {"GearHandlePosition"};

    // Lights
    constexpr const char *LightTaxi {"LightTaxi"};
    constexpr const char *LightLanding {"LightLanding"};
    constexpr const char *LightStrobe {"LightStrobe"};
    constexpr const char *LightBeacon {"LightBeacon"};
    constexpr const char *LightNav {"LightNav"};
    constexpr const char *LightWing {"LightWing"};
    constexpr const char *LightLogo {"LightLogo"};
    constexpr const char *LightRecognition {"LightRecognition"};
    constexpr const char *LightCabin {"LightCabin"};

    constexpr int InvalidIdx = std::numeric_limits<int>::max();
}

struct FlightRecorderCsvParserPrivate
{
    FlightRecorderCsvParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    Flight *flight {nullptr};
    std::unordered_map<QByteArray, int> columnIndexes;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

FlightRecorderCsvParser::FlightRecorderCsvParser() noexcept
    : d(std::make_unique<FlightRecorderCsvParserPrivate>())
{}

FlightRecorderCsvParser::~FlightRecorderCsvParser() = default;

bool FlightRecorderCsvParser::parse(QIODevice &io, QDateTime &firstDateTimeUtc, [[maybe_unused]] QString &flightNumber, Flight &flight) noexcept
{
    d->flight = &flight;
    QFile *file = qobject_cast<QFile *>(&io);
    firstDateTimeUtc = (file != nullptr) ? QFileInfo(*file).birthTime().toUTC() : QDateTime::currentDateTimeUtc();
    bool ok = parseHeader(io);
    if (ok) {
        ok = parseData(io);
    }
    // We are done with the export
    d->flight = nullptr;
    return ok;
}

// PRIVATE

bool FlightRecorderCsvParser::parseHeader(QIODevice &io) noexcept
{
    // Headers
    const QByteArray header = io.readLine();
    bool ok = !header.isNull();

    if (ok) {
        QList<QByteArray> headers = header.split(::Separator);
        ok = headers.count() > 0;
        if (ok) {
            int index = 0;
            for (const QByteArray &header : headers) {
                d->columnIndexes[header] = index;
                ++index;
            }
        }
    }

    return ok;
}

bool FlightRecorderCsvParser::parseData(QIODevice &io) noexcept
{
    // Position
    int latitudeIdx {InvalidIdx}, longitudeIdx {InvalidIdx}, altitdueIdx = {InvalidIdx};
    int pitchIdx {InvalidIdx}, bankIdx {InvalidIdx}, trueHeadingIdx {InvalidIdx};
    int velocityBodyXIdx {InvalidIdx}, velocityBodyYIdx {InvalidIdx}, velocityBodyZIdx {InvalidIdx};
    int RotationVelocityBodyXIdx {InvalidIdx}, rotationVelocityBodyYIdx {InvalidIdx}, rotationVelocityBodyZIdx {InvalidIdx};
    // Engine
    int throttleLeverPosition1Idx {InvalidIdx}, throttleLeverPosition2Idx {InvalidIdx},
        throttleLeverPosition3Idx {InvalidIdx}, throttleLeverPosition4Idx {InvalidIdx};
    // Primary flight controls
    int rudderPositionIdx {InvalidIdx}, elevatorPositionIdx {InvalidIdx}, aileronPositionIdx {InvalidIdx};
    // Secondary flight controls
    int leadingEdgeFlapsLeftPercentIdx {InvalidIdx}, leadingEdgeFlapsRightPercentIdx {InvalidIdx},
        trailingEdgeFlapsLeftPercentIdx {InvalidIdx}, trailingEdgeFlapsRightPercentIdx {InvalidIdx},
        spoilerHandlePositionIdx {InvalidIdx}, flapsHandleIndexIdx {InvalidIdx};
    //  Aircraft handles
    int brakeLeftPositionIdx {InvalidIdx}, brakeRightPositionIdx {InvalidIdx},
        waterRudderHandlePositionIdx {InvalidIdx}, gearHandlePositionIdx {InvalidIdx};
    // Lights
    int lightTaxiIdx {InvalidIdx}, lightLandingIdx {InvalidIdx}, lightStrobeIdx {InvalidIdx}, lightBeaconIdx {InvalidIdx},
        lightNavIdx {InvalidIdx}, lightWingIdx {InvalidIdx}, lightLogoIdx {InvalidIdx}, lightRecognitionIdx {InvalidIdx},
        lightCabinIdx {InvalidIdx};

    Aircraft &aircraft = d->flight->getUserAircraft();
    Position &position = aircraft.getPosition();
    Engine &engine = aircraft.getEngine();
    PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControl();
    SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
    Light &light = aircraft.getLight();

    QByteArray data = io.readLine();
    // At least one data row expected
    bool ok = !data.isNull();
    bool firstRow {true};
    std::int64_t timestampDelta {0};
    while (ok && !data.isNull()) {

        QList<QByteArray> values = data.split(::Separator);

        std::int64_t timestamp {TimeVariableData::InvalidTime};
        ok = importTimestamp(values, firstRow, timestamp, timestampDelta);

        // Position
        PositionData positionData;
        positionData.timestamp = timestamp;
        ok = ok && importValue(values, ::Latitude, latitudeIdx, positionData.latitude);
        ok = ok && importValue(values, ::Longitude, longitudeIdx, positionData.longitude);
        ok = ok && importValue(values, ::Altitude, altitdueIdx, positionData.altitude);
        ok = ok && importValue(values, ::Pitch, pitchIdx, positionData.pitch);
        ok = ok && importValue(values, ::Bank, bankIdx, positionData.bank);
        ok = ok && importValue(values, ::TrueHeading, trueHeadingIdx, positionData.trueHeading);
        ok = ok && importValue(values, ::VelocityBodyX, velocityBodyXIdx, positionData.velocityBodyX);
        ok = ok && importValue(values, ::VelocityBodyY, velocityBodyYIdx, positionData.velocityBodyY);
        ok = ok && importValue(values, ::VelocityBodyZ, velocityBodyZIdx, positionData.velocityBodyZ);
        ok = ok && importValue(values, ::RotationVelocityBodyX, RotationVelocityBodyXIdx, positionData.rotationVelocityBodyX);
        ok = ok && importValue(values, ::RotationVelocityBodyY, rotationVelocityBodyYIdx, positionData.rotationVelocityBodyY);
        ok = ok && importValue(values, ::RotationVelocityBodyZ, rotationVelocityBodyZIdx, positionData.rotationVelocityBodyZ);

        if (ok) {
            positionData.indicatedAltitude = positionData.altitude;
            position.upsertLast(positionData);
        } else {
            break;
        }

        // Engine
        EngineData engineData;
        engineData.timestamp = timestamp;
        double throttleLeverPosition1 {0.0f};
        double throttleLeverPosition2 {0.0f};
        double throttleLeverPosition3 {0.0f};
        double throttleLeverPosition4 {0.0f};
        ok = ok && importValue(values, ::ThrottleLeverPosition1, throttleLeverPosition1Idx, throttleLeverPosition1);
        ok = ok && importValue(values, ::ThrottleLeverPosition2, throttleLeverPosition2Idx, throttleLeverPosition2);
        ok = ok && importValue(values, ::ThrottleLeverPosition3, throttleLeverPosition3Idx, throttleLeverPosition3);
        ok = ok && importValue(values, ::ThrottleLeverPosition4, throttleLeverPosition4Idx, throttleLeverPosition4);
        if (ok) {
            engineData.throttleLeverPosition1 = SkyMath::fromPosition(throttleLeverPosition1);
            engineData.throttleLeverPosition2 = SkyMath::fromPosition(throttleLeverPosition2);
            engineData.throttleLeverPosition3 = SkyMath::fromPosition(throttleLeverPosition3);
            engineData.throttleLeverPosition4 = SkyMath::fromPosition(throttleLeverPosition4);
            // Flight recorder does not support all Sky Dolly simulation variables, so we initialise them to "engine on"
            initEngineDefaultValues(engineData);
            engine.upsertLast(engineData);
        } else {
            break;
        }

        // Primary flight controls
        PrimaryFlightControlData primaryFlightControlData;
        primaryFlightControlData.timestamp = timestamp;

        double rudderPosition {0.0f};
        double elevatorPosition {0.0f};
        double aileronPosition {0.0f};
        ok = ok && importValue(values, ::RudderPosition, rudderPositionIdx, rudderPosition);
        ok = ok && importValue(values, ::ElevatorPosition, elevatorPositionIdx, elevatorPosition);
        ok = ok && importValue(values, ::AileronPosition, aileronPositionIdx, aileronPosition);
        if (ok) {
            primaryFlightControlData.rudderPosition = SkyMath::fromPosition(rudderPosition);
            primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(elevatorPosition);
            primaryFlightControlData.aileronPosition = SkyMath::fromPosition(aileronPosition);
            primaryFlightControl.upsertLast(primaryFlightControlData);
        } else {
            break;
        }

        // Secondary flight controls
        SecondaryFlightControlData secondaryFlightControlData;
        secondaryFlightControlData.timestamp = timestamp;

        double leadingEdgeFlapsLeftPosition {0.0f};
        double leadingEdgeFlapsRightPosition {0.0f};
        double trailingEdgeFlapsLeftPosition {0.0f};
        double trailingEdgeFlapsRightPosition {0.0f};
        double spoilerHandlePositionPercent {0.0f};
        ok = ok && importValue(values, ::LeadingEdgeFlapsLeftPercent, leadingEdgeFlapsLeftPercentIdx, leadingEdgeFlapsLeftPosition);
        ok = ok && importValue(values, ::LeadingEdgeFlapsRightPercent, leadingEdgeFlapsRightPercentIdx, leadingEdgeFlapsRightPosition);
        ok = ok && importValue(values, ::TrailingEdgeFlapsLeftPercent, trailingEdgeFlapsLeftPercentIdx, trailingEdgeFlapsLeftPosition);
        ok = ok && importValue(values, ::TrailingEdgeFlapsRightPercent, trailingEdgeFlapsRightPercentIdx, trailingEdgeFlapsRightPosition);
        ok = ok && importValue(values, ::SpoilerHandlePosition, spoilerHandlePositionIdx, spoilerHandlePositionPercent);
        ok = ok && importValue(values, ::FlapsHandleIndex, flapsHandleIndexIdx, secondaryFlightControlData.flapsHandleIndex);
        if (ok) {
            secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromPosition(leadingEdgeFlapsLeftPosition);
            secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromPosition(leadingEdgeFlapsRightPosition);
            secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromPosition(trailingEdgeFlapsLeftPosition);
            secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromPosition(trailingEdgeFlapsRightPosition);
            secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(spoilerHandlePositionPercent);
            secondaryFlightControl.upsertLast(secondaryFlightControlData);
        } else {
            break;
        }

        // Aircraft handle
        AircraftHandleData aircraftHandleData;
        aircraftHandleData.timestamp = timestamp;

        double brakeLeftPosition {0.0f};
        double brakeRightPosition {0.0f};
        double waterRudderHandlePosition {0.0f};
        ok = ok && importValue(values, ::BrakeLeftPosition, brakeLeftPositionIdx, brakeLeftPosition);
        ok = ok && importValue(values, ::BrakeRightPosition, brakeRightPositionIdx, brakeRightPosition);
        ok = ok && importValue(values, ::WaterRudderHandlePosition, waterRudderHandlePositionIdx, waterRudderHandlePosition);
        ok = ok && importValue(values, ::GearHandlePosition, gearHandlePositionIdx, aircraftHandleData.gearHandlePosition);
        if (ok) {
            aircraftHandleData.brakeLeftPosition = SkyMath::fromPosition(leadingEdgeFlapsLeftPosition);
            aircraftHandleData.brakeLeftPosition = SkyMath::fromPosition(leadingEdgeFlapsRightPosition);
            aircraftHandleData.waterRudderHandlePosition = SkyMath::fromPosition(waterRudderHandlePosition);
            // Flight recorder does not support all Sky Dolly simulation variables, so we initialise them to
            // some reasonable values
            initAircraftHandleDefaultValues(aircraftHandleData);
            aircraftHandle.upsertLast(aircraftHandleData);
        } else {
            break;
        }

        // Lights
        LightData lightData;
        lightData.timestamp = timestamp;

        bool lightTaxi {false};
        bool lightLanding {false};
        bool lightStrobe {false};
        bool lightBeacon {false};
        bool lightNav {false};
        bool lightWing {false};
        bool lightLogo {false};
        bool lightRecognition {false};
        bool lightCabin {false};
        ok = ok && importValue(values, ::LightTaxi, lightTaxiIdx, lightTaxi);
        ok = ok && importValue(values, ::LightLanding, lightLandingIdx, lightLanding);
        ok = ok && importValue(values, ::LightStrobe, lightStrobeIdx, lightStrobe);
        ok = ok && importValue(values, ::LightBeacon, lightBeaconIdx, lightBeacon);
        ok = ok && importValue(values, ::LightNav, lightNavIdx, lightNav);
        ok = ok && importValue(values, ::LightWing, lightWingIdx, lightWing);
        ok = ok && importValue(values, ::LightLogo, lightLogoIdx, lightLogo);
        ok = ok && importValue(values, ::LightRecognition, lightRecognitionIdx, lightRecognition);
        ok = ok && importValue(values, ::LightCabin, lightCabinIdx, lightCabin);
        if (ok) {
            lightData.lightStates.setFlag(SimType::LightState::Taxi, lightTaxi);
            lightData.lightStates.setFlag(SimType::LightState::Landing, lightLanding);
            lightData.lightStates.setFlag(SimType::LightState::Strobe, lightStrobe);
            lightData.lightStates.setFlag(SimType::LightState::Beacon, lightBeacon);
            lightData.lightStates.setFlag(SimType::LightState::Navigation, lightNav);
            lightData.lightStates.setFlag(SimType::LightState::Wing, lightWing);
            lightData.lightStates.setFlag(SimType::LightState::Logo, lightLogo);
            lightData.lightStates.setFlag(SimType::LightState::Recognition, lightRecognition);
            lightData.lightStates.setFlag(SimType::LightState::Cabin, lightCabin);
            light.upsertLast(lightData);
        } else {
            break;
        }

        data = io.readLine();
        firstRow = false;
    }

    return ok;
}

inline bool FlightRecorderCsvParser::importTimestamp(const QList<QByteArray> &values, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta) noexcept
{
    static int timestampIdx = ::InvalidIdx;
    bool ok {true};
    if (timestampIdx == ::InvalidIdx) {
        auto it = d->columnIndexes.find(::Milliseconds);
        if (it != d->columnIndexes.end()) {
            timestampIdx = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && timestampIdx < values.count()) {
        timestamp = values.at(timestampIdx).toLongLong(&ok);
        if (ok) {
            if (!firstRow) {
                timestamp += timestampDelta;
            } else {
                // The first timestamp must be 0, so shift all timestamps by
                // the timestamp delta, derived from the first timestamp
                // (which is usually 0 already)
                timestampDelta = -timestamp;
                timestamp = 0.0;
            }
        }
    }
    return ok;
}

template <typename T>
inline bool FlightRecorderCsvParser::importValue(const QList<QByteArray> &values, const char *name, int &idx, T &value) noexcept
{
    bool ok {true};
    if (idx == ::InvalidIdx) {
        auto it = d->columnIndexes.find(name);
        if (it != d->columnIndexes.end()) {
            idx = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && idx < values.count()) {
        if constexpr (std::is_floating_point<T>::value) {
            if constexpr (std::is_same<T, double>::value) {
                value = values.at(idx).toDouble(&ok);
            } else {
                value = values.at(idx).toFloat(&ok);
            }
        } else if constexpr (std::is_integral<T>::value) {
            if constexpr (std::is_same<T, long long int>::value) {
                value = values.at(idx).toLongLong(&ok);
            } else if constexpr (std::is_same<T, bool>::value) {
                value = values.at(idx).toInt(&ok) == 1;
            } else {
                value = values.at(idx).toInt(&ok);
            }
        } else {
            // Also refer to https://stackoverflow.com/questions/38304847/constexpr-if-and-static-assert
            // From C++20 onwards we can use lambda parameters, e.g.
            // []<bool flag = false>() {static_assert(flag, "no match");}();
            static_assert(!sizeof(T), "The type T is not supported.");
        }
    }
    return ok;
}

inline void FlightRecorderCsvParser::initEngineDefaultValues(EngineData &engineData) noexcept
{
    // Turn "engine on" and "full propeller", no cowl flaps
    engineData.propellerLeverPosition1 = SkyMath::PositionMax16;
    engineData.propellerLeverPosition2 = SkyMath::PositionMax16;
    engineData.propellerLeverPosition3 = SkyMath::PositionMax16;
    engineData.propellerLeverPosition4 = SkyMath::PositionMax16;
    engineData.mixtureLeverPosition1 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition2 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition3 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition4 = SkyMath::PercentMax8;
    engineData.cowlFlapPosition1 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition2 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition3 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition4 = SkyMath::PercentMin8;
    engineData.electricalMasterBattery1 = true;
    engineData.electricalMasterBattery2 = true;
    engineData.electricalMasterBattery3 = true;
    engineData.electricalMasterBattery4 = true;
    engineData.generalEngineStarter1 = true;
    engineData.generalEngineStarter2 = true;
    engineData.generalEngineStarter3 = true;
    engineData.generalEngineStarter4 = true;
    engineData.generalEngineCombustion1 = true;
    engineData.generalEngineCombustion2 = true;
    engineData.generalEngineCombustion3 = true;
    engineData.generalEngineCombustion4 = true;
}

inline void FlightRecorderCsvParser::initAircraftHandleDefaultValues(AircraftHandleData &aircraftHandleData) noexcept
{
    aircraftHandleData.tailhookPosition = 0;
    aircraftHandleData.canopyOpen = 0;
    aircraftHandleData.leftWingFolding = 0;
    aircraftHandleData.rightWingFolding = 0;
    aircraftHandleData.smokeEnabled = false;
}