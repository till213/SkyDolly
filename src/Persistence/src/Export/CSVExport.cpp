/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QIODevice>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>

#include "../../../Kernel/src/Enum.h"
#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Position.h"
#include "../../../Model/src/PositionData.h"
#include "../../../Model/src/Engine.h"
#include "../../../Model/src/EngineData.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/PrimaryFlightControlData.h"
#include "../../../Model/src/SecondaryFlightControl.h"
#include "../../../Model/src/SecondaryFlightControlData.h"
#include "../../../Model/src/AircraftHandle.h"
#include "../../../Model/src/AircraftHandleData.h"
#include "../../../Model/src/Light.h"
#include "../../../Model/src/LightData.h"
#include "../CSVConst.h"
#include "CSVExport.h"

// PUBLIC

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io) noexcept
{
    bool ok = io.open(QIODevice::WriteOnly);
    if (ok) {
        io.setTextModeEnabled(true);

        QString csv = QString(CSVConst::TypeColumnName) % CSVConst::Sep %
                      getAircraftHeader() % CSVConst::Sep %
                      getEngineHeader() % CSVConst::Sep %
                      getPrimaryFlightControlHeader()  % CSVConst::Sep %
                      getSecondaryFlightControlHeader() % CSVConst::Sep %
                      getAircraftHandleHeader() % CSVConst::Sep %
                      getLighteHeader() % CSVConst::Sep %
                      QString(SimVar::Timestamp) % CSVConst::Ln;

        ok = io.write(csv.toUtf8());
        if (ok) {
            const PositionData positionData;
            const EngineData engineData;
            const PrimaryFlightControlData primaryFlightControlData;
            const SecondaryFlightControlData secondaryFlightControlData;
            const AircraftHandleData aircraftHandleData;
            const LightData lightData;

            // Position data
            for (const PositionData &data : aircraft.getPositionConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::Aircraft)) % CSVConst::Sep %
                      getPositionData(data) % CSVConst::Sep %
                      getEngineData(engineData) % CSVConst::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                      getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                      getLightData(lightData) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Engine data
            for (const EngineData &data : aircraft.getEngineConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::Engine)) % CSVConst::Sep %
                      getPositionData(positionData) % CSVConst::Sep %
                      getEngineData(data) % CSVConst::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                      getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                      getLightData(lightData) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Primary flight controls
            for (const PrimaryFlightControlData &data : aircraft.getPrimaryFlightControlConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::PrimaryFlightControl)) % CSVConst::Sep %
                      getPositionData(positionData) % CSVConst::Sep %
                      getEngineData(engineData) % CSVConst::Sep %
                      getPrimaryFlightControlData(data) % CSVConst::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                      getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                      getLightData(lightData) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Secondary flight controls
            for (const SecondaryFlightControlData &data : aircraft.getSecondaryFlightControlConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::SecondaryFlightControl)) % CSVConst::Sep %
                      getPositionData(positionData) % CSVConst::Sep %
                      getEngineData(engineData) % CSVConst::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                      getSecondaryFlightControlData(data) % CSVConst::Sep %
                      getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                      getLightData(lightData) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Aircraft handles
            for (const AircraftHandleData &data : aircraft.getAircraftHandleConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::AircraftHandle)) % CSVConst::Sep %
                      getPositionData(positionData) % CSVConst::Sep %
                      getEngineData(engineData) % CSVConst::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                      getAircraftHandleData(data) % CSVConst::Sep %
                      getLightData(lightData) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Lights
            for (const LightData &data : aircraft.getLightConst().getAllConst()) {
                csv = QChar(Enum::toUnderlyingType(CSVConst::DataType::Light)) % CSVConst::Sep %
                      getPositionData(positionData) % CSVConst::Sep %
                      getEngineData(engineData) % CSVConst::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % CSVConst::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % CSVConst::Sep %
                      getAircraftHandleData(aircraftHandleData) % CSVConst::Sep %
                      getLightData(data) % CSVConst::Sep %
                      QString::number(data.timestamp) % CSVConst::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }
        }
        io.close();
    }
    return ok;
}

// PRIVATE

inline QString CSVExport::getAircraftHeader() noexcept
{
    return QString(SimVar::Latitude) % CSVConst::Sep %
           QString(SimVar::Longitude) % CSVConst::Sep %
           QString(SimVar::Altitude) % CSVConst::Sep %
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

inline QString CSVExport::getPositionData(const PositionData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.latitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.longitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
              QString::number(data.altitude, CSVConst::Format, CSVConst::Precision) % CSVConst::Sep %
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
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getEngineHeader() noexcept
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
           QString(SimVar::GeneralEngineStarter4);
}

inline QString CSVExport::getEngineData(const EngineData &data) noexcept
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
              QString::number(data.generalEngineStarter4);
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
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::RudderPosition) % CSVConst::Sep %
           QString(SimVar::ElevatorPosition) % CSVConst::Sep %
           QString(SimVar::AileronPosition);
}

inline QString CSVExport::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
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

inline QString CSVExport::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % CSVConst::Sep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % CSVConst::Sep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % CSVConst::Sep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % CSVConst::Sep %
           QString(SimVar::SpoilersHandlePosition) % CSVConst::Sep %
           QString(SimVar::FlapsHandleIndex);
}

inline QString CSVExport::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leadingEdgeFlapsLeftPercent) % CSVConst::Sep %
              QString::number(data.leadingEdgeFlapsRightPercent) % CSVConst::Sep %
              QString::number(data.trailingEdgeFlapsLeftPercent) % CSVConst::Sep %
              QString::number(data.trailingEdgeFlapsRightPercent) % CSVConst::Sep %
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

inline QString CSVExport::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % CSVConst::Sep %
           QString(SimVar::BrakeLeftPosition) % CSVConst::Sep %
           QString(SimVar::BrakeRightPosition) % CSVConst::Sep %
           QString(SimVar::WaterRudderHandlePosition) % CSVConst::Sep %
           QString(SimVar::TailhookPosition) % CSVConst::Sep %
           QString(SimVar::FoldingWingLeftPercent)  % CSVConst::Sep %
           QString(SimVar::FoldingWingRightPercent)  % CSVConst::Sep %
           QString(SimVar::CanopyOpen);
}

inline QString CSVExport::getAircraftHandleData(const AircraftHandleData &data) noexcept
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
              QString::number(data.canopyOpen);

    } else {
        const QString EmptyString;
        csv = EmptyString % CSVConst::Sep %
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

inline QString CSVExport::getLighteHeader() noexcept
{
    return QString(SimVar::LightStates);
}

inline QString CSVExport::getLightData(const LightData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.lightStates);
    } else {
        csv = QString();
    }
    return csv;
}
