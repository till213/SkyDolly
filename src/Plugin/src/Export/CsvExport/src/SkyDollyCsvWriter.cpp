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

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Model/src/SimVar.h"
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
#include "../../../CsvConst.h"
#include "CsvExportSettings.h"
#include "SkyDollyCsvWriter.h"

class SkyDollyCsvWriterPrivate
{
public:
    SkyDollyCsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;

    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

SkyDollyCsvWriter::SkyDollyCsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<SkyDollyCsvWriterPrivate>(pluginSettings))
{
#ifdef DEBUG
    qDebug("SkyDollyCsvWriter::SkyDollyCsvWriter: CREATED");
#endif
}

SkyDollyCsvWriter::~SkyDollyCsvWriter() noexcept
{
#ifdef DEBUG
    qDebug("SkyDollyCsvWriter::~SkyDollyCsvWriter: DELETED");
#endif
}

bool SkyDollyCsvWriter::write([[maybe_unused]] const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
    QString csv = QString(CsvConst::TypeColumnName) % CsvConst::TabSep %
                          getPositionHeader() % CsvConst::TabSep %
                          getEngineHeader() % CsvConst::TabSep %
                          getPrimaryFlightControlHeader()  % CsvConst::TabSep %
                          getSecondaryFlightControlHeader() % CsvConst::TabSep %
                          getAircraftHandleHeader() % CsvConst::TabSep %
                          getLightHeader() % CsvConst::TabSep %
                          QString(SimVar::Timestamp) % CsvConst::Ln;

    bool ok = io.write(csv.toUtf8());
    if (ok) {
        const PositionData emptyPositionData;
        const EngineData emptyEngineData;
        const PrimaryFlightControlData emptyPrimaryFlightControlData;
        const SecondaryFlightControlData emptySecondaryFlightControlData;
        const AircraftHandleData emptyAircraftHandleData;
        const LightData emptyLightData;

        const SampleRate::ResamplingPeriod resamplingPeriod = d->pluginSettings.getResamplingPeriod();
        const std::int64_t deltaTime = Enum::toUnderlyingType(resamplingPeriod);
        std::int64_t duration;
        std::int64_t timestamp;

        // Position data
        const Position &position = aircraft.getPositionConst();
        QChar dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::Aircraft));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = position.getLast().timestamp;
            timestamp = 0;
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
        const Engine &engine = aircraft.getEngineConst();
        dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::Engine));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = engine.getLast().timestamp;
            timestamp = 0;
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
        const PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControlConst();
        dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::PrimaryFlightControl));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = primaryFlightControl.getLast().timestamp;
            timestamp = 0;
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
        const SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControlConst();
        dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::SecondaryFlightControl));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = secondaryFlightControl.getLast().timestamp;
            timestamp = 0;
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
        const AircraftHandle &aircraftHandle = aircraft.getAircraftHandleConst();
        dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::AircraftHandle));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = aircraftHandle.getLast().timestamp;
            timestamp = 0;
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
        const Light &light = aircraft.getLightConst();
        dataType = QChar(Enum::toUnderlyingType(CsvConst::DataType::Light));
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            duration = light.getLast().timestamp;
            timestamp = 0;
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
    return QString(SimVar::Latitude) % CsvConst::TabSep %
           QString(SimVar::Longitude) % CsvConst::TabSep %
           QString(SimVar::Altitude) % CsvConst::TabSep %
           QString(SimVar::IndicatedAltitude) % CsvConst::TabSep %
           QString(SimVar::Pitch) % CsvConst::TabSep %
           QString(SimVar::Bank) % CsvConst::TabSep %
           QString(SimVar::Heading) % CsvConst::TabSep %
           QString(SimVar::VelocityBodyX) % CsvConst::TabSep %
           QString(SimVar::VelocityBodyY) % CsvConst::TabSep %
           QString(SimVar::VelocityBodyZ) % CsvConst::TabSep %
           QString(SimVar::RotationVelocityBodyX) % CsvConst::TabSep %
           QString(SimVar::RotationVelocityBodyY) % CsvConst::TabSep %
           QString(SimVar::RotationVelocityBodyZ);
}

inline QString SkyDollyCsvWriter::getPositionData(const PositionData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.latitude, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.longitude, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.altitude, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.indicatedAltitude, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.pitch, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.bank, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.heading, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.velocityBodyX, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.velocityBodyY, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.velocityBodyZ, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.rotationVelocityBodyX, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.rotationVelocityBodyY, CsvConst::Format, CsvConst::Precision) % CsvConst::TabSep %
              QString::number(data.rotationVelocityBodyZ, CsvConst::Format, CsvConst::Precision);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getEngineHeader() noexcept
{
    return QString(SimVar::ThrottleLeverPosition1) % CsvConst::TabSep %
           QString(SimVar::ThrottleLeverPosition2) % CsvConst::TabSep %
           QString(SimVar::ThrottleLeverPosition3) % CsvConst::TabSep %
           QString(SimVar::ThrottleLeverPosition4) % CsvConst::TabSep %
           QString(SimVar::PropellerLeverPosition1) % CsvConst::TabSep %
           QString(SimVar::PropellerLeverPosition2) % CsvConst::TabSep %
           QString(SimVar::PropellerLeverPosition3) % CsvConst::TabSep %
           QString(SimVar::PropellerLeverPosition4) % CsvConst::TabSep %
           QString(SimVar::MixtureLeverPosition1) % CsvConst::TabSep %
           QString(SimVar::MixtureLeverPosition2) % CsvConst::TabSep %
           QString(SimVar::MixtureLeverPosition3) % CsvConst::TabSep %
           QString(SimVar::MixtureLeverPosition4) % CsvConst::TabSep %
           QString(SimVar::RecipEngineCowlFlapPosition1) % CsvConst::TabSep %
           QString(SimVar::RecipEngineCowlFlapPosition2) % CsvConst::TabSep %
           QString(SimVar::RecipEngineCowlFlapPosition3) % CsvConst::TabSep %
           QString(SimVar::RecipEngineCowlFlapPosition4) % CsvConst::TabSep %
           QString(SimVar::ElectricalMasterBattery1) % CsvConst::TabSep %
           QString(SimVar::ElectricalMasterBattery2) % CsvConst::TabSep %
           QString(SimVar::ElectricalMasterBattery3) % CsvConst::TabSep %
           QString(SimVar::ElectricalMasterBattery4) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineStarter1) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineStarter2) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineStarter3) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineStarter4) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineCombustion1) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineCombustion2) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineCombustion3) % CsvConst::TabSep %
           QString(SimVar::GeneralEngineCombustion4);
}

inline QString SkyDollyCsvWriter::getEngineData(const EngineData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.throttleLeverPosition1) % CsvConst::TabSep %
              QString::number(data.throttleLeverPosition2) % CsvConst::TabSep %
              QString::number(data.throttleLeverPosition3) % CsvConst::TabSep %
              QString::number(data.throttleLeverPosition4) % CsvConst::TabSep %
              QString::number(data.propellerLeverPosition1) % CsvConst::TabSep %
              QString::number(data.propellerLeverPosition2) % CsvConst::TabSep %
              QString::number(data.propellerLeverPosition3) % CsvConst::TabSep %
              QString::number(data.propellerLeverPosition4) % CsvConst::TabSep %
              QString::number(data.mixtureLeverPosition1) % CsvConst::TabSep %
              QString::number(data.mixtureLeverPosition2) % CsvConst::TabSep %
              QString::number(data.mixtureLeverPosition3) % CsvConst::TabSep %
              QString::number(data.mixtureLeverPosition4) % CsvConst::TabSep %
              QString::number(data.cowlFlapPosition1) % CsvConst::TabSep %
              QString::number(data.cowlFlapPosition2) % CsvConst::TabSep %
              QString::number(data.cowlFlapPosition3) % CsvConst::TabSep %
              QString::number(data.cowlFlapPosition4) % CsvConst::TabSep %
              QString::number(data.electricalMasterBattery1) % CsvConst::TabSep %
              QString::number(data.electricalMasterBattery2) % CsvConst::TabSep %
              QString::number(data.electricalMasterBattery3) % CsvConst::TabSep %
              QString::number(data.electricalMasterBattery4) % CsvConst::TabSep %
              QString::number(data.generalEngineStarter1) % CsvConst::TabSep %
              QString::number(data.generalEngineStarter2) % CsvConst::TabSep %
              QString::number(data.generalEngineStarter3) % CsvConst::TabSep %
              QString::number(data.generalEngineStarter4) % CsvConst::TabSep %
              QString::number(data.generalEngineCombustion1) % CsvConst::TabSep %
              QString::number(data.generalEngineCombustion2) % CsvConst::TabSep %
              QString::number(data.generalEngineCombustion3) % CsvConst::TabSep %
              QString::number(data.generalEngineCombustion4);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::RudderPosition) % CsvConst::TabSep %
           QString(SimVar::ElevatorPosition) % CsvConst::TabSep %
           QString(SimVar::AileronPosition);
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.rudderPosition) % CsvConst::TabSep %
                QString::number(data.elevatorPosition) % CsvConst::TabSep %
                QString::number(data.aileronPosition);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % CsvConst::TabSep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % CsvConst::TabSep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % CsvConst::TabSep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % CsvConst::TabSep %
           QString(SimVar::SpoilersHandlePosition) % CsvConst::TabSep %
           QString(SimVar::FlapsHandleIndex);
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leadingEdgeFlapsLeftPosition) % CsvConst::TabSep %
              QString::number(data.leadingEdgeFlapsRightPosition) % CsvConst::TabSep %
              QString::number(data.trailingEdgeFlapsLeftPosition) % CsvConst::TabSep %
              QString::number(data.trailingEdgeFlapsRightPosition) % CsvConst::TabSep %
              QString::number(data.spoilersHandlePosition) % CsvConst::TabSep %
              QString::number(data.flapsHandleIndex);
    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % CsvConst::TabSep %
           QString(SimVar::BrakeLeftPosition) % CsvConst::TabSep %
           QString(SimVar::BrakeRightPosition) % CsvConst::TabSep %
           QString(SimVar::WaterRudderHandlePosition) % CsvConst::TabSep %
           QString(SimVar::TailhookPosition) % CsvConst::TabSep %
           QString(SimVar::FoldingWingLeftPercent)  % CsvConst::TabSep %
           QString(SimVar::FoldingWingRightPercent)  % CsvConst::TabSep %
           QString(SimVar::CanopyOpen) % CsvConst::TabSep %
           QString(SimVar::SmokeEnable);
}

inline QString SkyDollyCsvWriter::getAircraftHandleData(const AircraftHandleData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.gearHandlePosition) % CsvConst::TabSep %
              QString::number(data.brakeLeftPosition) % CsvConst::TabSep %
              QString::number(data.brakeRightPosition) % CsvConst::TabSep %
              QString::number(data.waterRudderHandlePosition) % CsvConst::TabSep %
              QString::number(data.tailhookPosition) % CsvConst::TabSep %
              QString::number(data.leftWingFolding) % CsvConst::TabSep %
              QString::number(data.rightWingFolding) % CsvConst::TabSep %
              QString::number(data.canopyOpen) % CsvConst::TabSep %
              QString::number(data.smokeEnabled);

    } else {
        const QString EmptyString;
        csv = EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
              EmptyString % CsvConst::TabSep %
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
    const QString csv = type % CsvConst::TabSep %
                        getPositionData(positionData) % CsvConst::TabSep %
                        getEngineData(engineData) % CsvConst::TabSep %
                        getPrimaryFlightControlData(primaryFlightControlData) % CsvConst::TabSep %
                        getSecondaryFlightControlData(secondaryFlightControlData) % CsvConst::TabSep %
                        getAircraftHandleData(aircraftHandleData) % CsvConst::TabSep %
                        getLightData(lightData) % CsvConst::TabSep %
                        QString::number(timestamp) % CsvConst::Ln;
    return io.write(csv.toUtf8());
}
