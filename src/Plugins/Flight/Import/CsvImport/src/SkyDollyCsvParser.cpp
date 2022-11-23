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
#include <QString>
#include <QDateTime>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/CsvParser.h>
#include <Kernel/Convert.h>
#include <Kernel/Enum.h>
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

    enum struct Index
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
    QFile *file = qobject_cast<QFile *>(&io);
    firstDateTimeUtc = (file != nullptr) ? QFileInfo(*file).birthTime().toUTC() : QDateTime::currentDateTimeUtc();
    flightNumber = QString();

    CsvParser csvParser;
    QTextStream textStream(&io);
    textStream.setCodec(QTextCodec::codecForName("UTF-8"));
    CsvParser::Rows rows = csvParser.parse(textStream, ::SkyDollyCsvHeader);

    Aircraft &aircraft = flight.getUserAircraft();
    // Heuristical memory pre-allocation: we expect that about
    // - half of the rows are position samples
    // - 1/4 are engine samples
    // - etc.
    aircraft.getPosition().reserve(rows.size() >> 1);
    aircraft.getEngine().reserve(rows.size() >> 2);
    aircraft.getPrimaryFlightControl().reserve(rows.size() >> 3);
    aircraft.getSecondaryFlightControl().reserve(rows.size() >> 4);
    aircraft.getAircraftHandle().reserve(rows.size() >> 6);
    aircraft.getLight().reserve(rows.size() >> 6);

#ifdef DEBUG
    qDebug() << "SkyDollyCsvParser::parse, total CSV rows:" << rows.size() << "\n"
             << "Position size:" << aircraft.getPosition().capacity() << "\n"
             << "Engine size:" << aircraft.getEngine().capacity() << "\n"
             << "Primary flight controls size:" << aircraft.getPrimaryFlightControl().capacity() << "\n"
             << "Secondary flight controls size:" << aircraft.getSecondaryFlightControl().capacity() << "\n"
             << "Aircraft handles size:" << aircraft.getAircraftHandle().capacity() << "\n"
             << "Light size:" << aircraft.getLight().capacity() << "\n";
#endif

    bool firstRow {true};
    bool ok {true};
    for (const auto &row : rows) {
        if (firstRow) {
            // The first position timestamp must be 0, so shift all timestamps by
            // the timestamp delta, derived from the first timestamp
            // (that is usually 0 already)
            m_timestampDelta = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok);
            firstRow = false;
        }
        if (ok) {
            ok = parseRow(row, aircraft);
        } else {
            break;
        }
    }
    return ok;
}

inline bool SkyDollyCsvParser::parseRow(const CsvParser::Row &row, Aircraft &aircraft) noexcept
{
    CsvConst::DataType dataType = static_cast<CsvConst::DataType>(row.at(Enum::toUnderlyingType(::Index::Type)).at(0).toLatin1());

    bool ok {true};
    switch (dataType) {
    case CsvConst::DataType::Aircraft:
        ok = importPositionData(row, aircraft);
        break;
    case CsvConst::DataType::Engine:
        ok = importEngineData(row, aircraft.getEngine());
        break;
    case CsvConst::DataType::PrimaryFlightControl:
        ok = importPrimaryFlightControlData(row, aircraft.getPrimaryFlightControl());
        break;
    case CsvConst::DataType::SecondaryFlightControl:
        ok = importSecondaryFlightControlData(row, aircraft.getSecondaryFlightControl());
        break;
    case CsvConst::DataType::AircraftHandle:
        ok = importAircraftHandleData(row, aircraft.getAircraftHandle());
        break;
    case CsvConst::DataType::Light:
        ok = importLightData(row, aircraft.getLight());
        break;
    }
    return ok;
}

// PRIVATE

inline bool SkyDollyCsvParser::importPositionData(const CsvParser::Row &row, Aircraft &aircraft) noexcept
{
    PositionData data;
    bool ok {true};

    data.latitude = row.at(Enum::toUnderlyingType(::Index::Latitude)).toDouble(&ok);
    if (ok) {
        data.longitude = row.at(Enum::toUnderlyingType(::Index::Longitude)).toDouble(&ok);
    }
    if (ok) {
        data.altitude = row.at(Enum::toUnderlyingType(::Index::Altitude)).toDouble(&ok);
    }
    if (ok) {
        data.indicatedAltitude = row.at(Enum::toUnderlyingType(::Index::IndicatedAltitude)).toDouble(&ok);
    }
    if (ok) {
        data.pitch = row.at(Enum::toUnderlyingType(::Index::Pitch)).toDouble(&ok);
    }
    if (ok) {
        data.bank = row.at(Enum::toUnderlyingType(::Index::Bank)).toDouble(&ok);
    }
    if (ok) {
        data.trueHeading = row.at(Enum::toUnderlyingType(::Index::TrueHeading)).toDouble(&ok);
    }
    // Velocity
    if (ok) {
        data.velocityBodyX = row.at(Enum::toUnderlyingType(::Index::VelocityBodyX)).toDouble(&ok);
    }
    if (ok) {
        data.velocityBodyY = row.at(Enum::toUnderlyingType(::Index::VelocityBodyY)).toDouble(&ok);
    }
    if (ok) {
        data.velocityBodyZ = row.at(Enum::toUnderlyingType(::Index::VelocityBodyZ)).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyX = row.at(Enum::toUnderlyingType(::Index::RotationVelocityBodyX)).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyY= row.at(Enum::toUnderlyingType(::Index::RotationVelocityBodyY)).toDouble(&ok);
    }
    if (ok) {
        data.rotationVelocityBodyZ = row.at(Enum::toUnderlyingType(::Index::RotationVelocityBodyZ)).toDouble(&ok);
    }
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        aircraft.getPosition().upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importEngineData(const CsvParser::Row &row, Engine &engine) noexcept
{
    EngineData data;
    bool ok {true};

    data.throttleLeverPosition1 = row.at(Enum::toUnderlyingType(::Index::ThrottleLeverPosition1)).toInt(&ok);
    if (ok) {
        data.throttleLeverPosition2 = row.at(Enum::toUnderlyingType(::Index::ThrottleLeverPosition2)).toInt(&ok);
    }
    if (ok) {
        data.throttleLeverPosition3 = row.at(Enum::toUnderlyingType(::Index::ThrottleLeverPosition3)).toInt(&ok);
    }
    if (ok) {
        data.throttleLeverPosition4 = row.at(Enum::toUnderlyingType(::Index::ThrottleLeverPosition4)).toInt(&ok);
    }
    if (ok) {
        data.propellerLeverPosition1 = row.at(Enum::toUnderlyingType(::Index::PropellerLeverPosition1)).toInt(&ok);
    }
    if (ok) {
        data.propellerLeverPosition2 = row.at(Enum::toUnderlyingType(::Index::PropellerLeverPosition2)).toInt(&ok);
    }
    if (ok) {
        data.propellerLeverPosition3 = row.at(Enum::toUnderlyingType(::Index::PropellerLeverPosition3)).toInt(&ok);
    }
    if (ok) {
        data.propellerLeverPosition4 = row.at(Enum::toUnderlyingType(::Index::PropellerLeverPosition4)).toInt(&ok);
    }
    if (ok) {
        data.mixtureLeverPosition1 = row.at(Enum::toUnderlyingType(::Index::MixtureLeverPosition1)).toInt(&ok);
    }
    if (ok) {
        data.mixtureLeverPosition2 = row.at(Enum::toUnderlyingType(::Index::MixtureLeverPosition2)).toInt(&ok);
    }
    if (ok) {
        data.mixtureLeverPosition3 = row.at(Enum::toUnderlyingType(::Index::MixtureLeverPosition3)).toInt(&ok);
    }
    if (ok) {
        data.mixtureLeverPosition4 = row.at(Enum::toUnderlyingType(::Index::MixtureLeverPosition4)).toInt(&ok);
    }
    if (ok) {
        data.cowlFlapPosition1 = row.at(Enum::toUnderlyingType(::Index::RecipEngineCowlFlapPosition1)).toInt(&ok);
    }
    if (ok) {
        data.cowlFlapPosition2 = row.at(Enum::toUnderlyingType(::Index::RecipEngineCowlFlapPosition2)).toInt(&ok);
    }
    if (ok) {
        data.cowlFlapPosition3 = row.at(Enum::toUnderlyingType(::Index::RecipEngineCowlFlapPosition3)).toInt(&ok);
    }
    if (ok) {
        data.cowlFlapPosition4 = row.at(Enum::toUnderlyingType(::Index::RecipEngineCowlFlapPosition4)).toInt(&ok);
    }
    if (ok) {
        data.electricalMasterBattery1 = row.at(Enum::toUnderlyingType(::Index::ElectricalMasterBattery1)).toInt(&ok) != 0;
    }
    if (ok) {
        data.electricalMasterBattery2 = row.at(Enum::toUnderlyingType(::Index::ElectricalMasterBattery2)).toInt(&ok) != 0;
    }
    if (ok) {
        data.electricalMasterBattery3 = row.at(Enum::toUnderlyingType(::Index::ElectricalMasterBattery3)).toInt(&ok) != 0;
    }
    if (ok) {
        data.electricalMasterBattery4 = row.at(Enum::toUnderlyingType(::Index::ElectricalMasterBattery4)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineStarter1 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineStarter1)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineStarter2 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineStarter2)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineStarter3 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineStarter3)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineStarter4 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineStarter4)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineCombustion1 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineCombustion1)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineCombustion2 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineCombustion2)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineCombustion3 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineCombustion3)).toInt(&ok) != 0;
    }
    if (ok) {
        data.generalEngineCombustion4 = row.at(Enum::toUnderlyingType(::Index::GeneralEngineCombustion4)).toInt(&ok) != 0;
    }
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        engine.upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importPrimaryFlightControlData(const CsvParser::Row &row, PrimaryFlightControl &primaryFlightControl) noexcept
{
    PrimaryFlightControlData data;
    bool ok {true};

    data.rudderPosition = row.at(Enum::toUnderlyingType(::Index::RudderPosition)).toInt(&ok);
    if (ok) {
        data.elevatorPosition =  row.at(Enum::toUnderlyingType(::Index::ElevatorPosition)).toInt(&ok);
    }
    if (ok) {
        data.aileronPosition = row.at(Enum::toUnderlyingType(::Index::AileronPosition)).toInt(&ok);
    }
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        primaryFlightControl.upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importSecondaryFlightControlData(const CsvParser::Row &row, SecondaryFlightControl &secondaryFlightControl) noexcept
{
    SecondaryFlightControlData data;
    bool ok {true};

    data.leadingEdgeFlapsLeftPosition = row.at(Enum::toUnderlyingType(::Index::LeadingEdgeFlapsLeftPercent)).toInt(&ok);
    if (ok) {
        data.leadingEdgeFlapsRightPosition = row.at(Enum::toUnderlyingType(::Index::LeadingEdgeFlapsRightPercent)).toInt(&ok);
    }
    if (ok) {
        data.trailingEdgeFlapsLeftPosition = row.at(Enum::toUnderlyingType(::Index::TrailingEdgeFlapsLeftPercent)).toInt(&ok);
    }
    if (ok) {
        data.trailingEdgeFlapsRightPosition = row.at(Enum::toUnderlyingType(::Index::TrailingEdgeFlapsRightPercent)).toInt(&ok);
    }
    if (ok) {
        data.spoilersHandlePosition = row.at(Enum::toUnderlyingType(::Index::SpoilersHandlePosition)).toInt(&ok);
    }
    if (ok) {
        data.flapsHandleIndex = row.at(Enum::toUnderlyingType(::Index::FlapsHandleIndex)).toInt(&ok);
    }
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        secondaryFlightControl.upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importAircraftHandleData(const CsvParser::Row &row, AircraftHandle &aircraftHandle) noexcept
{
    AircraftHandleData data;;
    bool ok {true};

    data.gearHandlePosition = row.at(Enum::toUnderlyingType(::Index::GearHandlePosition)).toInt(&ok) == 1 ? true : false;
    if (ok) {
        data.brakeLeftPosition = row.at(Enum::toUnderlyingType(::Index::BrakeLeftPosition)).toInt(&ok);
    }
    if (ok) {
        data.brakeRightPosition = row.at(Enum::toUnderlyingType(::Index::BrakeRightPosition)).toInt(&ok);
    }
    if (ok) {
        data.waterRudderHandlePosition = row.at(Enum::toUnderlyingType(::Index::WaterRudderHandlePosition)).toInt(&ok);
    }
    if (ok) {
        data.tailhookPosition = row.at(Enum::toUnderlyingType(::Index::TailhookPosition)).toInt(&ok);
    }
    if (ok) {
        data.canopyOpen = row.at(Enum::toUnderlyingType(::Index::CanopyOpen)).toInt(&ok);
    }
    if (ok) {
        data.leftWingFolding = row.at(Enum::toUnderlyingType(::Index::FoldingWingLeftPercent)).toInt(&ok);
    }
    if (ok) {
        data.rightWingFolding = row.at(Enum::toUnderlyingType(::Index::FoldingWingRightPercent)).toInt(&ok);
    }
    if (ok) {
        data.smokeEnabled = row.at(Enum::toUnderlyingType(::Index::SmokeEnable)).toInt(&ok) == 1 ? true : false;
    }
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        aircraftHandle.upsertLast(data);
    }
    return ok;
}

inline bool SkyDollyCsvParser::importLightData(const CsvParser::Row &row, Light &light) noexcept
{
    LightData data;
    bool ok {true};

    data.lightStates = static_cast<SimType::LightStates>(row.at(Enum::toUnderlyingType(::Index::LightStates)).toInt(&ok));
    // Timestamp
    if (ok) {
        data.timestamp = row.at(Enum::toUnderlyingType(::Index::Timestamp)).toLongLong(&ok) - m_timestampDelta;
    }

    if (ok) {
        light.upsertLast(data);
    }
    return ok;
}
