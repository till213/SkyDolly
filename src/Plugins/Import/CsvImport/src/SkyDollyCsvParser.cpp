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
#include <cstdint>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

#include <Kernel/Convert.h>
#include <Model/SimVar.h>
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
#include <PluginManager/CsvConst.h>
#include "SkyDollyCsvParser.h"

// PUBLIC

SkyDollyCsvParser::SkyDollyCsvParser() noexcept
{
#ifdef DEBUG
    qDebug("SkyDollyCsvParser::~SkyDollyCsvParser: CREATED");
#endif
}

SkyDollyCsvParser::~SkyDollyCsvParser() noexcept
{
#ifdef DEBUG
    qDebug("SkyDollyCsvParser::~SkyDollyCsvParser: DELETED");
#endif
}

bool SkyDollyCsvParser::parse(QFile &file, QDateTime &firstDateTimeUtc, [[maybe_unused]] QString &flightNumber, Flight &flight) noexcept
{
    Aircraft &aircraft = flight.getUserAircraft();

    firstDateTimeUtc = QFileInfo(file).birthTime().toUTC();

    // Headers
    QByteArray line = file.readLine();
    // Trim away line endings (\r\n for instance)
    QByteArray data = line.trimmed();

    bool ok = !data.isNull();
    if (ok) {
        QList<QByteArray> headers = data.split(CsvConst::TabSep);
        if (headers.first() == QString(CsvConst::TypeColumnName)) {
            headers.removeFirst();

            // CSV data
            data = file.readLine();
            bool firstPositionData = true;
            bool firstEngineData = true;
            bool firstPrimaryFlightControlData = true;
            bool firstSecondaryFlightControlData = true;
            bool firstAircraftHandleData = true;
            bool firstLightData = true;
            while (!data.isNull()) {

                PositionData positionData;
                QList<QByteArray> values = data.split(CsvConst::TabSep);

                // Type
                ok = values.at(0).size() > 0;
                if (ok) {
                    CsvConst::DataType dataType = static_cast<CsvConst::DataType>(values.at(0).at(0));
                    values.removeFirst();
                    switch (dataType) {
                    case CsvConst::DataType::Aircraft:
                        ok = importPositionData(headers, values, firstPositionData, aircraft);
                        firstPositionData = false;
                        break;
                    case CsvConst::DataType::Engine:
                        ok = importEngineData(headers, values, firstEngineData, aircraft.getEngine());
                        firstEngineData = false;
                        break;
                    case CsvConst::DataType::PrimaryFlightControl:
                        ok = importPrimaryFlightControlData(headers, values, firstPrimaryFlightControlData, aircraft.getPrimaryFlightControl());
                        firstPrimaryFlightControlData = false;
                        break;
                    case CsvConst::DataType::SecondaryFlightControl:
                        ok = importSecondaryFlightControlData(headers, values, firstSecondaryFlightControlData, aircraft.getSecondaryFlightControl());
                        firstSecondaryFlightControlData = false;
                        break;
                    case CsvConst::DataType::AircraftHandle:
                        ok = importAircraftHandleData(headers, values, firstAircraftHandleData, aircraft.getAircraftHandle());
                        firstAircraftHandleData = false;
                        break;
                    case CsvConst::DataType::Light:
                        ok = importLightData(headers, values, firstLightData, aircraft.getLight());
                        firstLightData = false;
                        break;
                    default:
                        // Ignore unknown data types
                        break;
                    }
                }

                // Read next line
                if (ok) {
                    data = file.readLine();
                } else {
                    break;
                }
            }

        } else {
            ok = false;
        }
    }
    return ok;
}

// PRIVATE

inline bool SkyDollyCsvParser::importPositionData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept
{
    PositionData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        // Position
        double doubleValue;
        if (header == SimVar::Latitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.latitude = doubleValue;
            }
        } else if (header == SimVar::Longitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.longitude = doubleValue;
            }
        } else if (header == SimVar::Altitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.altitude = doubleValue;
            }
        } else if (header == SimVar::IndicatedAltitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.indicatedAltitude = doubleValue;
            }
        } else if (header == SimVar::Pitch) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.pitch = doubleValue;
            }
        } else if (header == SimVar::Bank) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.bank = doubleValue;
            }
        } else if (header == SimVar::Heading) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.heading = doubleValue;
            }
            // Velocity
        } else if (header == SimVar::VelocityBodyX) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyX = doubleValue;
            }
        } else if (header == SimVar::VelocityBodyY) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyY = doubleValue;
            }
        } else if (header == SimVar::VelocityBodyZ) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyZ = doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyX) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyX = doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyY) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyY= doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyZ) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyZ = doubleValue;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        aircraft.getPosition().upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept
{
    EngineData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::ThrottleLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition1 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition2 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition3 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition4 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition1 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition2 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition3 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition4 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition1 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition2 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition3 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition4 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition1 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition2 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition3 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition4 = intValue;
            }
        } else if (header == SimVar::ElectricalMasterBattery1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery1 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery2 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery3 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery4 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter1 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter2 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter3 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter4 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineCombustion1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineCombustion1 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineCombustion2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineCombustion2 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineCombustion3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineCombustion3 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineCombustion4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineCombustion4 = intValue != 0;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        engine.upsertLast(std::move(data));
    }
    return ok;
}

inline bool SkyDollyCsvParser::importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept
{
    PrimaryFlightControlData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::RudderPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.rudderPosition = intValue;
            }
        } else if (header == SimVar::ElevatorPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.elevatorPosition = intValue;
            }
        } else if (header == SimVar::AileronPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.aileronPosition = intValue;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        primaryFlightControl.upsertLast(std::move(data));
    }
    return ok;
}

inline bool SkyDollyCsvParser::importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept
{
    SecondaryFlightControlData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::LeadingEdgeFlapsLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leadingEdgeFlapsLeftPosition = intValue;
            }
        } else if (header == SimVar::LeadingEdgeFlapsRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leadingEdgeFlapsRightPosition = intValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.trailingEdgeFlapsLeftPosition = intValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.trailingEdgeFlapsRightPosition = intValue;
            }
        } else if (header == SimVar::SpoilersHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.spoilersHandlePosition = intValue;
            }
        } else if (header == SimVar::FlapsHandleIndex) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.flapsHandleIndex = intValue;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        secondaryFlightControl.upsertLast(std::move(data));
    }
    return ok;
}

inline bool SkyDollyCsvParser::importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept
{
    AircraftHandleData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::GearHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.gearHandlePosition = intValue == 1 ? true : false;
            }
        } else if (header == SimVar::BrakeLeftPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.brakeLeftPosition = intValue;
            }
        } else if (header == SimVar::BrakeRightPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.brakeRightPosition = intValue;
            }
        } else if (header == SimVar::WaterRudderHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.waterRudderHandlePosition = intValue;
            }
        } else if (header == SimVar::TailhookPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.tailhookPosition = intValue;
            }
        } else if (header == SimVar::CanopyOpen) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.canopyOpen = intValue;
            }
        } else if (header == SimVar::FoldingWingLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leftWingFolding = intValue;
            }
        } else if (header == SimVar::FoldingWingRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.rightWingFolding = intValue;
            }
        } else if (header == SimVar::SmokeEnable) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.smokeEnabled = intValue == 1 ? true : false;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        aircraftHandle.upsertLast(std::move(data));
    }
    return ok;
}

inline bool SkyDollyCsvParser::importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept
{
    LightData data;
    int columnIndex = 0;
    std::int64_t timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::LightStates) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.lightStates = static_cast<SimType::LightStates>(intValue);
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        light.upsertLast(std::move(data));
    }
    return ok;
}

inline bool SkyDollyCsvParser::importTimestamp(const QList<QByteArray> &values, int columnIndex, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta) noexcept
{
    bool ok;
    timestamp = values.at(columnIndex).toLongLong(&ok);
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
    return ok;
}
