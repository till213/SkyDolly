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
#include "../Aircraft.h"
#include "CSVExport.h"

namespace {
    // Format and precision for double
    constexpr char Format = 'g';
    constexpr int Precision = 9;
}

// PUBLIC

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io)
{
    bool ok = io.open(QIODevice::WriteOnly);
    if (ok) {
        io.setTextModeEnabled(true);
        // Header
        QString csv = QString(Const::Latitude) % Const::Sep %
                      QString(Const::Longitude) % Const::Sep %
                      QString(Const::Altitude) % Const::Sep %
                      QString(Const::Pitch) % Const::Sep %
                      QString(Const::Bank) % Const::Sep %
                      QString(Const::Heading) % Const::Sep %
                      QString(Const::YokeXPosition) % Const::Sep %
                      QString(Const::YokeYPosition) % Const::Sep %
                      QString(Const::RudderPosition) % Const::Sep %
                      QString(Const::ElevatorPosition) % Const::Sep %
                      QString(Const::AileronPosition) % Const::Sep %
                      QString(Const::ThrottleLeverPosition1) % Const::Sep %
                      QString(Const::ThrottleLeverPosition2) % Const::Sep %
                      QString(Const::ThrottleLeverPosition3) % Const::Sep %
                      QString(Const::ThrottleLeverPosition4) % Const::Sep %
                      QString(Const::PropellerLeverPosition1) % Const::Sep %
                      QString(Const::PropellerLeverPosition2) % Const::Sep %
                      QString(Const::PropellerLeverPosition3) % Const::Sep %
                      QString(Const::PropellerLeverPosition4) % Const::Sep %
                      QString(Const::MixtureLeverPosition1) % Const::Sep %
                      QString(Const::MixtureLeverPosition2) % Const::Sep %
                      QString(Const::MixtureLeverPosition3) % Const::Sep %
                      QString(Const::MixtureLeverPosition4) % Const::Sep %
                      QString(Const::LeadingEdgeFlapsLeftPercent) % Const::Sep %
                      QString(Const::LeadingEdgeFlapsRightPercent) % Const::Sep %
                      QString(Const::TrailingEdgeFlapsLeftPercent) % Const::Sep %
                      QString(Const::TrailingEdgeFlapsRightPercent) % Const::Sep %
                      QString(Const::SpoilersHandlePosition) % Const::Sep %
                      QString(Const::FlapsHandleIndex) % Const::Sep %
                      QString(Const::GearHandlePosition) % Const::Sep %
                      QString(Const::WaterRudderHandlePosition) % Const::Sep %
                      QString(Const::BrakeLeftPosition) % Const::Sep %
                      QString(Const::BrakeRightPosition) % Const::Sep %
                      QString(Const::Timestamp) % Const::Ln;
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
                              QString::number(data.yokeXPosition) % Const::Sep %
                              QString::number(data.yokeYPosition) % Const::Sep %
                              QString::number(data.rudderPosition) % Const::Sep %
                              QString::number(data.elevatorPosition) % Const::Sep %
                              QString::number(data.aileronPosition) % Const::Sep %
                              QString::number(data.throttleLeverPosition1) % Const::Sep %
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
                              QString::number(data.leadingEdgeFlapsLeftPercent) % Const::Sep %
                              QString::number(data.leadingEdgeFlapsRightPercent) % Const::Sep %
                              QString::number(data.trailingEdgeFlapsLeftPercent) % Const::Sep %
                              QString::number(data.trailingEdgeFlapsRightPercent) % Const::Sep %
                              QString::number(data.spoilersHandlePosition) % Const::Sep %
                              QString::number(data.flapsHandleIndex) % Const::Sep %
                              QString::number(data.gearHandlePosition) % Const::Sep %
                              QString::number(data.waterRudderHandlePosition) % Const::Sep %
                              QString::number(data.brakeLeftPosition) % Const::Sep %
                              QString::number(data.brakeRightPosition) % Const::Sep %
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
