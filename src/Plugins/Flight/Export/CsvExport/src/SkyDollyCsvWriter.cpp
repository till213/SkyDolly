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
#include <Model/PositionData.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandleData.h>
#include <Model/LightData.h>
#include <PluginManager/Csv.h>
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

SkyDollyCsvWriter::~SkyDollyCsvWriter() = default;

bool SkyDollyCsvWriter::write([[maybe_unused]] const Flight &flight, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QString header = QString(Csv::TypeColumnName) % Csv::CommaSep %
                                   getPositionHeader() % Csv::CommaSep %
                                   getEngineHeader() % Csv::CommaSep %
                                   getPrimaryFlightControlHeader()  % Csv::CommaSep %
                                   getSecondaryFlightControlHeader() % Csv::CommaSep %
                                   getAircraftHandleHeader() % Csv::CommaSep %
                                   getLightHeader() % Csv::CommaSep %
                                   QString(SimVar::Timestamp) % Csv::Ln;

    bool ok = io.write(header.toUtf8());
    if (ok) {
        const PositionData emptyPositionData;
        const EngineData emptyEngineData;
        const PrimaryFlightControlData emptyPrimaryFlightControlData;
        const SecondaryFlightControlData emptySecondaryFlightControlData;
        const AircraftHandleData emptyAircraftHandleData;
        const LightData emptyLightData;

        // Position data
        auto dataType = QChar(Enum::underly(Csv::DataType::Aircraft));
        const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const PositionData &positionData : interpolatedPositionData) {
            ok = writeLine(dataType, positionData, emptyEngineData,
                           emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                           emptyAircraftHandleData, emptyLightData,
                           positionData.timestamp, io);
            if (!ok) {
                break;
            }
        }

        // Engine data
        dataType = QChar(Enum::underly(Csv::DataType::Engine));
        const std::vector<EngineData> interpolatedEngineData = Export::resampleEngineDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const EngineData &engineData : interpolatedEngineData) {
            ok = writeLine(dataType, emptyPositionData, engineData,
                           emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                           emptyAircraftHandleData, emptyLightData,
                           engineData.timestamp, io);
            if (!ok) {
                break;
            }
        }

        // Primary flight controls
        dataType = QChar(Enum::underly(Csv::DataType::PrimaryFlightControl));
        const std::vector<PrimaryFlightControlData> interpolatedPrimaryFlightControlData = Export::resamplePrimaryFlightControlDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const PrimaryFlightControlData &primaryFlightControlData : interpolatedPrimaryFlightControlData) {
            ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                           primaryFlightControlData, emptySecondaryFlightControlData,
                           emptyAircraftHandleData, emptyLightData,
                           primaryFlightControlData.timestamp, io);
            if (!ok) {
                break;
            }
        }

        // Primary flight controls
        dataType = QChar(Enum::underly(Csv::DataType::SecondaryFlightControl));
        const std::vector<SecondaryFlightControlData> interpolatedSecondaryFlightControlData = Export::resampleSecondaryFlightControlDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const SecondaryFlightControlData &secondaryFlightControlData : interpolatedSecondaryFlightControlData) {
            ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                           emptyPrimaryFlightControlData, secondaryFlightControlData,
                           emptyAircraftHandleData, emptyLightData,
                           secondaryFlightControlData.timestamp, io);
            if (!ok) {
                break;
            }
        }

        // Aircraft handles
        dataType = QChar(Enum::underly(Csv::DataType::AircraftHandle));
        const std::vector<AircraftHandleData> interpolatedAircraftHandleData = Export::resampleAircraftHandleDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const AircraftHandleData &aircraftHandleData : interpolatedAircraftHandleData) {
            ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                           emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                           aircraftHandleData, emptyLightData,
                           aircraftHandleData.timestamp, io);
            if (!ok) {
                break;
            }
        }

        // Lights
        dataType = QChar(Enum::underly(Csv::DataType::Light));
        const std::vector<LightData> interpolatedLightData = Export::resampleLightDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const LightData &lightData : interpolatedLightData) {
            ok = writeLine(dataType, emptyPositionData, emptyEngineData,
                           emptyPrimaryFlightControlData, emptySecondaryFlightControlData,
                           emptyAircraftHandleData, lightData,
                           lightData.timestamp, io);
            if (!ok) {
                break;
            }
        }
    }

    return ok;
}

// PRIVATE

inline QString SkyDollyCsvWriter::getPositionHeader() noexcept
{
    return QString(SimVar::Latitude) % Csv::CommaSep %
           QString(SimVar::Longitude) % Csv::CommaSep %
           QString(SimVar::Altitude) % Csv::CommaSep %
           QString(SimVar::IndicatedAltitude) % Csv::CommaSep %
           QString(SimVar::Pitch) % Csv::CommaSep %
           QString(SimVar::Bank) % Csv::CommaSep %
           QString(SimVar::TrueHeading) % Csv::CommaSep %
           QString(SimVar::VelocityBodyX) % Csv::CommaSep %
           QString(SimVar::VelocityBodyY) % Csv::CommaSep %
           QString(SimVar::VelocityBodyZ);
}

inline QString SkyDollyCsvWriter::getPositionData(const PositionData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = Export::formatCoordinate(data.latitude) % Csv::CommaSep %
              Export::formatCoordinate(data.longitude) % Csv::CommaSep %
              Export::formatNumber(data.altitude) % Csv::CommaSep %
              Export::formatNumber(data.indicatedAltitude) % Csv::CommaSep %
              Export::formatNumber(data.pitch) % Csv::CommaSep %
              Export::formatNumber(data.bank) % Csv::CommaSep %
              Export::formatNumber(data.trueHeading) % Csv::CommaSep %
              Export::formatNumber(data.velocityBodyX) % Csv::CommaSep %
              Export::formatNumber(data.velocityBodyY) % Csv::CommaSep %
              Export::formatNumber(data.velocityBodyZ);
    } else {
        static const QString EmptyString;
        csv = EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getEngineHeader() noexcept
{
    return QString(SimVar::ThrottleLeverPosition1) % Csv::CommaSep %
           QString(SimVar::ThrottleLeverPosition2) % Csv::CommaSep %
           QString(SimVar::ThrottleLeverPosition3) % Csv::CommaSep %
           QString(SimVar::ThrottleLeverPosition4) % Csv::CommaSep %
           QString(SimVar::PropellerLeverPosition1) % Csv::CommaSep %
           QString(SimVar::PropellerLeverPosition2) % Csv::CommaSep %
           QString(SimVar::PropellerLeverPosition3) % Csv::CommaSep %
           QString(SimVar::PropellerLeverPosition4) % Csv::CommaSep %
           QString(SimVar::MixtureLeverPosition1) % Csv::CommaSep %
           QString(SimVar::MixtureLeverPosition2) % Csv::CommaSep %
           QString(SimVar::MixtureLeverPosition3) % Csv::CommaSep %
           QString(SimVar::MixtureLeverPosition4) % Csv::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition1) % Csv::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition2) % Csv::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition3) % Csv::CommaSep %
           QString(SimVar::RecipEngineCowlFlapPosition4) % Csv::CommaSep %
           QString(SimVar::ElectricalMasterBattery1) % Csv::CommaSep %
           QString(SimVar::ElectricalMasterBattery2) % Csv::CommaSep %
           QString(SimVar::ElectricalMasterBattery3) % Csv::CommaSep %
           QString(SimVar::ElectricalMasterBattery4) % Csv::CommaSep %
           QString(SimVar::GeneralEngineStarter1) % Csv::CommaSep %
           QString(SimVar::GeneralEngineStarter2) % Csv::CommaSep %
           QString(SimVar::GeneralEngineStarter3) % Csv::CommaSep %
           QString(SimVar::GeneralEngineStarter4) % Csv::CommaSep %
           QString(SimVar::GeneralEngineCombustion1) % Csv::CommaSep %
           QString(SimVar::GeneralEngineCombustion2) % Csv::CommaSep %
           QString(SimVar::GeneralEngineCombustion3) % Csv::CommaSep %
           QString(SimVar::GeneralEngineCombustion4);
}

inline QString SkyDollyCsvWriter::getEngineData(const EngineData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.throttleLeverPosition1) % Csv::CommaSep %
              QString::number(data.throttleLeverPosition2) % Csv::CommaSep %
              QString::number(data.throttleLeverPosition3) % Csv::CommaSep %
              QString::number(data.throttleLeverPosition4) % Csv::CommaSep %
              QString::number(data.propellerLeverPosition1) % Csv::CommaSep %
              QString::number(data.propellerLeverPosition2) % Csv::CommaSep %
              QString::number(data.propellerLeverPosition3) % Csv::CommaSep %
              QString::number(data.propellerLeverPosition4) % Csv::CommaSep %
              QString::number(data.mixtureLeverPosition1) % Csv::CommaSep %
              QString::number(data.mixtureLeverPosition2) % Csv::CommaSep %
              QString::number(data.mixtureLeverPosition3) % Csv::CommaSep %
              QString::number(data.mixtureLeverPosition4) % Csv::CommaSep %
              QString::number(data.cowlFlapPosition1) % Csv::CommaSep %
              QString::number(data.cowlFlapPosition2) % Csv::CommaSep %
              QString::number(data.cowlFlapPosition3) % Csv::CommaSep %
              QString::number(data.cowlFlapPosition4) % Csv::CommaSep %
              QString::number(data.electricalMasterBattery1 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.electricalMasterBattery2 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.electricalMasterBattery3 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.electricalMasterBattery4 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineStarter1 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineStarter2 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineStarter3 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineStarter4 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineCombustion1 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineCombustion2 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineCombustion3 ? 1 : 0) % Csv::CommaSep %
              QString::number(data.generalEngineCombustion4 ? 1 : 0);
    } else {
        static const QString EmptyString;
        csv = EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::AileronLeftDeflection) % Csv::CommaSep %
           QString(SimVar::AileronRightDeflection) % Csv::CommaSep %
           QString(SimVar::ElevatorDeflection) % Csv::CommaSep %
           QString(SimVar::RudderDeflection) % Csv::CommaSep %
           QString(SimVar::RudderPosition) % Csv::CommaSep %
           QString(SimVar::ElevatorPosition) % Csv::CommaSep %
           QString(SimVar::AileronPosition);
}

inline QString SkyDollyCsvWriter::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leftAileronDeflection) % Csv::CommaSep %
              QString::number(data.rightAileronDeflection) % Csv::CommaSep %
              QString::number(data.elevatorDeflection) % Csv::CommaSep %
              QString::number(data.rudderDeflection) % Csv::CommaSep %
              QString::number(data.rudderPosition) % Csv::CommaSep %
              QString::number(data.elevatorPosition) % Csv::CommaSep %
              QString::number(data.aileronPosition);
    } else {
        static const QString EmptyString;
        csv = EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % Csv::CommaSep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % Csv::CommaSep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % Csv::CommaSep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % Csv::CommaSep %
           QString(SimVar::FlapsHandleIndex) % Csv::CommaSep %
           QString(SimVar::SpoilersLeftPosition) % Csv::CommaSep %
           QString(SimVar::SpoilersRightPosition) % Csv::CommaSep %
           QString(SimVar::SpoilersHandlePosition) % Csv::CommaSep %           
           QString(SimVar::SpoilersArmed);
}

inline QString SkyDollyCsvWriter::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leftLeadingEdgeFlapsPosition) % Csv::CommaSep %
              QString::number(data.rightLeadingEdgeFlapsPosition) % Csv::CommaSep %
              QString::number(data.leftTrailingEdgeFlapsPosition) % Csv::CommaSep %
              QString::number(data.rightTrailingEdgeFlapsPosition) % Csv::CommaSep %
              QString::number(data.flapsHandleIndex) % Csv::CommaSep %
              QString::number(data.leftSpoilersPosition) % Csv::CommaSep %
              QString::number(data.rightSpoilersPosition) % Csv::CommaSep %
              QString::number(data.spoilersHandlePercent) % Csv::CommaSep %              
              QString::number(data.spoilersArmed ? 1 : 0);
    } else {
        static const QString EmptyString;
        csv = EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % Csv::CommaSep %
           QString(SimVar::BrakeLeftPosition) % Csv::CommaSep %
           QString(SimVar::BrakeRightPosition) % Csv::CommaSep %
           QString(SimVar::WaterRudderHandlePosition) % Csv::CommaSep %
           QString(SimVar::TailhookPosition) % Csv::CommaSep %
           QString(SimVar::TailhookHandle) % Csv::CommaSep %
           QString(SimVar::FoldingWingLeftPercent)  % Csv::CommaSep %
           QString(SimVar::FoldingWingRightPercent)  % Csv::CommaSep %
           QString(SimVar::FoldingWingHandlePosition)  % Csv::CommaSep %
           QString(SimVar::CanopyOpen) % Csv::CommaSep %
           QString(SimVar::SmokeEnable);
}

inline QString SkyDollyCsvWriter::getAircraftHandleData(const AircraftHandleData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.gearHandlePosition ? 1 : 0) % Csv::CommaSep %
              QString::number(data.brakeLeftPosition) % Csv::CommaSep %
              QString::number(data.brakeRightPosition) % Csv::CommaSep %
              QString::number(data.waterRudderHandlePosition) % Csv::CommaSep %
              QString::number(data.tailhookPosition) % Csv::CommaSep %
              QString::number(data.tailhookHandlePosition ? 1 : 0) % Csv::CommaSep %
              QString::number(data.leftWingFolding) % Csv::CommaSep %
              QString::number(data.rightWingFolding) % Csv::CommaSep %
              QString::number(data.foldingWingHandlePosition ? 1 : 0) % Csv::CommaSep %
              QString::number(data.canopyOpen) % Csv::CommaSep %
              QString::number(data.smokeEnabled ? 1 : 0);

    } else {
        static const QString EmptyString;
        csv = EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString % Csv::CommaSep %
              EmptyString;
    }
    return csv;
}

inline QString SkyDollyCsvWriter::getLightHeader() noexcept
{
    return {SimVar::LightStates};
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
    const QString csv = type % Csv::CommaSep %
                        getPositionData(positionData) % Csv::CommaSep %
                        getEngineData(engineData) % Csv::CommaSep %
                        getPrimaryFlightControlData(primaryFlightControlData) % Csv::CommaSep %
                        getSecondaryFlightControlData(secondaryFlightControlData) % Csv::CommaSep %
                        getAircraftHandleData(aircraftHandleData) % Csv::CommaSep %
                        getLightData(lightData) % Csv::CommaSep %
                        QString::number(timestamp) % Csv::Ln;
    return io.write(csv.toUtf8());
}
