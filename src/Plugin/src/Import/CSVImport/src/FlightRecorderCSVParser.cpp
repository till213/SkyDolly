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

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Persistence/src/CSVConst.h"
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

    constexpr int InvalidIndex = std::numeric_limits<int>::max();
}

class FlightRecorderCSVParserPrivate
{
public:
    FlightRecorderCSVParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    std::unordered_map<QByteArray, int> columnIndexes;
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
    int latitudeIndex {InvalidIndex}, longitudeIndex {InvalidIndex}, altitdueIndex = {InvalidIndex};
    int pitchIndex {InvalidIndex}, bankIndex {InvalidIndex}, trueHeadingIndex {InvalidIndex};
    int velocityBodyXIndex {InvalidIndex}, velocityBodyYIndex {InvalidIndex}, velocityBodyZIndex {InvalidIndex};
    int RotationVelocityBodyXIndex {InvalidIndex}, rotationVelocityBodyYIndex {InvalidIndex}, rotationVelocityBodyZIndex {InvalidIndex};
    // Engine
    int throttleLeverPosition1Index {InvalidIndex}, throttleLeverPosition2Index {InvalidIndex},
        throttleLeverPosition3Index {InvalidIndex}, throttleLeverPosition4Index {InvalidIndex};
    // Primary flight controls
    int rudderPositionIndex {InvalidIndex}, elevatorPositionIndex {InvalidIndex}, aileronPositionIndex {InvalidIndex};
    bool ok;
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();
    Engine &engine = aircraft.getEngine();
    PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControl();

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
        ok = ok && importValue(values, ::Latitude, latitudeIndex, positionData.latitude);
        ok = ok && importValue(values, ::Longitude, longitudeIndex, positionData.longitude);
        ok = ok && importValue(values, ::Altitude, altitdueIndex, positionData.altitude);
        ok = ok && importValue(values, ::Pitch, pitchIndex, positionData.pitch);
        ok = ok && importValue(values, ::Bank, bankIndex, positionData.bank);
        ok = ok && importValue(values, ::TrueHeading, trueHeadingIndex, positionData.heading);
        ok = ok && importValue(values, ::VelocityBodyX, velocityBodyXIndex, positionData.velocityBodyX);
        ok = ok && importValue(values, ::VelocityBodyY, velocityBodyYIndex, positionData.velocityBodyY);
        ok = ok && importValue(values, ::VelocityBodyZ, velocityBodyZIndex, positionData.velocityBodyZ);
        ok = ok && importValue(values, ::RotationVelocityBodyX, RotationVelocityBodyXIndex, positionData.rotationVelocityBodyX);
        ok = ok && importValue(values, ::RotationVelocityBodyY, rotationVelocityBodyYIndex, positionData.rotationVelocityBodyY);
        ok = ok && importValue(values, ::RotationVelocityBodyZ, rotationVelocityBodyZIndex, positionData.rotationVelocityBodyZ);

        if (ok) {
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
        ok = ok && importValue(values, ::ThrottleLeverPosition1, throttleLeverPosition1Index, throttleLeverPosition1);
        ok = ok && importValue(values, ::ThrottleLeverPosition2, throttleLeverPosition2Index, throttleLeverPosition2);
        ok = ok && importValue(values, ::ThrottleLeverPosition3, throttleLeverPosition3Index, throttleLeverPosition3);
        ok = ok && importValue(values, ::ThrottleLeverPosition4, throttleLeverPosition4Index, throttleLeverPosition4);
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
        ok = ok && importValue(values, ::RudderPosition, rudderPositionIndex, rudderPosition);
        ok = ok && importValue(values, ::ElevatorPosition, elevatorPositionIndex, elevatorPosition);
        ok = ok && importValue(values, ::AileronPosition, aileronPositionIndex, aileronPosition);
        if (ok) {
            primaryFlightControlData.rudderPosition = SkyMath::fromPosition(rudderPosition);
            primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(elevatorPosition);
            primaryFlightControlData.aileronPosition = SkyMath::fromPosition(aileronPosition);
            primaryFlightControl.upsertLast(primaryFlightControlData);
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
    static int timestampIndex = ::InvalidIndex;
    bool ok = true;;
    if (timestampIndex == ::InvalidIndex) {
        auto it = d->columnIndexes.find(::Milliseconds);
        if (it != d->columnIndexes.end()) {
            timestampIndex = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && timestampIndex < values.count()) {
        timestamp = values.at(timestampIndex).toLongLong(&ok);
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
inline bool FlightRecorderCSVParser::importValue(const QList<QByteArray> &values, const char *name, int &index, T &value) noexcept
{
    bool ok = true;;
    if (index == ::InvalidIndex) {
        auto it = d->columnIndexes.find(name);
        if (it != d->columnIndexes.end()) {
            index = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && index < values.count()) {
        // @todo call proper toXYZ method based on type T
        if constexpr (std::is_floating_point<T>::value) {
            if constexpr (std::is_same<T, double>::value) {
                value = values.at(index).toDouble(&ok);
            } else {
                value = values.at(index).toFloat(&ok);
            }
        } else if constexpr (std::is_integral<T>::value) {
            if constexpr (std::is_same<T, long long int>::value) {
                value = values.at(index).toLongLong(&ok);
            } else {
                value = values.at(index).toInt(&ok);
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
