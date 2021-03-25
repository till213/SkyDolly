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
#include <QFlags>

#include "../Const.h"
#include "../SimVar.h"
#include "../AircraftData.h"
#include "../Aircraft.h"
#include "CSVImport.h"


// PUBLIC

bool CSVImport::importData(QIODevice &io, Aircraft &aircraft) noexcept
{

    bool ok = io.open(QIODevice::ReadOnly);
    if (ok) {
        // Headers
        QByteArray line = io.readLine();
        // Trim away line endings (\r\n for instance)
        QByteArray data = line.trimmed();

        ok = !data.isNull();
        if (ok) {
            QList<QByteArray> headers = data.split(Const::Sep);

            // Clear existing data
            aircraft.blockSignals(true);
            aircraft.clear();

            // CSV data
            data = io.readLine();
            int rowIndex = 0;
            double timestampDelta = 0.0;
            while (!data.isNull()) {

                AircraftData aircraftData;
                QList<QByteArray> values = data.split(Const::Sep);

                int columnIndex = 0;
                for (QByteArray &header : headers) {

                    double doubleValue;
                    int    intValue;
                    // Position
                    if (header == SimVar::Latitude) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.latitude = doubleValue;
                        }
                    } else if (header == SimVar::Longitude) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.longitude = doubleValue;
                        }
                    } else if (header == SimVar::Altitude) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.altitude = doubleValue;
                        }
                    } else if (header == SimVar::Pitch) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.pitch = doubleValue;
                        }
                    } else if (header == SimVar::Bank) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.bank = doubleValue;
                        }
                    } else if (header == SimVar::Heading) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.heading = doubleValue;
                        }
                    // Velocity
                    } else if (header == SimVar::VelocityBodyX) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.velocityBodyX = doubleValue;
                        }
                    } else if (header == SimVar::VelocityBodyY) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.velocityBodyY = doubleValue;
                        }
                    } else if (header == SimVar::VelocityBodyZ) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.velocityBodyZ = doubleValue;
                        }
                    } else if (header == SimVar::RotationVelocityBodyX) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.rotationVelocityBodyX = doubleValue;
                        }
                    } else if (header == SimVar::RotationVelocityBodyY) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.rotationVelocityBodyY= doubleValue;
                        }
                    } else if (header == SimVar::RotationVelocityBodyZ) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.rotationVelocityBodyZ = doubleValue;
                        }
                        // TODO IMPLEMENT ME!!!
                    // Aircraft controls
//                    } else if (header == SimVar::YokeXPosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.yokeXPosition = doubleValue;
//                        }
//                    } else if (header == SimVar::YokeYPosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.yokeYPosition = doubleValue;
//                        }
//                    } else if (header == SimVar::RudderPosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.rudderPosition = doubleValue;
//                        }
//                    } else if (header == SimVar::ElevatorPosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.elevatorPosition = doubleValue;
//                        }
//                    } else if (header == SimVar::AileronPosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.aileronPosition = doubleValue;
//                        }
                    // Engine
                        // TODO IMPLEMENT ME!!!
//                    } else if (header == SimVar::ThrottleLeverPosition1) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.throttleLeverPosition1 = doubleValue;
//                        }
//                    } else if (header == SimVar::ThrottleLeverPosition2) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.throttleLeverPosition2 = doubleValue;
//                        }
//                    } else if (header == SimVar::ThrottleLeverPosition3) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.throttleLeverPosition3 = doubleValue;
//                        }
//                    } else if (header == SimVar::ThrottleLeverPosition4) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.throttleLeverPosition4 = doubleValue;
//                        }
//                    } else if (header == SimVar::PropellerLeverPosition1) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.propellerLeverPosition1 = doubleValue;
//                        }
//                    } else if (header == SimVar::PropellerLeverPosition2) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.propellerLeverPosition2 = doubleValue;
//                        }
//                    } else if (header == SimVar::PropellerLeverPosition3) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.propellerLeverPosition3 = doubleValue;
//                        }
//                    } else if (header == SimVar::PropellerLeverPosition4) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.propellerLeverPosition4 = doubleValue;
//                        }
//                    } else if (header == SimVar::MixtureLeverPosition1) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.mixtureLeverPosition1 = doubleValue;
//                        }
//                    } else if (header == SimVar::MixtureLeverPosition2) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.mixtureLeverPosition2 = doubleValue;
//                        }
//                    } else if (header == SimVar::MixtureLeverPosition3) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.mixtureLeverPosition3 = doubleValue;
//                        }
//                    } else if (header == SimVar::MixtureLeverPosition4) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.mixtureLeverPosition4 = doubleValue;
//                        }
                    // Flaps & speed brakes
//                    } else if (header == SimVar::LeadingEdgeFlapsLeftPercent) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.leadingEdgeFlapsLeftPercent = doubleValue;
//                        }
//                    } else if (header == SimVar::LeadingEdgeFlapsRightPercent) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.leadingEdgeFlapsRightPercent = doubleValue;
//                        }
//                    } else if (header == SimVar::TrailingEdgeFlapsLeftPercent) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.trailingEdgeFlapsLeftPercent = doubleValue;
//                        }
//                    } else if (header == SimVar::TrailingEdgeFlapsRightPercent) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.trailingEdgeFlapsRightPercent = doubleValue;
//                        }
//                    } else if (header == SimVar::SpoilersHandlePosition) {
//                        doubleValue = values.at(columnIndex).toDouble(&ok);
//                        if (ok) {
//                            aircraftData.spoilersHandlePosition = doubleValue;
//                        }
//                    } else if (header == SimVar::FlapsHandleIndex) {
//                        intValue = values.at(columnIndex).toInt(&ok);
//                        if (ok) {
//                            aircraftData.flapsHandleIndex = intValue;
//                        }
                    // Gear, brakes & handles
                    } else if (header == SimVar::GearHandlePosition) {
                        intValue = values.at(columnIndex).toInt(&ok);
                        if (ok) {
                            aircraftData.gearHandlePosition = intValue == 1 ? true : false;
                        }
                    } else if (header == SimVar::BrakeLeftPosition) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.brakeLeftPosition = doubleValue;
                        }
                    } else if (header == SimVar::BrakeRightPosition) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.brakeRightPosition = doubleValue;
                        }
                    } else if (header == SimVar::WaterRudderHandlePosition) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.waterRudderHandlePosition = doubleValue;
                        }
                    } else if (header == SimVar::TailhookPosition) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.tailhookPosition = doubleValue;
                        }
                    } else if (header == SimVar::CanopyOpen) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            aircraftData.canopyOpen = doubleValue;
                        }
                    // Lights
                    } else if (header == SimVar::LightStates) {
                        intValue = values.at(columnIndex).toInt(&ok);
                        if (ok) {
                            aircraftData.lightStates = SimType::LightStates(intValue);
                        }
                    // Timestamp
                    } else if (header == SimVar::Timestamp) {
                        doubleValue = values.at(columnIndex).toDouble(&ok);
                        if (ok) {
                            if (rowIndex != 0) {
                                aircraftData.timestamp = doubleValue + timestampDelta;
                            } else {
                                // The first timestamp must be 0, so shift all timestamps by
                                // the timestamp delta, derived from the first timestamp
                                // (which is usually 0 already)
                                timestampDelta = -doubleValue;
                                aircraftData.timestamp = 0.0;
                            }
                        }
                    }

                    if (ok) {
                        // Next value
                        ++columnIndex;
                    } else {
                        // Parse error
                        break;
                    }

                }

                aircraft.upsertAircraftData(std::move(aircraftData));

                // Read next line
                data = io.readLine();
                ++rowIndex;

            }

            aircraft.blockSignals(false);
            emit aircraft.dataChanged();
        }
        io.close();
    }
    return ok;
}
