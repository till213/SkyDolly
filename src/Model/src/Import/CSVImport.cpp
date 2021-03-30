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
#include <QByteArray>
#include <QList>

#include "../CSVConst.h"
#include "../Const.h"
#include "../SimVar.h"
#include "../Aircraft.h"
#include "../AircraftData.h"
#include "../Engine.h"
#include "../EngineData.h"
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
            headers.removeFirst();

            // Clear existing data
            aircraft.blockSignals(true);
            aircraft.clear();

            // CSV data
            data = io.readLine();
            bool firstAircraftData = true;
            bool firstEngineData = true;
            bool firstPrimaryFlightControlData = true;
            bool firstSecondaryFlightControlData = true;
            bool firstAircraftHandleData = true;
            bool firstLightData = true;
            while (!data.isNull()) {

                AircraftData aircraftData;
                QList<QByteArray> values = data.split(Const::Sep);

                // Type
                CSVConst::DataType dataType = static_cast<CSVConst::DataType>(values.at(0).toInt(&ok));
                if (ok) {
                    values.removeFirst();
                    switch (dataType) {
                    case CSVConst::DataType::Aircraft:
                        ok = importAircraftData(headers, values, firstAircraftData, aircraft);
                        firstAircraftData = false;
                        break;
                    case CSVConst::DataType::Engine:
                        ok = importEngineData(headers, values, firstEngineData, aircraft.getEngine());
                        firstEngineData = false;
                        break;
                    case CSVConst::DataType::PrimaryFlightControl:
                        ok = importPrimaryFlightControlData(headers, values, firstPrimaryFlightControlData, aircraft.getPrimaryFlightControl());
                        firstPrimaryFlightControlData = false;
                        break;
                    case CSVConst::DataType::SecondaryFlightControl:
                        ok = importSecondaryFlightControlData(headers, values, firstSecondaryFlightControlData, aircraft.getSecondaryFlightControl());
                        firstSecondaryFlightControlData = false;
                        break;
                    case CSVConst::DataType::AircraftHandle:
                        ok = importAircraftHandleData(headers, values, firstAircraftHandleData, aircraft.getAircraftHandle());
                        firstAircraftHandleData = false;
                        break;
                    case CSVConst::DataType::Light:
                        ok = importLightData(headers, values, firstLightData, aircraft.getLight());
                        firstLightData = false;
                        break;
                    default:
                        // Ignore unknown data types
                        break;
                    }
                }

                // Read next line
                if (ok) {
                    data = io.readLine();
                } else {
                    break;
                }
            }
            aircraft.blockSignals(false);
            emit aircraft.dataChanged();
        }
        io.close();
    }
    return ok;
}

inline bool CSVImport::importAircraftData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept
{
    AircraftData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        // Position
        double doubleValue;
        if (header == SimVar::Latitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.latitude = doubleValue;
            }
        } else if (header == SimVar::Longitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.longitude = doubleValue;
            }
        } else if (header == SimVar::Altitude) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.altitude = doubleValue;
            }
        } else if (header == SimVar::Pitch) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.pitch = doubleValue;
            }
        } else if (header == SimVar::Bank) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.bank = doubleValue;
            }
        } else if (header == SimVar::Heading) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.heading = doubleValue;
            }
        // Velocity
        } else if (header == SimVar::VelocityBodyX) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyX = doubleValue;
            }
        } else if (header == SimVar::VelocityBodyY) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyY = doubleValue;
            }
        } else if (header == SimVar::VelocityBodyZ) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.velocityBodyZ = doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyX) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyX = doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyY) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyY= doubleValue;
            }
        } else if (header == SimVar::RotationVelocityBodyZ) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rotationVelocityBodyZ = doubleValue;
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        aircraft.upsert(std::move(data));
    }
    return ok;
}

inline bool CSVImport::importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept
{
    EngineData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        double doubleValue;
        if (header == SimVar::ThrottleLeverPosition1) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.throttleLeverPosition1 = doubleValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition2) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.throttleLeverPosition2 = doubleValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition3) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.throttleLeverPosition3 = doubleValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition4) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.throttleLeverPosition4 = doubleValue;
            }
        } else if (header == SimVar::PropellerLeverPosition1) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.propellerLeverPosition1 = doubleValue;
            }
        } else if (header == SimVar::PropellerLeverPosition2) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.propellerLeverPosition2 = doubleValue;
            }
        } else if (header == SimVar::PropellerLeverPosition3) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.propellerLeverPosition3 = doubleValue;
            }
        } else if (header == SimVar::PropellerLeverPosition4) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.propellerLeverPosition4 = doubleValue;
            }
        } else if (header == SimVar::MixtureLeverPosition1) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.mixtureLeverPosition1 = doubleValue;
            }
        } else if (header == SimVar::MixtureLeverPosition2) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.mixtureLeverPosition2 = doubleValue;
            }
        } else if (header == SimVar::MixtureLeverPosition3) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.mixtureLeverPosition3 = doubleValue;
            }
        } else if (header == SimVar::MixtureLeverPosition4) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.mixtureLeverPosition4 = doubleValue;
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        engine.upsert(std::move(data));
    }
    return ok;
}

inline bool CSVImport::importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept
{
    PrimaryFlightControlData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        double doubleValue;
        // Position
        if (header == SimVar::YokeXPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.yokeXPosition = doubleValue;
            }
        } else if (header == SimVar::YokeYPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.yokeYPosition = doubleValue;
            }
        } else if (header == SimVar::RudderPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.rudderPosition = doubleValue;
            }
        } else if (header == SimVar::ElevatorPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.elevatorPosition = doubleValue;
            }
        } else if (header == SimVar::AileronPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.aileronPosition = doubleValue;
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        primaryFlightControl.upsert(std::move(data));
    }
    return ok;
}

inline bool CSVImport::importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept
{
    SecondaryFlightControlData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        double doubleValue;
        int intValue;
        if (header == SimVar::LeadingEdgeFlapsLeftPercent) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.leadingEdgeFlapsLeftPercent = doubleValue;
            }
        } else if (header == SimVar::LeadingEdgeFlapsRightPercent) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.leadingEdgeFlapsRightPercent = doubleValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsLeftPercent) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.trailingEdgeFlapsLeftPercent = doubleValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsRightPercent) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.trailingEdgeFlapsRightPercent = doubleValue;
            }
        } else if (header == SimVar::SpoilersHandlePosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.spoilersHandlePosition = doubleValue;
            }
        } else if (header == SimVar::FlapsHandleIndex) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.flapsHandleIndex = intValue;
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        secondaryFlightControl.upsert(std::move(data));
    }
    return ok;
}

inline bool CSVImport::importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept
{
    AircraftHandleData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        double doubleValue;
        int intValue;
        if (header == SimVar::GearHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.gearHandlePosition = intValue == 1 ? true : false;
            }
        } else if (header == SimVar::BrakeLeftPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.brakeLeftPosition = doubleValue;
            }
        } else if (header == SimVar::BrakeRightPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.brakeRightPosition = doubleValue;
            }
        } else if (header == SimVar::WaterRudderHandlePosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.waterRudderHandlePosition = doubleValue;
            }
        } else if (header == SimVar::TailhookPosition) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.tailhookPosition = doubleValue;
            }
        } else if (header == SimVar::CanopyOpen) {
            doubleValue = values.at(columnIndex).toDouble(&ok);
            if (ok) {
                data.canopyOpen = doubleValue;
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        aircraftHandle.upsert(std::move(data));
    }
    return ok;
}

inline bool CSVImport::importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept
{
    LightData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        int intValue;
        if (header == SimVar::LightStates) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.lightStates = SimType::LightStates(intValue);
            }
        // Timestamp
        } else if (header == SimVar::Timestamp) {
            timestamp = values.at(columnIndex).toLongLong(&ok);
            if (ok) {
                if (!firstRow) {
                    data.timestamp = timestamp + timestampDelta;
                } else {
                    // The first timestamp must be 0, so shift all timestamps by
                    // the timestamp delta, derived from the first timestamp
                    // (which is usually 0 already)
                    timestampDelta = -timestamp;
                    data.timestamp = 0.0;
                }
            }
        } else {
            ok = true;
        }

        if (ok) {
            // Next value
            ++columnIndex;
        } else {
            // Parse error
            break;
        }

    }
    if (ok) {
        light.upsert(std::move(data));
    }
    return ok;
}
