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

#include "../SimVar.h"
#include "../Aircraft.h"
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
        QString csv = QString(SimVar::Latitude) % SimVar::Sep %
                      QString(SimVar::Longitude) % SimVar::Sep %
                      QString(SimVar::Altitude) % SimVar::Sep %
                      QString(SimVar::Pitch) % SimVar::Sep %
                      QString(SimVar::Bank) % SimVar::Sep %
                      QString(SimVar::Heading) % SimVar::Sep %
                      QString(SimVar::VelocityBodyX) % SimVar::Sep %
                      QString(SimVar::VelocityBodyY) % SimVar::Sep %
                      QString(SimVar::VelocityBodyZ) % SimVar::Sep %
                      QString(SimVar::RotationVelocityBodyX) % SimVar::Sep %
                      QString(SimVar::RotationVelocityBodyY) % SimVar::Sep %
                      QString(SimVar::RotationVelocityBodyZ) % SimVar::Sep %
                      QString(SimVar::YokeXPosition) % SimVar::Sep %
                      QString(SimVar::YokeYPosition) % SimVar::Sep %
                      QString(SimVar::RudderPosition) % SimVar::Sep %
                      QString(SimVar::ElevatorPosition) % SimVar::Sep %
                      QString(SimVar::AileronPosition) % SimVar::Sep %
                      QString(SimVar::ThrottleLeverPosition1) % SimVar::Sep %
                      QString(SimVar::ThrottleLeverPosition2) % SimVar::Sep %
                      QString(SimVar::ThrottleLeverPosition3) % SimVar::Sep %
                      QString(SimVar::ThrottleLeverPosition4) % SimVar::Sep %
                      QString(SimVar::PropellerLeverPosition1) % SimVar::Sep %
                      QString(SimVar::PropellerLeverPosition2) % SimVar::Sep %
                      QString(SimVar::PropellerLeverPosition3) % SimVar::Sep %
                      QString(SimVar::PropellerLeverPosition4) % SimVar::Sep %
                      QString(SimVar::MixtureLeverPosition1) % SimVar::Sep %
                      QString(SimVar::MixtureLeverPosition2) % SimVar::Sep %
                      QString(SimVar::MixtureLeverPosition3) % SimVar::Sep %
                      QString(SimVar::MixtureLeverPosition4) % SimVar::Sep %
                      QString(SimVar::LeadingEdgeFlapsLeftPercent) % SimVar::Sep %
                      QString(SimVar::LeadingEdgeFlapsRightPercent) % SimVar::Sep %
                      QString(SimVar::TrailingEdgeFlapsLeftPercent) % SimVar::Sep %
                      QString(SimVar::TrailingEdgeFlapsRightPercent) % SimVar::Sep %
                      QString(SimVar::SpoilersHandlePosition) % SimVar::Sep %
                      QString(SimVar::FlapsHandleIndex) % SimVar::Sep %
                      QString(SimVar::GearHandlePosition) % SimVar::Sep %
                      QString(SimVar::BrakeLeftPosition) % SimVar::Sep %
                      QString(SimVar::BrakeRightPosition) % SimVar::Sep %
                      QString(SimVar::WaterRudderHandlePosition) % SimVar::Sep %
                      QString(SimVar::TailhookPosition) % SimVar::Sep %
                      QString(SimVar::CanopyOpen) % SimVar::Sep %
                      QString(SimVar::LightStates) % SimVar::Sep %
                      QString(SimVar::Timestamp) % SimVar::Ln;
        if (!io.write(csv.toUtf8())) {
            ok = false;
        }

        if (ok) {
            // CSV data
            for (const AircraftData &data : aircraft.getAllAircraftData()) {
                QString csv = QString::number(data.latitude, Format, Precision) % SimVar::Sep %
                              QString::number(data.longitude, Format, Precision) % SimVar::Sep %
                              QString::number(data.altitude, Format, Precision) % SimVar::Sep %
                              QString::number(data.pitch, Format, Precision) % SimVar::Sep %
                              QString::number(data.bank, Format, Precision) % SimVar::Sep %
                              QString::number(data.heading, Format, Precision) % SimVar::Sep %
                              QString::number(data.velocityBodyX, Format, Precision) % SimVar::Sep %
                              QString::number(data.velocityBodyY, Format, Precision) % SimVar::Sep %
                              QString::number(data.velocityBodyZ, Format, Precision) % SimVar::Sep %
                              QString::number(data.rotationVelocityBodyX, Format, Precision) % SimVar::Sep %
                              QString::number(data.rotationVelocityBodyY, Format, Precision) % SimVar::Sep %
                              QString::number(data.rotationVelocityBodyZ, Format, Precision) % SimVar::Sep %
                              QString::number(data.yokeXPosition) % SimVar::Sep %
                              QString::number(data.yokeYPosition) % SimVar::Sep %
                              QString::number(data.rudderPosition) % SimVar::Sep %
                              QString::number(data.elevatorPosition) % SimVar::Sep %
                              QString::number(data.aileronPosition) % SimVar::Sep %
                        // TODO IMPLEMENT ME!!!
//                              QString::number(data.throttleLeverPosition1) % SimVar::Sep %
//                              QString::number(data.throttleLeverPosition2) % SimVar::Sep %
//                              QString::number(data.throttleLeverPosition3) % SimVar::Sep %
//                              QString::number(data.throttleLeverPosition4) % SimVar::Sep %
//                              QString::number(data.propellerLeverPosition1) % SimVar::Sep %
//                              QString::number(data.propellerLeverPosition2) % SimVar::Sep %
//                              QString::number(data.propellerLeverPosition3) % SimVar::Sep %
//                              QString::number(data.propellerLeverPosition4) % SimVar::Sep %
//                              QString::number(data.mixtureLeverPosition1) % SimVar::Sep %
//                              QString::number(data.mixtureLeverPosition2) % SimVar::Sep %
//                              QString::number(data.mixtureLeverPosition3) % SimVar::Sep %
//                              QString::number(data.mixtureLeverPosition4) % SimVar::Sep %
                              QString::number(data.leadingEdgeFlapsLeftPercent) % SimVar::Sep %
                              QString::number(data.leadingEdgeFlapsRightPercent) % SimVar::Sep %
                              QString::number(data.trailingEdgeFlapsLeftPercent) % SimVar::Sep %
                              QString::number(data.trailingEdgeFlapsRightPercent) % SimVar::Sep %
                              QString::number(data.spoilersHandlePosition) % SimVar::Sep %
                              QString::number(data.flapsHandleIndex) % SimVar::Sep %
                              QString::number(data.gearHandlePosition) % SimVar::Sep %
                              QString::number(data.brakeLeftPosition) % SimVar::Sep %
                              QString::number(data.brakeRightPosition) % SimVar::Sep %
                              QString::number(data.waterRudderHandlePosition) % SimVar::Sep %
                              QString::number(data.tailhookPosition) % SimVar::Sep %
                              QString::number(data.canopyOpen) % SimVar::Sep %
                              QString::number(data.lightStates) % SimVar::Sep %
                              QString::number(data.timestamp) % SimVar::Ln;
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
