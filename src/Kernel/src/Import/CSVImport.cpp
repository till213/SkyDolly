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
#include "../AircraftData.h"
#include "../Aircraft.h"
#include "CSVImport.h"


// PUBLIC

bool CSVImport::importData(QIODevice &io, Aircraft &aircraft)
{
    bool ok = io.open(QIODevice::ReadOnly);
    if (ok) {
        // Headers
        QByteArray data = io.readLine();
        data.truncate(data.length() - 1);
        ok = !data.isNull();
        if (ok) {
            QList<QByteArray> headers = data.split(Const::Sep);

            // Clear existing data
            aircraft.clear();

            // CSV data
            data = io.readLine();
            while (!data.isNull()) {

                data.truncate(data.length() - 1);
                AircraftData aircraftData;
                QList<QByteArray> values = data.split(Const::Sep);

                int index = 0;
                for (QByteArray &header : headers) {

                    double doubleValue;
                    int    intValue;
                    bool   boolValue;
                    // Position
                    if (header == Const::Latitude) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.latitude = doubleValue;
                        }
                    } else if (header == Const::Longitude) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.longitude = doubleValue;
                        }
                    } else if (header == Const::Altitude) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.altitude = doubleValue;
                        }
                    } else if (header == Const::Pitch) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.pitch = doubleValue;
                        }
                    } else if (header == Const::Bank) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.bank = doubleValue;
                        }
                    } else if (header == Const::Heading) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.heading = doubleValue;
                        }
                    // Aircraft controls
                    } else if (header == Const::YokeXPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.yokeXPosition = doubleValue;
                        }
                    } else if (header == Const::YokeYPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.yokeYPosition = doubleValue;
                        }
                    } else if (header == Const::RudderPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.rudderPosition = doubleValue;
                        }
                    } else if (header == Const::ElevatorPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.elevatorPosition = doubleValue;
                        }
                    } else if (header == Const::AileronPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.aileronPosition = doubleValue;
                        }
                    // Engine
                    } else if (header == Const::ThrottleLeverPosition1) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.throttleLeverPosition1 = doubleValue;
                        }
                    } else if (header == Const::ThrottleLeverPosition2) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.throttleLeverPosition2 = doubleValue;
                        }
                    } else if (header == Const::ThrottleLeverPosition3) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.throttleLeverPosition3 = doubleValue;
                        }
                    } else if (header == Const::ThrottleLeverPosition4) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.throttleLeverPosition4 = doubleValue;
                        }
                    // Flaps & speed brakes
                    } else if (header == Const::LeadingEdgeFlapsLeftPercent) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.leadingEdgeFlapsLeftPercent = doubleValue;
                        }
                    } else if (header == Const::LeadingEdgeFlapsRightPercent) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.leadingEdgeFlapsRightPercent = doubleValue;
                        }
                    } else if (header == Const::TrailingEdgeFlapsLeftPercent) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.trailingEdgeFlapsLeftPercent = doubleValue;
                        }
                    } else if (header == Const::TrailingEdgeFlapsRightPercent) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.trailingEdgeFlapsRightPercent = doubleValue;
                        }
                    } else if (header == Const::SpoilersHandlePosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.spoilersHandlePosition = doubleValue;
                        }
                    } else if (header == Const::FlapsHandleIndex) {
                        intValue = values.at(index).toInt(&ok);
                        if (ok) {
                            aircraftData.flapsHandleIndex = intValue;
                        }
                    // Gear & brakes
                    } else if (header == Const::GearHandlePosition) {
                        intValue = values.at(index).toInt(&ok);
                        if (ok) {
                            aircraftData.gearHandlePosition = intValue == 1 ? true : false;
                        }
                    } else if (header == Const::WaterRudderHandlePosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.waterRudderHandlePosition = doubleValue;
                        }
                    } else if (header == Const::BrakeLeftPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.brakeLeftPosition = doubleValue;
                        }
                    } else if (header == Const::BrakeRightPosition) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.brakeRightPosition = doubleValue;
                        }
                    // Timestamp
                    } else if (header == Const::Timestamp) {
                        doubleValue = values.at(index).toDouble(&ok);
                        if (ok) {
                            aircraftData.timestamp = doubleValue;
                        }
                    }

                    if (ok) {
                        // Next value
                        ++index;
                    } else {
                        // Parse error
                        break;
                    }

                }

                aircraft.upsertAircraftData(std::move(aircraftData));

                // Read next line
                data = io.readLine();

            }
        }
        io.close();
    }
    return ok;
}
