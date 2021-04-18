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
#include "../CSVConst.h"
#include "../Const.h"
#include "../SimVar.h"
#include "../Aircraft.h"
#include "../AircraftData.h"
#include "../Engine.h"
#include "../EngineData.h"
#include "../PrimaryFlightControl.h"
#include "../PrimaryFlightControlData.h"
#include "../SecondaryFlightControl.h"
#include "../SecondaryFlightControlData.h"
#include "../AircraftHandle.h"
#include "../AircraftHandleData.h"
#include "../Light.h"
#include "../LightData.h"
#include "../CSVConst.h"
#include "CSVExport.h"

// PUBLIC

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io) noexcept
{
    bool ok = io.open(QIODevice::WriteOnly);
    if (ok) {
        io.setTextModeEnabled(true);

        QString csv = QString(CSVConst::TypeColumnName) % Const::Sep %
                      getAircraftHeader() % Const::Sep %
                      getEngineHeader() % Const::Sep %
                      getPrimaryFlightControlHeader()  % Const::Sep %
                      getSecondaryFlightControlHeader() % Const::Sep %
                      getAircraftHandleHeader() % Const::Sep %
                      getLighteHeader() % Const::Sep %
                      QString(SimVar::Timestamp) % Const::Ln;

        ok = io.write(csv.toUtf8());
        if (ok) {
            const AircraftData aircraftData;
            const EngineData engineData;
            const PrimaryFlightControlData primaryFlightControlData;
            const SecondaryFlightControlData secondaryFlightControlData;
            const AircraftHandleData aircraftHandleData;
            const LightData lightData;

            // Aircraft data
            for (const AircraftData &data : aircraft.getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::Aircraft)) % Const::Sep %
                      getAircraftData(data) % Const::Sep %
                      getEngineData(engineData) % Const::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % Const::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % Const::Sep %
                      getAircraftHandleData(aircraftHandleData) % Const::Sep %
                      getLightData(lightData) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Engine data
            for (const EngineData &data : aircraft.getEngineConst().getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::Engine)) % Const::Sep %
                      getAircraftData(aircraftData) % Const::Sep %
                      getEngineData(data) % Const::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % Const::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % Const::Sep %
                      getAircraftHandleData(aircraftHandleData) % Const::Sep %
                      getLightData(lightData) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Primary flight controls
            for (const PrimaryFlightControlData &data : aircraft.getPrimaryFlightControlConst().getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::PrimaryFlightControl)) % Const::Sep %
                      getAircraftData(aircraftData) % Const::Sep %
                      getEngineData(engineData) % Const::Sep %
                      getPrimaryFlightControlData(data) % Const::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % Const::Sep %
                      getAircraftHandleData(aircraftHandleData) % Const::Sep %
                      getLightData(lightData) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Secondary flight controls
            for (const SecondaryFlightControlData &data : aircraft.getSecondaryFlightControlConst().getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::SecondaryFlightControl)) % Const::Sep %
                      getAircraftData(aircraftData) % Const::Sep %
                      getEngineData(engineData) % Const::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % Const::Sep %
                      getSecondaryFlightControlData(data) % Const::Sep %
                      getAircraftHandleData(aircraftHandleData) % Const::Sep %
                      getLightData(lightData) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Aircraft handles
            for (const AircraftHandleData &data : aircraft.getAircraftHandleConst().getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::AircraftHandle)) % Const::Sep %
                      getAircraftData(aircraftData) % Const::Sep %
                      getEngineData(engineData) % Const::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % Const::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % Const::Sep %
                      getAircraftHandleData(data) % Const::Sep %
                      getLightData(lightData) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Lights
            for (const LightData &data : aircraft.getLightConst().getAll()) {
                csv = QString::number(Enum::toUnderlyingType(CSVConst::DataType::Light)) % Const::Sep %
                      getAircraftData(aircraftData) % Const::Sep %
                      getEngineData(engineData) % Const::Sep %
                      getPrimaryFlightControlData(primaryFlightControlData) % Const::Sep %
                      getSecondaryFlightControlData(secondaryFlightControlData) % Const::Sep %
                      getAircraftHandleData(aircraftHandleData) % Const::Sep %
                      getLightData(data) % Const::Sep %
                      QString::number(data.timestamp) % Const::Ln;
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
    return QString(SimVar::Latitude) % Const::Sep %
           QString(SimVar::Longitude) % Const::Sep %
           QString(SimVar::Altitude) % Const::Sep %
           QString(SimVar::Pitch) % Const::Sep %
           QString(SimVar::Bank) % Const::Sep %
           QString(SimVar::Heading) % Const::Sep %
           QString(SimVar::VelocityBodyX) % Const::Sep %
           QString(SimVar::VelocityBodyY) % Const::Sep %
           QString(SimVar::VelocityBodyZ) % Const::Sep %
           QString(SimVar::RotationVelocityBodyX) % Const::Sep %
           QString(SimVar::RotationVelocityBodyY) % Const::Sep %
           QString(SimVar::RotationVelocityBodyZ);
}

inline QString CSVExport::getAircraftData(const AircraftData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.latitude, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.longitude, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.altitude, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.pitch, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.bank, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.heading, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.velocityBodyX, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.velocityBodyY, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.velocityBodyZ, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.rotationVelocityBodyX, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.rotationVelocityBodyY, CSVConst::Format, CSVConst::Precision) % Const::Sep %
              QString::number(data.rotationVelocityBodyZ, CSVConst::Format, CSVConst::Precision);
    } else {
        const QString EmptyString;
        csv = EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getEngineHeader() noexcept
{
    return QString(SimVar::ThrottleLeverPosition1) % Const::Sep %
           QString(SimVar::ThrottleLeverPosition2) % Const::Sep %
           QString(SimVar::ThrottleLeverPosition3) % Const::Sep %
           QString(SimVar::ThrottleLeverPosition4) % Const::Sep %
           QString(SimVar::PropellerLeverPosition1) % Const::Sep %
           QString(SimVar::PropellerLeverPosition2) % Const::Sep %
           QString(SimVar::PropellerLeverPosition3) % Const::Sep %
           QString(SimVar::PropellerLeverPosition4) % Const::Sep %
           QString(SimVar::MixtureLeverPosition1) % Const::Sep %
           QString(SimVar::MixtureLeverPosition2) % Const::Sep %
           QString(SimVar::MixtureLeverPosition3) % Const::Sep %
           QString(SimVar::MixtureLeverPosition4) % Const::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition1) % Const::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition2) % Const::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition3) % Const::Sep %
           QString(SimVar::RecipEngineCowlFlapPosition4) % Const::Sep %
           QString(SimVar::ElectricalMasterBattery1) % Const::Sep %
           QString(SimVar::ElectricalMasterBattery2) % Const::Sep %
           QString(SimVar::ElectricalMasterBattery3) % Const::Sep %
           QString(SimVar::ElectricalMasterBattery4) % Const::Sep %
           QString(SimVar::GeneralEngineStarter1) % Const::Sep %
           QString(SimVar::GeneralEngineStarter2) % Const::Sep %
           QString(SimVar::GeneralEngineStarter3) % Const::Sep %
           QString(SimVar::GeneralEngineStarter4);
}

inline QString CSVExport::getEngineData(const EngineData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.throttleLeverPosition1) % Const::Sep %
              QString::number(data.throttleLeverPosition2) % Const::Sep %
              QString::number(data.throttleLeverPosition3) % Const::Sep %
              QString::number(data.throttleLeverPosition4) % Const::Sep %
              QString::number(data.propellerLeverPosition1) % Const::Sep %
              QString::number(data.propellerLeverPosition2) % Const::Sep %
              QString::number(data.propellerLeverPosition3) % Const::Sep %
              QString::number(data.propellerLeverPosition4) % Const::Sep %
              QString::number(data.mixtureLeverPosition1) % Const::Sep %
              QString::number(data.mixtureLeverPosition2) % Const::Sep %
              QString::number(data.mixtureLeverPosition3) % Const::Sep %
              QString::number(data.mixtureLeverPosition4) % Const::Sep %
              QString::number(data.cowlFlapPosition1) % Const::Sep %
              QString::number(data.cowlFlapPosition2) % Const::Sep %
              QString::number(data.cowlFlapPosition3) % Const::Sep %
              QString::number(data.cowlFlapPosition4) % Const::Sep %
              QString::number(data.electricalMasterBattery1) % Const::Sep %
              QString::number(data.electricalMasterBattery2) % Const::Sep %
              QString::number(data.electricalMasterBattery3) % Const::Sep %
              QString::number(data.electricalMasterBattery4) % Const::Sep %
              QString::number(data.generalEngineStarter1) % Const::Sep %
              QString::number(data.generalEngineStarter2) % Const::Sep %
              QString::number(data.generalEngineStarter3) % Const::Sep %
              QString::number(data.generalEngineStarter4);
    } else {
        const QString EmptyString;
        csv = EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getPrimaryFlightControlHeader() noexcept
{
    return QString(SimVar::YokeXPosition) % Const::Sep %
           QString(SimVar::YokeYPosition) % Const::Sep %
           QString(SimVar::RudderPosition) % Const::Sep %
           QString(SimVar::ElevatorPosition) % Const::Sep %
           QString(SimVar::AileronPosition);
}

inline QString CSVExport::getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.yokeXPosition) % Const::Sep %
              QString::number(data.yokeYPosition) % Const::Sep %
              QString::number(data.rudderPosition) % Const::Sep %
              QString::number(data.elevatorPosition) % Const::Sep %
              QString::number(data.aileronPosition);
    } else {
        const QString EmptyString;
        csv = EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getSecondaryFlightControlHeader() noexcept
{
    return QString(SimVar::LeadingEdgeFlapsLeftPercent) % Const::Sep %
           QString(SimVar::LeadingEdgeFlapsRightPercent) % Const::Sep %
           QString(SimVar::TrailingEdgeFlapsLeftPercent) % Const::Sep %
           QString(SimVar::TrailingEdgeFlapsRightPercent) % Const::Sep %
           QString(SimVar::SpoilersHandlePosition) % Const::Sep %
           QString(SimVar::FlapsHandleIndex);
}

inline QString CSVExport::getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.leadingEdgeFlapsLeftPercent) % Const::Sep %
              QString::number(data.leadingEdgeFlapsRightPercent) % Const::Sep %
              QString::number(data.trailingEdgeFlapsLeftPercent) % Const::Sep %
              QString::number(data.trailingEdgeFlapsRightPercent) % Const::Sep %
              QString::number(data.spoilersHandlePosition) % Const::Sep %
              QString::number(data.flapsHandleIndex);
    } else {
        const QString EmptyString;
        csv = EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString;
    }
    return csv;
}

inline QString CSVExport::getAircraftHandleHeader() noexcept
{
    return QString(SimVar::GearHandlePosition) % Const::Sep %
           QString(SimVar::BrakeLeftPosition) % Const::Sep %
           QString(SimVar::BrakeRightPosition) % Const::Sep %
           QString(SimVar::WaterRudderHandlePosition) % Const::Sep %
           QString(SimVar::TailhookPosition) % Const::Sep %
           QString(SimVar::CanopyOpen) % Const::Sep %
           QString(SimVar::FoldingWingHandlePosition);
}

inline QString CSVExport::getAircraftHandleData(const AircraftHandleData &data) noexcept
{
    QString csv;
    if (!data.isNull()) {
        csv = QString::number(data.gearHandlePosition) % Const::Sep %
              QString::number(data.brakeLeftPosition) % Const::Sep %
              QString::number(data.brakeRightPosition) % Const::Sep %
              QString::number(data.waterRudderHandlePosition) % Const::Sep %
              QString::number(data.tailhookPosition) % Const::Sep %
              QString::number(data.canopyOpen) % Const::Sep %
              QString::number(data.foldingWingHandlePosition);
    } else {
        const QString EmptyString;
        csv = EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
              EmptyString % Const::Sep %
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
