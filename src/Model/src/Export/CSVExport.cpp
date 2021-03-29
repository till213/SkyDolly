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
#include "CSVExport.h"

// PUBLIC

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io) noexcept
{
    bool ok = io.open(QIODevice::WriteOnly);
    if (ok) {
        io.setTextModeEnabled(true);

        QString csv = QString("Type") % Const::Sep;
        appendAircraftHeader(csv);
        csv.append(Const::Sep);
        appendEngineHeader(csv);
        csv.append(Const::Sep);
        appendPrimaryFlightControlHeader(csv);
        csv.append(Const::Sep);
        appendSecondaryFlightControlHeader(csv);
        csv.append(Const::Sep);
        appendAircraftHandleHeader(csv);
        csv.append(Const::Sep);
        appendLighteHeader(csv);
        csv.append(Const::Sep % QString(SimVar::Timestamp) % Const::Ln);

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
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::Aircraft)) % Const::Sep);
                appendAircraftData(data, csv);
                csv.append(Const::Sep);
                appendEngineData(engineData, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(primaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(secondaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(aircraftHandleData, csv);
                csv.append(Const::Sep);
                appendLightData(lightData, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Engine data
            for (const EngineData &data : aircraft.getEngineConst().getAll()) {
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::Engine)) % Const::Sep);
                appendAircraftData(aircraftData, csv);
                csv.append(Const::Sep);
                appendEngineData(data, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(primaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(secondaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(aircraftHandleData, csv);
                csv.append(Const::Sep);
                appendLightData(lightData, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Primary flight controls
            for (const PrimaryFlightControlData &data : aircraft.getPrimaryFlightControlConst().getAll()) {
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::PrimaryFlightControl)) % Const::Sep);
                appendAircraftData(aircraftData, csv);
                csv.append(Const::Sep);
                appendEngineData(engineData, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(data, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(secondaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(aircraftHandleData, csv);
                csv.append(Const::Sep);
                appendLightData(lightData, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Secondary flight controls
            for (const SecondaryFlightControlData &data : aircraft.getSecondaryFlightControlConst().getAll()) {
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::SecondaryFlightControl)) % Const::Sep);
                appendAircraftData(aircraftData, csv);
                csv.append(Const::Sep);
                appendEngineData(engineData, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(primaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(data, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(aircraftHandleData, csv);
                csv.append(Const::Sep);
                appendLightData(lightData, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Aircraft handles
            for (const AircraftHandleData &data : aircraft.getAircraftHandleConst().getAll()) {
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::AircraftHandle)) % Const::Sep);
                appendAircraftData(aircraftData, csv);
                csv.append(Const::Sep);
                appendEngineData(engineData, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(primaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(secondaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(data, csv);
                csv.append(Const::Sep);
                appendLightData(lightData, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
                if (!io.write(csv.toUtf8())) {
                    ok = false;
                    break;
                }
            }

            // Lights
            for (const LightData &data : aircraft.getLightConst().getAll()) {
                csv.clear();
                csv.append(QString::number(Enum::toUnderlyingType(CSVConst::DataType::Light)) % Const::Sep);
                appendAircraftData(aircraftData, csv);
                csv.append(Const::Sep);
                appendEngineData(engineData, csv);
                csv.append(Const::Sep);
                appendPrimaryFlightControlData(primaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendSecondaryFlightControlData(secondaryFlightControlData, csv);
                csv.append(Const::Sep);
                appendAircraftHandleData(aircraftHandleData, csv);
                csv.append(Const::Sep);
                appendLightData(data, csv);
                csv.append(Const::Sep % QString::number(data.timestamp) % Const::Ln);
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

void CSVExport::appendAircraftHeader(QString &header) noexcept
{
    header.append(QString(SimVar::Latitude) % Const::Sep %
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
                  QString(SimVar::RotationVelocityBodyZ));
}

void CSVExport::appendAircraftData(const AircraftData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.latitude, CSVConst::Format, CSVConst::Precision) % Const::Sep %
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
                   QString::number(data.rotationVelocityBodyZ, CSVConst::Format, CSVConst::Precision));
    } else {
        const QString EmptyString;
        csv.append(EmptyString % Const::Sep %
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
                   EmptyString);
    }
}

void CSVExport::appendEngineHeader(QString &header) noexcept
{
    header.append(QString(SimVar::ThrottleLeverPosition1) % Const::Sep %
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
                  QString(SimVar::MixtureLeverPosition4));
}

void CSVExport::appendEngineData(const EngineData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.throttleLeverPosition1) % Const::Sep %
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
                   QString::number(data.mixtureLeverPosition4));
    } else {
        const QString EmptyString;
        csv.append(EmptyString % Const::Sep %
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
                   EmptyString);
    }
}

void CSVExport::appendPrimaryFlightControlHeader(QString &header) noexcept
{
    header.append(QString(SimVar::YokeXPosition) % Const::Sep %
                  QString(SimVar::YokeYPosition) % Const::Sep %
                  QString(SimVar::RudderPosition) % Const::Sep %
                  QString(SimVar::ElevatorPosition) % Const::Sep %
                  QString(SimVar::AileronPosition));
}

void CSVExport::appendPrimaryFlightControlData(const PrimaryFlightControlData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.yokeXPosition) % Const::Sep %
                   QString::number(data.yokeYPosition) % Const::Sep %
                   QString::number(data.rudderPosition) % Const::Sep %
                   QString::number(data.elevatorPosition) % Const::Sep %
                   QString::number(data.aileronPosition));
    } else {
        const QString EmptyString;
        csv.append(EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString);
    }
}

void CSVExport::appendSecondaryFlightControlHeader(QString &header) noexcept
{
    header.append(QString(SimVar::LeadingEdgeFlapsLeftPercent) % Const::Sep %
                  QString(SimVar::LeadingEdgeFlapsRightPercent) % Const::Sep %
                  QString(SimVar::TrailingEdgeFlapsLeftPercent) % Const::Sep %
                  QString(SimVar::TrailingEdgeFlapsRightPercent) % Const::Sep %
                  QString(SimVar::SpoilersHandlePosition) % Const::Sep %
                  QString(SimVar::FlapsHandleIndex));
}

void CSVExport::appendSecondaryFlightControlData(const SecondaryFlightControlData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.leadingEdgeFlapsLeftPercent) % Const::Sep %
                   QString::number(data.leadingEdgeFlapsRightPercent) % Const::Sep %
                   QString::number(data.trailingEdgeFlapsLeftPercent) % Const::Sep %
                   QString::number(data.trailingEdgeFlapsRightPercent) % Const::Sep %
                   QString::number(data.spoilersHandlePosition) % Const::Sep %
                   QString::number(data.flapsHandleIndex));
    } else {
        const QString EmptyString;
        csv.append(EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString);
    }
}

void CSVExport::appendAircraftHandleHeader(QString &header) noexcept
{
    header.append(QString(SimVar::GearHandlePosition) % Const::Sep %
                  QString(SimVar::BrakeLeftPosition) % Const::Sep %
                  QString(SimVar::BrakeRightPosition) % Const::Sep %
                  QString(SimVar::WaterRudderHandlePosition) % Const::Sep %
                  QString(SimVar::TailhookPosition) % Const::Sep %
                  QString(SimVar::CanopyOpen));
}

void CSVExport::appendAircraftHandleData(const AircraftHandleData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.gearHandlePosition) % Const::Sep %
                   QString::number(data.brakeLeftPosition) % Const::Sep %
                   QString::number(data.brakeRightPosition) % Const::Sep %
                   QString::number(data.waterRudderHandlePosition) % Const::Sep %
                   QString::number(data.tailhookPosition) % Const::Sep %
                   QString::number(data.canopyOpen));
    } else {
        const QString EmptyString;
        csv.append(EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString % Const::Sep %
                   EmptyString);
    }
}

void CSVExport::appendLighteHeader(QString &header) noexcept
{
    header.append(QString(SimVar::LightStates));
}

void CSVExport::appendLightData(const LightData &data, QString &csv) noexcept
{
    if (!data.isNull()) {
        csv.append(QString::number(data.lightStates));
    } else {
        csv.append(QString());
    }
}
