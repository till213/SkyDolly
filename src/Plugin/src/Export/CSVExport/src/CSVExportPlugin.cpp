/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation ios (the "Software"), to deal in the Software
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
#include <QWidget>
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Settings.h"
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
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../CSVConst.h"
#include "../../../Export.h"
#include "CSVExportSettings.h"
#include "CSVExportPlugin.h"

class CSVExportPluginPrivate
{
public:
    CSVExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    Flight &flight;
    CSVExportSettings settings;

    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

CSVExportPlugin::CSVExportPlugin() noexcept
    : d(std::make_unique<CSVExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("CSVExportPlugin::CSVExportPlugin: PLUGIN LOADED");
#endif
}

CSVExportPlugin::~CSVExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("CSVExportPlugin::~CSVExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ExportPluginBaseSettings &CSVExportPlugin::getSettings() const noexcept
{
    return d->settings;
}

QString CSVExportPlugin::getFileExtension() const noexcept
{
    return CSVExportPluginPrivate::FileExtension;
}

QString CSVExportPlugin::getFileFilter() const noexcept
{
    return tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> CSVExportPlugin::createOptionWidget() const noexcept
{
    // No plugin-specific settings yet
    return nullptr;
}

bool CSVExportPlugin::writeFile(QIODevice &io) noexcept
{
    bool ok;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraftConst();
    io.setTextModeEnabled(true);

    QString csv = QString(CSVConst::TypeColumnName) % CSVConst::Sep %
                          getPositionHeader() % CSVConst::Sep %
                          getEngineHeader() % CSVConst::Sep %
                          getPrimaryFlightControlHeader()  % CSVConst::Sep %
                          getSecondaryFlightControlHeader() % CSVConst::Sep %
                          getAircraftHandleHeader() % CSVConst::Sep %
                          getLightHeader() % CSVConst::Sep %
                          QString(SimVar::Timestamp) % CSVConst::Ln;

    ok = io.write(csv.toUtf8());
    if (ok) {
        const PositionData emptyPositionData;
        const EngineData emptyEngineData;
        const PrimaryFlightControlData emptyPrimaryFlightControlData;
        const SecondaryFlightControlData emptySecondaryFlightControlData;
        const AircraftHandleData emptyAircraftHandleData;
        const LightData emptyLightData;

        const SampleRate::ResamplingPeriod resamplingPeriod = d->settings.getResamplingPeriod();
        const std::int64_t deltaTime = Enum::toUnderlyingType(resamplingPeriod);
        std::int64_t duration;
        std::int64_t timestamp;

        // Position data
        const Position &position = aircraft.getPositionConst();
        QChar dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::Aircraft));
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
        dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::Engine));
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
        dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::PrimaryFlightControl));
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
        dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::SecondaryFlightControl));
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
        dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::AircraftHandle));
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
        dataType = QChar(Enum::toUnderlyingType(CSVConst::DataType::Light));
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

// PROTECTED SLOTS

void CSVExportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

inline QString CSVExportPlugin::getPositionHeader() noexcept
{
    return QString(SimVar::Latitude) % CSVConst::Sep %
           QString(SimVar::Longitude) % CSVConst::Sep %
           QString(SimVar::Altitude) % CSVConst::Sep %
           QString(SimVar::IndicatedAltitude) % CSVConst::Sep %
           QString(SimVar::Pitch) % CSVConst::Sep %
           QString(SimVar::Bank) % CSVConst::Sep %
           QString(SimVar::Heading) % CSVConst::Sep %
           QString(SimVar::VelocityBodyX) % CSVConst::Sep %
           QString(SimVar::VelocityBodyY) % CSVConst::Sep %
           QString(SimVar::VelocityBodyZ) % CSVConst::Sep %
           QString(SimVar::RotationVelocityBodyX) % CSVConst::Sep %
           QString(SimVar::RotationVelocityBodyY) % CSVConst::Sep %
           QString(SimVar::RotationVelocityBodyZ);
}

inline QString CSVExportPlugin::getPositionData(const PositionData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.latitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.longitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.altitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.indicatedAltitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.pitch, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.bank, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.heading, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.velocityBodyX, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.velocityBodyY, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.velocityBodyZ, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.rotationVelocityBodyX, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.rotationVelocityBodyY, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.rotationVelocityBodyZ, CSVConst::Format, CSVConst::Precision);
    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExportPlugin::getEngineHeader() noexcept
{
    return QString(SimVar::ThrottleLeverPosition1) % CSVConst::Sep %
           QString(SimVar::ThrottleLeverPosition2) % CSVConst::Sep %
           QString(SimVar::ThrottleLeverPosition3) % CSVConst::Sep %
           QString(SimVar::ThrottleLeverPosition4) % CSVConst::Sep %
           QString(SimVar::PropellerLeverPosition1) % CSVConst::Sep %
           QString(SimVar::PropellerLeverPosition2) % CSVConst::Sep %
           QString(SimVar::PropellerLeverPosition3) % CSVConst::Sep %
           QString(SimVar::PropellerLeverPosition4) % CSVConst::Sep %
           QString(SimVar::MixtureLeverPosition1) % CSVConst::Sep %
           QString(SimVar::MixtureLeverPosition2) % CSVConst::Sep %
           QString(SimVar::MixtureLeverPosition3) % CSVConst::Sep %
           QString(SimVar::MixtureLeverPosition4) % CSVConst::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition1) % CSVConst::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition2) % CSVConst::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition3) % CSVConst::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition4) % CSVConst::Sep %
           QString(SimVar::ElectricalMasterBattery1) % CSVConst::Sep %
           QString(SimVar::ElectricalMasterBattery2) % CSVConst::Sep %
           QString(SimVar::ElectricalMasterBattery3) % CSVConst::Sep %
           QString(SimVar::ElectricalMasterBattery4) % CSVConst::Sep %
           QString(SimVar::GeneralEngineStarter1) % CSVConst::Sep %
           QString(SimVar::GeneralEngineStarter2) % CSVConst::Sep %
           QString(SimVar::GeneralEngineStarter3) % CSVConst::Sep %
           QString(SimVar::GeneralEngineStarter4) % CSVConst::Sep %
           QString(SimVar::GeneralEngineCombustion1) % CSVConst::Sep %
           QString(SimVar::GeneralEngineCombustion2) % CSVConst::Sep %
           QString(SimVar::GeneralEngineCombustion3) % CSVConst::Sep %
           QString(SimVar::GeneralEngineCombustion4);
}

inline QString CSVExportPlugin::getEngineData(const EngineData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.throttleLeverPosition1) % CSVConst::Sep %
              QString::number(data.throttleLeverPosition2) % CSVConst::Sep %
              QString::number(data.throttleLeverPosition3) % CSVConst::Sep %
              QString::number(data.throttleLeverPosition4) % CSVConst::Sep %
              QString::number(data.propellerLeverPosition1) % CSVConst::Sep %
              QString::number(data.propellerLeverPosition2) % CSVConst::Sep %
              QString::number(data.propellerLeverPosition3) % CSVConst::Sep %
              QString::number(data.propellerLeverPosition4) % CSVConst::Sep %
              QString::number(data.mixtureLeverPosition1) % CSVConst::Sep %
              QString::number(data.mixtureLeverPosition2) % CSVConst::Sep %
              QString::number(data.mixtureLeverPosition3) % CSVConst::Sep %
              QString::number(data.mixtureLeverPosition4) % CSVConst::Sep %
              QString::number(data.cowlFlapPosition1) % CSVConst::Sep %
              QString::number(data.cowlFlapPosition2) % CSVConst::Sep %
              QString::number(data.cowlFlapPosition3) % CSVConst::Sep %
              QString::number(data.cowlFlapPosition4) % CSVConst::Sep %
              QString::number(data.electricalMasterBattery1) % CSVConst::Sep %
              QString::number(data.electricalMasterBattery2) % CSVConst::Sep %
              QString::number(data.electricalMasterBattery3) % CSVConst::Sep %
              QString::number(data.electricalMasterBattery4) % CSVConst::Sep %
              QString::number(data.generalEngineStarter1) % CSVConst::Sep %
              QString::number(data.generalEngineStarter2) % CSVConst::Sep %
              QString::number(data.generalEngineStarter3) % CSVConst::Sep %
              QString::number(data.generalEngineStarter4) % CSVConst::Sep %
              QString::number(data.generalEngineCombustion1) % CSVConst::Sep %
              QString::number(data.generalEngineCombustion2) % CSVConst::Sep %
              QString::number(data.generalEngineCombustion3) % CSVConst::Sep %
              QString::number(data.generalEngineCombustion4);
    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExportPlugin::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::RudderPosition) % CSVConst::Sep %
           QString(SimVar::ElevatorPosition) % CSVConst::Sep %
           QString(SimVar::AileronPosition);
}

inline QString CSVExportPlugin::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.rudderPosition) % CSVConst::Sep %
                QString::number(data.elevatorPosition) % CSVConst::Sep %
                QString::number(data.aileronPosition);
    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExportPlugin::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % CSVConst::Sep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % CSVConst::Sep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % CSVConst::Sep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % CSVConst::Sep %
           QString(SimVar::SpoilersHandlePosition) % CSVConst::Sep %
           QString(SimVar::FlapsHandleIndex);
}

inline QString CSVExportPlugin::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leadingEdgeFlapsLeftPosition) % CSVConst::Sep %
              QString::number(data.leadingEdgeFlapsRightPosition) % CSVConst::Sep %
              QString::number(data.trailingEdgeFlapsLeftPosition) % CSVConst::Sep %
              QString::number(data.trailingEdgeFlapsRightPosition) % CSVConst::Sep %
              QString::number(data.spoilersHandlePosition) % CSVConst::Sep %
              QString::number(data.flapsHandleIndex);
    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExportPlugin::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % CSVConst::Sep %
           QString(SimVar::BrakeLeftPosition) % CSVConst::Sep %
           QString(SimVar::BrakeRightPosition) % CSVConst::Sep %
           QString(SimVar::WaterRudderHandlePosition) % CSVConst::Sep %
           QString(SimVar::TailhookPosition) % CSVConst::Sep %
           QString(SimVar::FoldingWingLeftPercent)  % CSVConst::Sep %
           QString(SimVar::FoldingWingRightPercent)  % CSVConst::Sep %
           QString(SimVar::CanopyOpen) % CSVConst::Sep %
           QString(SimVar::SmokeEnable);
}

inline QString CSVExportPlugin::getAircraftHandleData(const AircraftHandleData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.gearHandlePosition) % CSVConst::Sep %
              QString::number(data.brakeLeftPosition) % CSVConst::Sep %
              QString::number(data.brakeRightPosition) % CSVConst::Sep %
              QString::number(data.waterRudderHandlePosition) % CSVConst::Sep %
              QString::number(data.tailhookPosition) % CSVConst::Sep %
              QString::number(data.leftWingFolding) % CSVConst::Sep %
              QString::number(data.rightWingFolding) % CSVConst::Sep %
              QString::number(data.canopyOpen) % CSVConst::Sep %
              QString::number(data.smokeEnabled);

    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString % CSVConst::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExportPlugin::getLightHeader() noexcept
{
    return QString(SimVar::LightStates);
}

inline QString CSVExportPlugin::getLightData(const LightData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.lightStates);
    } else {
        csv = QString();
    }
    return csv;
}

inline bool CSVExportPlugin::writeLine(QChar type,
                                       const PositionData &positionData,
                                       const EngineData &engineData,
                                       const PrimaryFlightControlData &primaryFlightControlData,
                                       const SecondaryFlightControlData &secondaryFlightControlData,
                                       const AircraftHandleData &aircraftHandleData,
                                       const LightData &lightData,
                                       std::int64_t timestamp,
                                       QIODevice &io) noexcept
{
    const QString csv = type % CSVConst::Sep %
                        getPositionData(positionData) % CSVConst::Sep %
                        getEngineData(engineData) % CSVConst::Sep %
                        getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                        getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                        getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                        getLightData(lightData) % CSVConst::Sep %
                        QString::number(timestamp) % CSVConst::Ln;
    return io.write(csv.toUtf8());
}
