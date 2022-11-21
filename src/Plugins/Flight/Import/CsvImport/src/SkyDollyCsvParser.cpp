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
#include <cstdint>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>

#include <Kernel/CsvParser.h>
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

namespace
{
    constexpr const char *SkyDollyCsvHeader {"Type,Plane Latitude,Plane Longitude,Plane Altitude"};

    enum Index
    {
        Type = 0,
        // Position
        Latitude,
        Longitude,
        Altitude,
        IndicatedAltitude,
        Pitch,
        Bank,
        TrueHeading,
        VelocityBodyX,
        VelocityBodyY,
        VelocityBodyZ,
        RotationVelocityBodyX,
        RotationVelocityBodyY,
        RotationVelocityBodyZ,
        // Engine
        ThrottleLeverPosition1,
        ThrottleLeverPosition2,
        ThrottleLeverPosition3,
        ThrottleLeverPosition4,
        PropellerLeverPosition1,
        PropellerLeverPosition2,
        PropellerLeverPosition3,
        PropellerLeverPosition4,
        MixtureLeverPosition1,
        MixtureLeverPosition2,
        MixtureLeverPosition3,
        MixtureLeverPosition4,
        RecipEngineCowlFlapPosition1,
        RecipEngineCowlFlapPosition2,
        RecipEngineCowlFlapPosition3,
        RecipEngineCowlFlapPosition4,
        ElectricalMasterBattery1,
        ElectricalMasterBattery2,
        ElectricalMasterBattery3,
        ElectricalMasterBattery4,
        GeneralEngineStarter1,
        GeneralEngineStarter2,
        GeneralEngineStarter3,
        GeneralEngineStarter4,
        GeneralEngineCombustion1,
        GeneralEngineCombustion2,
        GeneralEngineCombustion3,
        GeneralEngineCombustion4,
        // Primary flight controls
        RudderPosition,
        ElevatorPosition,
        AileronPosition,
        // Secondary flight controls
        LeadingEdgeFlapsLeftPercent,
        LeadingEdgeFlapsRightPercent,
        TrailingEdgeFlapsLeftPercent,
        TrailingEdgeFlapsRightPercent,
        SpoilersHandlePosition,
        FlapsHandleIndex,
        // Aircraft handles
        GearHandlePosition,
        BrakeLeftPosition,
        BrakeRightPosition,
        WaterRudderHandlePosition,
        TailhookPosition,
        CanopyOpen,
        FoldingWingLeftPercent,
        FoldingWingRightPercent,
        SmokeEnable,
        // Light
        LightStates,
        // Common
        Timestamp
    };
}

// PUBLIC

bool SkyDollyCsvParser::parse(QIODevice &io, QDateTime &firstDateTimeUtc, [[maybe_unused]] QString &flightNumber, Flight &flight) noexcept
{
    Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();

    QFile *file = qobject_cast<QFile *>(&io);
    firstDateTimeUtc = (file != nullptr) ? QFileInfo(*file).birthTime().toUTC() : QDateTime::currentDateTimeUtc();

    CsvParser csvParser;
    QTextStream textStream(&io);
    textStream.setCodec(QTextCodec::codecForName("UTF-8"));
    CsvParser::Rows rows = csvParser.parse(textStream, ::SkyDollyCsvHeader);
    position.reserve(rows.size());
    bool ok {true};
    for (const auto &row : rows) {
        ok = parseRow(row, aircraft, firstDateTimeUtc, flightNumber);
        if (!ok) {
            break;
        }
    }
    return ok;
}

inline bool SkyDollyCsvParser::parseRow(const CsvParser::Row &row, Aircraft &aircraft, QDateTime &firstDateTimeUtc, QString &flightNumber) noexcept
{
    PositionData positionData;

    bool firstPositionData {true};
    bool firstEngineData {true};
    bool firstPrimaryFlightControlData {true};
    bool firstSecondaryFlightControlData {true};
    bool firstAircraftHandleData {true};
    bool firstLightData {true};

    CsvConst::DataType dataType = static_cast<CsvConst::DataType>(row.at(::Index::Type).at(0).toLatin1());

    bool ok {true};
    switch (dataType) {
    case CsvConst::DataType::Aircraft:
        ok = importPositionData(row, firstPositionData, aircraft);
        firstPositionData = false;
        break;
    case CsvConst::DataType::Engine:
        ok = importEngineData(row, firstEngineData, aircraft.getEngine());
        firstEngineData = false;
        break;
    case CsvConst::DataType::PrimaryFlightControl:
        ok = importPrimaryFlightControlData(row, firstPrimaryFlightControlData, aircraft.getPrimaryFlightControl());
        firstPrimaryFlightControlData = false;
        break;
    case CsvConst::DataType::SecondaryFlightControl:
        ok = importSecondaryFlightControlData(row, firstSecondaryFlightControlData, aircraft.getSecondaryFlightControl());
        firstSecondaryFlightControlData = false;
        break;
    case CsvConst::DataType::AircraftHandle:
        ok = importAircraftHandleData(row, firstAircraftHandleData, aircraft.getAircraftHandle());
        firstAircraftHandleData = false;
        break;
    case CsvConst::DataType::Light:
        ok = importLightData(row, firstLightData, aircraft.getLight());
        firstLightData = false;
        break;
    }

    return ok;
}

// PRIVATE

inline bool SkyDollyCsvParser::importPositionData(const CsvParser::Row &row, bool firstRow, Aircraft &aircraft) noexcept
{
    PositionData data;
    std::int64_t timestampDelta {0};
    bool ok {true};

    // Position
    data.latitude = row.at(::Index::Latitude).toDouble(&ok);
    if (ok) {
        data.longitude = row.at(::Index::Longitude).toDouble(&ok);
    }
    if (ok) {
        data.altitude = row.at(::Index::Altitude).toDouble(&ok);
    }
    if (ok) {
        data.indicatedAltitude = row.at(::Index::IndicatedAltitude).toDouble(&ok);
    }
    if (ok) {
        data.pitch = row.at(::Index::Pitch).toDouble(&ok);
    }
    if (ok) {
        data.bank = row.at(::Index::Bank).toDouble(&ok);
    }
    if (ok) {
        data.trueHeading = row.at(::Index::TrueHeading).toDouble(&ok);
    }
    // Velocity
    if (ok) {
        data.velocityBodyX = row.at(::Index::VelocityBodyX).toDouble(&ok);
    }
    if (ok) {
        data.velocityBodyY = row.at(::Index::VelocityBodyY).toDouble(&ok);
    }
    if (ok) {
        data.velocityBodyZ = row.at(::Index::VelocityBodyZ).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyX = row.at(::Index::RotationVelocityBodyX).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyY= row.at(::Index::RotationVelocityBodyY).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyZ = row.at(::Index::RotationVelocityBodyZ).toDouble(&ok);
    }
    // Timestamp
    ok = importTimestamp(row, firstRow, data.timestamp, timestampDelta);

    if (ok) {
        aircraft.getPosition().upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importEngineData(const CsvParser::Row &row, bool firstRow, Engine &engine) noexcept
{
//    EngineData data;
//    int ::Index::Longitude = 0;
//    std::int64_t timestampDelta = 0;
    bool ok {true};
//    for (const QByteArray &header : headers) {

//        if (::Index::Longitude >= row.count()) {
//            // Less values than headers
//            ok = false;
//            break;
//        }

//        int intValue;
//        if (header == SimVar::ThrottleLeverPosition1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.throttleLeverPosition1 = intValue;
//            }
//        } else if (header == SimVar::ThrottleLeverPosition2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.throttleLeverPosition2 = intValue;
//            }
//        } else if (header == SimVar::ThrottleLeverPosition3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.throttleLeverPosition3 = intValue;
//            }
//        } else if (header == SimVar::ThrottleLeverPosition4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.throttleLeverPosition4 = intValue;
//            }
//        } else if (header == SimVar::PropellerLeverPosition1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.propellerLeverPosition1 = intValue;
//            }
//        } else if (header == SimVar::PropellerLeverPosition2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.propellerLeverPosition2 = intValue;
//            }
//        } else if (header == SimVar::PropellerLeverPosition3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.propellerLeverPosition3 = intValue;
//            }
//        } else if (header == SimVar::PropellerLeverPosition4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.propellerLeverPosition4 = intValue;
//            }
//        } else if (header == SimVar::MixtureLeverPosition1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.mixtureLeverPosition1 = intValue;
//            }
//        } else if (header == SimVar::MixtureLeverPosition2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.mixtureLeverPosition2 = intValue;
//            }
//        } else if (header == SimVar::MixtureLeverPosition3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.mixtureLeverPosition3 = intValue;
//            }
//        } else if (header == SimVar::MixtureLeverPosition4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.mixtureLeverPosition4 = intValue;
//            }
//        } else if (header == SimVar::RecipEngineCowlFlapPosition1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.cowlFlapPosition1 = intValue;
//            }
//        } else if (header == SimVar::RecipEngineCowlFlapPosition2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.cowlFlapPosition2 = intValue;
//            }
//        } else if (header == SimVar::RecipEngineCowlFlapPosition3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.cowlFlapPosition3 = intValue;
//            }
//        } else if (header == SimVar::RecipEngineCowlFlapPosition4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.cowlFlapPosition4 = intValue;
//            }
//        } else if (header == SimVar::ElectricalMasterBattery1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.electricalMasterBattery1 = intValue != 0;
//            }
//        } else if (header == SimVar::ElectricalMasterBattery2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.electricalMasterBattery2 = intValue != 0;
//            }
//        } else if (header == SimVar::ElectricalMasterBattery3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.electricalMasterBattery3 = intValue != 0;
//            }
//        } else if (header == SimVar::ElectricalMasterBattery4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.electricalMasterBattery4 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineStarter1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineStarter1 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineStarter2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineStarter2 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineStarter3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineStarter3 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineStarter4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineStarter4 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineCombustion1) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineCombustion1 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineCombustion2) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineCombustion2 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineCombustion3) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineCombustion3 = intValue != 0;
//            }
//        } else if (header == SimVar::GeneralEngineCombustion4) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.generalEngineCombustion4 = intValue != 0;
//            }
//            // Timestamp
//        } else if (header == SimVar::Timestamp) {
//            ok = importTimestamp(values, ::Index::Longitude, firstRow, data.timestamp, timestampDelta);
//        } else {
//            ok = true;
//        }

//        if (ok) {
//            // Next value
//            ++::Index::Longitude;
//        } else {
//            // Parse error
//            break;
//        }

//    }
//    if (ok) {
//        engine.upsertLast(std::move(data));
//    }
    return ok;
}

inline bool SkyDollyCsvParser::importPrimaryFlightControlData(const CsvParser::Row &row, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept
{
//    PrimaryFlightControlData data;
//    int ::Index::Longitude = 0;
//    std::int64_t timestampDelta = 0;
    bool ok {true};
//    for (const QByteArray &header : headers) {

//        if (::Index::Longitude >= row.count()) {
//            // Less values than headers
//            ok = false;
//            break;
//        }

//        int intValue;
//        if (header == SimVar::RudderPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.rudderPosition = intValue;
//            }
//        } else if (header == SimVar::ElevatorPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.elevatorPosition = intValue;
//            }
//        } else if (header == SimVar::AileronPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.aileronPosition = intValue;
//            }
//            // Timestamp
//        } else if (header == SimVar::Timestamp) {
//            ok = importTimestamp(values, ::Index::Longitude, firstRow, data.timestamp, timestampDelta);
//        } else {
//            ok = true;
//        }

//        if (ok) {
//            // Next value
//            ++::Index::Longitude;
//        } else {
//            // Parse error
//            break;
//        }

//    }
//    if (ok) {
//        primaryFlightControl.upsertLast(std::move(data));
//    }
    return ok;
}

inline bool SkyDollyCsvParser::importSecondaryFlightControlData(const CsvParser::Row &row, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept
{
//    SecondaryFlightControlData data;
//    int ::Index::Longitude = 0;
//    std::int64_t timestampDelta = 0;
    bool ok {true};
//    for (const QByteArray &header : headers) {

//        if (::Index::Longitude >= row.count()) {
//            // Less values than headers
//            ok = false;
//            break;
//        }

//        int intValue;
//        if (header == SimVar::LeadingEdgeFlapsLeftPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.leadingEdgeFlapsLeftPosition = intValue;
//            }
//        } else if (header == SimVar::LeadingEdgeFlapsRightPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.leadingEdgeFlapsRightPosition = intValue;
//            }
//        } else if (header == SimVar::TrailingEdgeFlapsLeftPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.trailingEdgeFlapsLeftPosition = intValue;
//            }
//        } else if (header == SimVar::TrailingEdgeFlapsRightPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.trailingEdgeFlapsRightPosition = intValue;
//            }
//        } else if (header == SimVar::SpoilersHandlePosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.spoilersHandlePosition = intValue;
//            }
//        } else if (header == SimVar::FlapsHandleIndex) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.flapsHandleIndex = intValue;
//            }
//            // Timestamp
//        } else if (header == SimVar::Timestamp) {
//            ok = importTimestamp(values, ::Index::Longitude, firstRow, data.timestamp, timestampDelta);
//        } else {
//            ok = true;
//        }

//        if (ok) {
//            // Next value
//            ++::Index::Longitude;
//        } else {
//            // Parse error
//            break;
//        }

//    }
//    if (ok) {
//        secondaryFlightControl.upsertLast(std::move(data));
//    }
    return ok;
}

inline bool SkyDollyCsvParser::importAircraftHandleData(const CsvParser::Row &row, bool firstRow, AircraftHandle &aircraftHandle) noexcept
{
//    AircraftHandleData data;
//    int ::Index::Longitude = 0;
//    std::int64_t timestampDelta = 0;
    bool ok {true};
//    for (const QByteArray &header : headers) {

//        if (::Index::Longitude >= row.count()) {
//            // Less values than headers
//            ok = false;
//            break;
//        }

//        int intValue;
//        if (header == SimVar::GearHandlePosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.gearHandlePosition = intValue == 1 ? true : false;
//            }
//        } else if (header == SimVar::BrakeLeftPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.brakeLeftPosition = intValue;
//            }
//        } else if (header == SimVar::BrakeRightPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.brakeRightPosition = intValue;
//            }
//        } else if (header == SimVar::WaterRudderHandlePosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.waterRudderHandlePosition = intValue;
//            }
//        } else if (header == SimVar::TailhookPosition) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.tailhookPosition = intValue;
//            }
//        } else if (header == SimVar::CanopyOpen) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.canopyOpen = intValue;
//            }
//        } else if (header == SimVar::FoldingWingLeftPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.leftWingFolding = intValue;
//            }
//        } else if (header == SimVar::FoldingWingRightPercent) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.rightWingFolding = intValue;
//            }
//        } else if (header == SimVar::SmokeEnable) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.smokeEnabled = intValue == 1 ? true : false;
//            }
//            // Timestamp
//        } else if (header == SimVar::Timestamp) {
//            ok = importTimestamp(values, ::Index::Longitude, firstRow, data.timestamp, timestampDelta);
//        } else {
//            ok = true;
//        }

//        if (ok) {
//            // Next value
//            ++::Index::Longitude;
//        } else {
//            // Parse error
//            break;
//        }

//    }
//    if (ok) {
//        aircraftHandle.upsertLast(std::move(data));
//    }
    return ok;
}

inline bool SkyDollyCsvParser::importLightData(const CsvParser::Row &row, bool firstRow, Light &light) noexcept
{
//    LightData data;
//    int ::Index::Longitude = 0;
//    std::int64_t timestampDelta = 0;
    bool ok {true};
//    for (const QByteArray &header : headers) {

//        if (::Index::Longitude >= row.count()) {
//            // Less values than headers
//            ok = false;
//            break;
//        }

//        int intValue;
//        if (header == SimVar::LightStates) {
//            intValue = row.at(::Index::Longitude).toInt(&ok);
//            if (ok) {
//                data.lightStates = static_cast<SimType::LightStates>(intValue);
//            }
//            // Timestamp
//        } else if (header == SimVar::Timestamp) {
//            ok = importTimestamp(values, ::Index::Longitude, firstRow, data.timestamp, timestampDelta);
//        } else {
//            ok = true;
//        }

//        if (ok) {
//            // Next value
//            ++::Index::Longitude;
//        } else {
//            // Parse error
//            break;
//        }

//    }
//    if (ok) {
//        light.upsertLast(std::move(data));
//    }
    return ok;
}

inline bool SkyDollyCsvParser::importTimestamp(const CsvParser::Row &row, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta) noexcept
{
    bool ok {true};
    timestamp = row.at(::Index::Timestamp).toLongLong(&ok);
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
