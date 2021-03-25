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

#include "../Const.h"
#include "../SimVar.h"
#include "../Aircraft.h"
#include "../AircraftData.h"
#include "CSVExport.h"

namespace {
    // Format and precision for double
    constexpr char Format = 'g';
    constexpr int Precision = 9;
}

// PUBLIC

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io) noexcept
{
    bool ok = io.open(QIODevice::WriteOnly);
    if (ok) {
        io.setTextModeEnabled(true);
        // Header
        QString csv = QString(SimVar::Latitude) % Const::Sep %
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
                      QString(SimVar::RotationVelocityBodyZ) % Const::Sep %
                      QString(SimVar::YokeXPosition) % Const::Sep %
                      QString(SimVar::YokeYPosition) % Const::Sep %
                      QString(SimVar::RudderPosition) % Const::Sep %
                      QString(SimVar::ElevatorPosition) % Const::Sep %
                      QString(SimVar::AileronPosition) % Const::Sep %
                      QString(SimVar::ThrottleLeverPosition1) % Const::Sep %
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
                      QString(SimVar::LeadingEdgeFlapsLeftPercent) % Const::Sep %
                      QString(SimVar::LeadingEdgeFlapsRightPercent) % Const::Sep %
                      QString(SimVar::TrailingEdgeFlapsLeftPercent) % Const::Sep %
                      QString(SimVar::TrailingEdgeFlapsRightPercent) % Const::Sep %
                      QString(SimVar::SpoilersHandlePosition) % Const::Sep %
                      QString(SimVar::FlapsHandleIndex) % Const::Sep %
                      QString(SimVar::GearHandlePosition) % Const::Sep %
                      QString(SimVar::BrakeLeftPosition) % Const::Sep %
                      QString(SimVar::BrakeRightPosition) % Const::Sep %
                      QString(SimVar::WaterRudderHandlePosition) % Const::Sep %
                      QString(SimVar::TailhookPosition) % Const::Sep %
                      QString(SimVar::CanopyOpen) % Const::Sep %
                      QString(SimVar::LightStates) % Const::Sep %
                      QString(SimVar::Timestamp) % Const::Ln;
        if (!io.write(csv.toUtf8())) {
            ok = false;
        }

        if (ok) {
            // CSV data
            for (const AircraftData &data : aircraft.getAllAircraftData()) {
                QString csv = QString::number(data.latitude, Format, Precision) % Const::Sep %
                              QString::number(data.longitude, Format, Precision) % Const::Sep %
                              QString::number(data.altitude, Format, Precision) % Const::Sep %
                              QString::number(data.pitch, Format, Precision) % Const::Sep %
                              QString::number(data.bank, Format, Precision) % Const::Sep %
                              QString::number(data.heading, Format, Precision) % Const::Sep %
                              QString::number(data.velocityBodyX, Format, Precision) % Const::Sep %
                              QString::number(data.velocityBodyY, Format, Precision) % Const::Sep %
                              QString::number(data.velocityBodyZ, Format, Precision) % Const::Sep %
                              QString::number(data.rotationVelocityBodyX, Format, Precision) % Const::Sep %
                              QString::number(data.rotationVelocityBodyY, Format, Precision) % Const::Sep %
                              QString::number(data.rotationVelocityBodyZ, Format, Precision) % Const::Sep %
                        // TODO IMPLEMENT ME!!!
//                              QString::number(data.yokeXPosition) % Const::Sep %
//                              QString::number(data.yokeYPosition) % Const::Sep %
//                              QString::number(data.rudderPosition) % Const::Sep %
//                              QString::number(data.elevatorPosition) % Const::Sep %
//                              QString::number(data.aileronPosition) % Const::Sep %
                        // TODO IMPLEMENT ME!!!
//                              QString::number(data.throttleLeverPosition1) % Const::Sep %
//                              QString::number(data.throttleLeverPosition2) % Const::Sep %
//                              QString::number(data.throttleLeverPosition3) % Const::Sep %
//                              QString::number(data.throttleLeverPosition4) % Const::Sep %
//                              QString::number(data.propellerLeverPosition1) % Const::Sep %
//                              QString::number(data.propellerLeverPosition2) % Const::Sep %
//                              QString::number(data.propellerLeverPosition3) % Const::Sep %
//                              QString::number(data.propellerLeverPosition4) % Const::Sep %
//                              QString::number(data.mixtureLeverPosition1) % Const::Sep %
//                              QString::number(data.mixtureLeverPosition2) % Const::Sep %
//                              QString::number(data.mixtureLeverPosition3) % Const::Sep %
//                              QString::number(data.mixtureLeverPosition4) % Const::Sep %
                              QString::number(data.leadingEdgeFlapsLeftPercent) % Const::Sep %
                              QString::number(data.leadingEdgeFlapsRightPercent) % Const::Sep %
                              QString::number(data.trailingEdgeFlapsLeftPercent) % Const::Sep %
                              QString::number(data.trailingEdgeFlapsRightPercent) % Const::Sep %
                              QString::number(data.spoilersHandlePosition) % Const::Sep %
                              QString::number(data.flapsHandleIndex) % Const::Sep %
                              QString::number(data.gearHandlePosition) % Const::Sep %
                              QString::number(data.brakeLeftPosition) % Const::Sep %
                              QString::number(data.brakeRightPosition) % Const::Sep %
                              QString::number(data.waterRudderHandlePosition) % Const::Sep %
                              QString::number(data.tailhookPosition) % Const::Sep %
                              QString::number(data.canopyOpen) % Const::Sep %
                              QString::number(data.lightStates) % Const::Sep %
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
