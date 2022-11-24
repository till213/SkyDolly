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

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Enum.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Unit.h>
#include <Model/SimVar.h>
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
#include <PluginManager/Export.h>
#include "CsvExportSettings.h"
#include "SkyDollyCsvWriter.h"

struct SkyDollyCsvWriterPrivate
{
    SkyDollyCsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;
};

// PUBLIC

SkyDollyCsvWriter::SkyDollyCsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<SkyDollyCsvWriterPrivate>(pluginSettings))
{}

SkyDollyCsvWriter::~SkyDollyCsvWriter() noexcept = default;

bool SkyDollyCsvWriter::write([[maybe_unused]] const Flight &flight, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QString header = QString(CsvConst::TypeColumnName) % CsvConst::CommaSep %
                                   getPositionHeader() % CsvConst::CommaSep %
                                   getEngineHeader() % CsvConst::CommaSep %
                                   getPrimaryFlightControlHeader()  % CsvConst::CommaSep %
                                   getSecondaryFlightControlHeader() % CsvConst::CommaSep %
                                   getAircraftHandleHeader() % CsvConst::CommaSep %
                                   getLightHeader() % CsvConst::CommaSep %
                                   QString(SimVar::Timestamp) % CsvConst::Ln;

    bool ok = io.write(header.toUtf8());
    if (ok) {
        const PositionData emptyPositionData;
        const EngineData emptyEngineData;
        const PrimaryFlightControlData emptyPrimaryFlightControlData;
        const SecondaryFlightControlData emptySecondaryFlightControlData;
        const AircraftHandleData emptyAircraftHandleData;
        const LightData emptyLightData;

        const SampleRate::ResamplingPeriod resamplingPeriod = d->pluginSettings.getResamplingPeriod();
        const std::int64_t deltaTime = Enum::underly(resamplingPeriod);

        // Position data
        const Position &position = aircraft.getPosition();
        QChar dataType = QChar(Enum::underly(CsvConst::DataType::Aircraft));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = position.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const PositionData &positionData = position.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!positionData.isNull()) {
                    ok = writeLine(dataType, positionData, emptyEngineData,
                                   emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                                   emptyAircraftHandleData, emptyLightData,
                                   positionData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const PositionData &positionData : position) {
                ok = writeLine(dataType, positionData, emptyEngineData,
                               emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                               emptyAircraftHandleData, emptyLightData,
                               positionData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }

        // Engine data
        Engine &engine = aircraft.getEngine();
        dataType = QChar(Enum::underly(CsvConst::DataType::Engine));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = engine.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const EngineData &engineData = engine.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!engineData.isNull()) {
                    ok = writeLine(dataType, emptyPositionData, engineData,
                                   emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                                   emptyAircraftHandleData, emptyLightData,
                                   engineData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const EngineData &engineData : engine) {
                ok = writeLine(dataType, emptyPositionData, engineData,
                               emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                               emptyAircraftHandleData, emptyLightData,
                               engineData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }

        // Primary flight controls
        PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControl();
        dataType = QChar(Enum::underly(CsvConst::DataType::PrimaryFlightControl));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = primaryFlightControl.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const PrimaryFlightControlData &primaryFlightControlData = primaryFlightControl.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!primaryFlightControlData.isNull()) {
                    ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                                   primaryFlightControlData, emptySecondaryFlightControlData,
                                   emptyAircraftHandleData, emptyLightData,
                                   primaryFlightControlData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const PrimaryFlightControlData &primaryFlightControlData : primaryFlightControl) {
                ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                               primaryFlightControlData, emptySecondaryFlightControlData,
                               emptyAircraftHandleData, emptyLightData,
                               primaryFlightControlData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }

        // Secondary flight controls
        SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
        dataType = QChar(Enum::underly(CsvConst::DataType::SecondaryFlightControl));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = secondaryFlightControl.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const SecondaryFlightControlData &secondaryFlightControlData = secondaryFlightControl.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!secondaryFlightControlData.isNull()) {
                    ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                                   emptyPrimaryFlightControlData, secondaryFlightControlData,
                                   emptyAircraftHandleData, emptyLightData,
                                   secondaryFlightControlData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const SecondaryFlightControlData &secondaryFlightControlData : secondaryFlightControl) {
                ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                               emptyPrimaryFlightControlData, secondaryFlightControlData,
                               emptyAircraftHandleData, emptyLightData,
                               secondaryFlightControlData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }

        // Aircraft handles
        AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
        dataType = QChar(Enum::underly(CsvConst::DataType::AircraftHandle));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = aircraftHandle.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const AircraftHandleData &aircraftHandleData = aircraftHandle.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!aircraftHandleData.isNull()) {
                    ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                                   emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                                   aircraftHandleData, emptyLightData,
                                   aircraftHandleData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const AircraftHandleData &aircraftHandleData : aircraftHandle) {
                ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                               emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                               aircraftHandleData, emptyLightData,
                               aircraftHandleData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }

        // Lights
        Light &light = aircraft.getLight();
        dataType = QChar(Enum::underly(CsvConst::DataType::Light));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            std::int64_t duration = light.getLast().timestamp;
            std::int64_t timestamp = 0;
            while (ok && timestamp <= duration) {
                const LightData &lightData = light.interpolate(timestamp, TimeVariableData::Access::Linear);
                if (!lightData.isNull()) {
                    ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                                   emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                                   emptyAircraftHandleData, lightData,
                                   lightData.timestamp, io);
                    if (!ok) {
                        break;
                    }
                }
                timestamp += deltaTime;
            }
        } else {
            for (const LightData &lightData : light) {
                ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                               emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                               emptyAircraftHandleData, lightData,
                               lightData.timestamp, io);
                if (!ok) {
                    break;
                }
            }
        }
    }

    return ok;
}

// PRIVATE

inline QString SkyDollyCsvWriter::getPositionHeader() noexcept
{
    return QString(SimVar::Latitude) % CsvConst::CommaSep %
           QString(SimVar::Longitude) % CsvConst::CommaSep %
           QString(SimVar::Altitude) % CsvConst::CommaSep %
           QString(SimVar::IndicatedAltitude) % CsvConst::CommaSep %
           QString(SimVar::Pitch) % CsvConst::CommaSep %
           QString(SimVar::Bank) % CsvConst::CommaSep %
           QString(SimVar::TrueHeading) % CsvConst::CommaSep %
           QString(SimVar::VelocityBodyX) % CsvConst::CommaSep %
           QString(SimVar::VelocityBodyY) % CsvConst::CommaSep %
           QString(SimVar::VelocityBodyZ) % CsvConst::CommaSep %
           QString(SimVar::RotationVelocityBodyX) % CsvConst::CommaSep %
           QString(SimVar::RotationVelocityBodyY) % CsvConst::CommaSep %
           QString(SimVar::RotationVelocityBodyZ);
}

inline QString SkyDollyCsvWriter::getPositionData(const PositionData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = Export::formatCoordinate(data.latitude) % CsvConst::CommaSep %
              Export::formatCoordinate(data.longitude) % CsvConst::CommaSep %
              Export::formatNumber(data.altitude) % CsvConst::CommaSep %
              Export::formatNumber(data.indicatedAltitude) % CsvConst::CommaSep %
              Export::formatNumber(data.pitch) % CsvConst::CommaSep %
              Export::formatNumber(data.bank) % CsvConst::CommaSep %
              Export::formatNumber(data.trueHeading) % CsvConst::CommaSep %
              Export::formatNumber(data.velocityBodyX) % CsvConst::CommaSep %
              Export::formatNumber(data.velocityBodyY) % CsvConst::CommaSep %
              Export::formatNumber(data.velocityBodyZ) % CsvConst::CommaSep %
              Export::formatNumber(data.rotationVelocityBodyX) % CsvConst::CommaSep %
              Export::formatNumber(data.rotationVelocityBodyY) % CsvConst::CommaSep %
              Export::formatNumber(data.rotationVelocityBodyZ);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getEngineHeader() noexcept
{
    return QString(SimVar::ThrottleLeverPosition1) % CsvConst::CommaSep %
           QString(SimVar::ThrottleLeverPosition2) % CsvConst::CommaSep %
           QString(SimVar::ThrottleLeverPosition3) % CsvConst::CommaSep %
           QString(SimVar::ThrottleLeverPosition4) % CsvConst::CommaSep %
           QString(SimVar::PropellerLeverPosition1) % CsvConst::CommaSep %
           QString(SimVar::PropellerLeverPosition2) % CsvConst::CommaSep %
           QString(SimVar::PropellerLeverPosition3) % CsvConst::CommaSep %
           QString(SimVar::PropellerLeverPosition4) % CsvConst::CommaSep %
           QString(SimVar::MixtureLeverPosition1) % CsvConst::CommaSep %
           QString(SimVar::MixtureLeverPosition2) % CsvConst::CommaSep %
           QString(SimVar::MixtureLeverPosition3) % CsvConst::CommaSep %
           QString(SimVar::MixtureLeverPosition4) % CsvConst::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition1) % CsvConst::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition2) % CsvConst::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition3) % CsvConst::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition4) % CsvConst::CommaSep %
           QString(SimVar::ElectricalMasterBattery1) % CsvConst::CommaSep %
           QString(SimVar::ElectricalMasterBattery2) % CsvConst::CommaSep %
           QString(SimVar::ElectricalMasterBattery3) % CsvConst::CommaSep %
           QString(SimVar::ElectricalMasterBattery4) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineStarter1) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineStarter2) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineStarter3) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineStarter4) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineCombustion1) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineCombustion2) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineCombustion3) % CsvConst::CommaSep %
           QString(SimVar::GeneralEngineCombustion4);
}

inline QString SkyDollyCsvWriter::getEngineData(const EngineData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.throttleLeverPosition1) % CsvConst::CommaSep %
              QString::number(data.throttleLeverPosition2) % CsvConst::CommaSep %
              QString::number(data.throttleLeverPosition3) % CsvConst::CommaSep %
              QString::number(data.throttleLeverPosition4) % CsvConst::CommaSep %
              QString::number(data.propellerLeverPosition1) % CsvConst::CommaSep %
              QString::number(data.propellerLeverPosition2) % CsvConst::CommaSep %
              QString::number(data.propellerLeverPosition3) % CsvConst::CommaSep %
              QString::number(data.propellerLeverPosition4) % CsvConst::CommaSep %
              QString::number(data.mixtureLeverPosition1) % CsvConst::CommaSep %
              QString::number(data.mixtureLeverPosition2) % CsvConst::CommaSep %
              QString::number(data.mixtureLeverPosition3) % CsvConst::CommaSep %
              QString::number(data.mixtureLeverPosition4) % CsvConst::CommaSep %
              QString::number(data.cowlFlapPosition1) % CsvConst::CommaSep %
              QString::number(data.cowlFlapPosition2) % CsvConst::CommaSep %
              QString::number(data.cowlFlapPosition3) % CsvConst::CommaSep %
              QString::number(data.cowlFlapPosition4) % CsvConst::CommaSep %
              QString::number(data.electricalMasterBattery1) % CsvConst::CommaSep %
              QString::number(data.electricalMasterBattery2) % CsvConst::CommaSep %
              QString::number(data.electricalMasterBattery3) % CsvConst::CommaSep %
              QString::number(data.electricalMasterBattery4) % CsvConst::CommaSep %
              QString::number(data.generalEngineStarter1) % CsvConst::CommaSep %
              QString::number(data.generalEngineStarter2) % CsvConst::CommaSep %
              QString::number(data.generalEngineStarter3) % CsvConst::CommaSep %
              QString::number(data.generalEngineStarter4) % CsvConst::CommaSep %
              QString::number(data.generalEngineCombustion1) % CsvConst::CommaSep %
              QString::number(data.generalEngineCombustion2) % CsvConst::CommaSep %
              QString::number(data.generalEngineCombustion3) % CsvConst::CommaSep %
              QString::number(data.generalEngineCombustion4);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::RudderPosition) % CsvConst::CommaSep %
           QString(SimVar::ElevatorPosition) % CsvConst::CommaSep %
           QString(SimVar::AileronPosition);
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.rudderPosition) % CsvConst::CommaSep %
              QString::number(data.elevatorPosition) % CsvConst::CommaSep %
              QString::number(data.aileronPosition);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % CsvConst::CommaSep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % CsvConst::CommaSep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % CsvConst::CommaSep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % CsvConst::CommaSep %
           QString(SimVar::SpoilersHandlePosition) % CsvConst::CommaSep %
           QString(SimVar::FlapsHandleIndex);
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leadingEdgeFlapsLeftPosition) % CsvConst::CommaSep %
              QString::number(data.leadingEdgeFlapsRightPosition) % CsvConst::CommaSep %
              QString::number(data.trailingEdgeFlapsLeftPosition) % CsvConst::CommaSep %
              QString::number(data.trailingEdgeFlapsRightPosition) % CsvConst::CommaSep %
              QString::number(data.spoilersHandlePosition) % CsvConst::CommaSep %
              QString::number(data.flapsHandleIndex);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % CsvConst::CommaSep %
           QString(SimVar::BrakeLeftPosition) % CsvConst::CommaSep %
           QString(SimVar::BrakeRightPosition) % CsvConst::CommaSep %
           QString(SimVar::WaterRudderHandlePosition) % CsvConst::CommaSep %
           QString(SimVar::TailhookPosition) % CsvConst::CommaSep %
           QString(SimVar::FoldingWingLeftPercent)  % CsvConst::CommaSep %
           QString(SimVar::FoldingWingRightPercent)  % CsvConst::CommaSep %
           QString(SimVar::CanopyOpen) % CsvConst::CommaSep %
           QString(SimVar::SmokeEnable);
}

inline QString SkyDollyCsvWriter::getAircraftHandleData(const AircraftHandleData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.gearHandlePosition) % CsvConst::CommaSep %
              QString::number(data.brakeLeftPosition) % CsvConst::CommaSep %
              QString::number(data.brakeRightPosition) % CsvConst::CommaSep %
              QString::number(data.waterRudderHandlePosition) % CsvConst::CommaSep %
              QString::number(data.tailhookPosition) % CsvConst::CommaSep %
              QString::number(data.leftWingFolding) % CsvConst::CommaSep %
              QString::number(data.rightWingFolding) % CsvConst::CommaSep %
              QString::number(data.canopyOpen) % CsvConst::CommaSep %
              QString::number(data.smokeEnabled);

    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString % CsvConst::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getLightHeader() noexcept
{
    return QString(SimVar::LightStates);
}

inline QString SkyDollyCsvWriter::getLightData(const LightData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.lightStates);
    } else {
        csv = QString();
    }
    return csv;
}

inline bool SkyDollyCsvWriter::writeLine(QChar type,
                                       const PositionData &positionData,
                                       const EngineData &engineData,
                                       const PrimaryFlightControlData &primaryFlightControlData,
                                       const SecondaryFlightControlData &secondaryFlightControlData,
                                       const AircraftHandleData &aircraftHandleData,
                                       const LightData &lightData,
                                       std::int64_t timestamp,
                                       QIODevice &io) noexcept
{
    const QString csv = type % CsvConst::CommaSep %
                        getPositionData(positionData) % CsvConst::CommaSep %
                        getEngineData(engineData) % CsvConst::CommaSep %
                        getPrimaryFlightControlData(primaryFlightControlData) % CsvConst::CommaSep %
                        getSecondaryFlightControlData(secondaryFlightControlData) % CsvConst::CommaSep %
                        getAircraftHandleData(aircraftHandleData) % CsvConst::CommaSep %
                        getLightData(lightData) % CsvConst::CommaSep %
                        QString::number(timestamp) % CsvConst::Ln;
    return io.write(csv.toUtf8());
}
