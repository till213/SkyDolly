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
#include <QFile>
#include <QFileInfo>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include "../../../../../Kernel/src/QStringHasher.h"
#endif

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "FlightRecorderCSVParser.h"

namespace
{
    constexpr char Separator = ',';

    // Column names

    // Position
    constexpr char Milliseconds[] = "Milliseconds";
    constexpr char Latitude[] = "Latitude";
    constexpr char Longitude[] = "Longitude";
    constexpr char Altitude[] = "Altitude";
    constexpr char Pitch[] = "Pitch";
    constexpr char Bank[] = "Bank";
    constexpr char TrueHeading[] = "TrueHeading";
    constexpr char VelocityBodyX[] = "VelocityBodyX";
    constexpr char VelocityBodyY[] = "VelocityBodyY";
    constexpr char VelocityBodyZ[] = "VelocityBodyZ";
    constexpr char RotationVelocityBodyX[] = "RotationVelocityBodyX";
    constexpr char RotationVelocityBodyY[] = "RotationVelocityBodyY";
    constexpr char RotationVelocityBodyZ[] = "RotationVelocityBodyZ";

    // Engine
    constexpr char ThrottleLeverPosition1[] = "ThrottleLeverPosition1";
    constexpr char ThrottleLeverPosition2[] = "ThrottleLeverPosition2";
    constexpr char ThrottleLeverPosition3[] = "ThrottleLeverPosition3";
    constexpr char ThrottleLeverPosition4[] = "ThrottleLeverPosition4";

    // Primary flight controls
    constexpr char RudderPosition[] = "RudderPosition";
    constexpr char ElevatorPosition[] = "ElevatorPosition";
    constexpr char AileronPosition[] = "AileronPosition";

    // Secondary flight controls
    constexpr char LeadingEdgeFlapsLeftPercent[] = "LeadingEdgeFlapsLeftPercent";
    constexpr char LeadingEdgeFlapsRightPercent[] = "LeadingEdgeFlapsRightPercent";
    constexpr char TrailingEdgeFlapsLeftPercent[] = "TrailingEdgeFlapsLeftPercent";
    constexpr char TrailingEdgeFlapsRightPercent[] = "TrailingEdgeFlapsRightPercent";
    constexpr char SpoilerHandlePosition[] = "SpoilerHandlePosition";
    constexpr char FlapsHandleIndex[] = "FlapsHandleIndex";

    // Aircraft handles
    constexpr char BrakeLeftPosition[] = "BrakeLeftPosition";
    constexpr char BrakeRightPosition[] = "BrakeRightPosition";
    constexpr char WaterRudderHandlePosition[] = "WaterRudderHandlePosition";
    constexpr char GearHandlePosition[] = "GearHandlePosition";

    // Lights
    constexpr char LightTaxi[] = "LightTaxi";
    constexpr char LightLanding[] = "LightLanding";
    constexpr char LightStrobe[] = "LightStrobe";
    constexpr char LightBeacon[] = "LightBeacon";
    constexpr char LightNav[] = "LightNav";
    constexpr char LightWing[] = "LightWing";
    constexpr char LightLogo[] = "LightLogo";
    constexpr char LightRecognition[] = "LightRecognition";
    constexpr char LightCabin[] = "LightCabin";

    constexpr int InvalidIdx = std::numeric_limits<int>::max();
}

class FlightRecorderCSVParserPrivate
{
public:
    FlightRecorderCSVParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    std::unordered_map<QByteArray, int, QStringHasher> columnIndexes;
#else
    std::unordered_map<QByteArray, int> columnIndexes;
#endif
    QDateTime firstDateTimeUtc;
};

// PUBLIC

FlightRecorderCSVParser::FlightRecorderCSVParser() noexcept
    : d(std::make_unique<FlightRecorderCSVParserPrivate>())
{
#ifdef DEBUG
    qDebug("FlightRecorderCSVParser::~FlightRecorderCSVParser: CREATED");
#endif
}

FlightRecorderCSVParser::~FlightRecorderCSVParser() noexcept
{
#ifdef DEBUG
    qDebug("FlightRecorderCSVParser::~FlightRecorderCSVParser: DELETED");
#endif
}

bool FlightRecorderCSVParser::parse(QFile &file, QDateTime &firstDateTimeUtc, QString &flightNumber) noexcept
{
    firstDateTimeUtc = QFileInfo(file).birthTime().toUTC();
    bool ok = parseHeader(file);
    if (ok) {
        ok = parseData(file);
    }

    return ok;
}

// PRIVATE

bool FlightRecorderCSVParser::parseHeader(QFile &file) noexcept
{
    // Headers
    const QByteArray header = file.readLine();
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

bool FlightRecorderCSVParser::parseData(QFile &file) noexcept
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

    bool ok;
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();
    Engine &engine = aircraft.getEngine();
    PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControl();
    SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
    Light &light = aircraft.getLight();

    QByteArray data = file.readLine();
    // At least one data row expected
    ok = !data.isNull();
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
        ok = ok && importValue(values, ::TrueHeading, trueHeadingIdx, positionData.heading);
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

        data = file.readLine();
        firstRow = false;
    }

    return ok;
}

inline bool FlightRecorderCSVParser::importTimestamp(const QList<QByteArray> &values, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta) noexcept
{
    static int timestampIdx = ::InvalidIdx;
    bool ok = true;
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
inline bool FlightRecorderCSVParser::importValue(const QList<QByteArray> &values, const char *name, int &idx, T &value) noexcept
{
    bool ok = true;
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
        // @todo call proper toXYZ method based on type T
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

inline void FlightRecorderCSVParser::initEngineDefaultValues(EngineData &engineData) noexcept
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

inline void FlightRecorderCSVParser::initAircraftHandleDefaultValues(AircraftHandleData &aircraftHandleData) noexcept
{
    aircraftHandleData.tailhookPosition = 0;
    aircraftHandleData.canopyOpen = 0;
    aircraftHandleData.leftWingFolding = 0;
    aircraftHandleData.rightWingFolding = 0;
    aircraftHandleData.smokeEnabled = false;
}
