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
#include <QFileDialog>
#include <QMessageBox>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../../Persistence/src/CSVConst.h"
#include "../../../../../Persistence/src/Service/FlightService.h"
#include "CSVImportPlugin.h"

// PUBLIC

CSVImportPlugin::CSVImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("CSVImportPlugin::CSVImportPlugin: PLUGIN LOADED");
#endif
}

CSVImportPlugin::~CSVImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("CSVImportPlugin::~CSVImportPlugin: PLUGIN UNLOADED");
#endif
}

bool CSVImportPlugin::importData(FlightService &flightService) const noexcept
{
    bool ok;

    // Start with the last export path
    QString exportPath = Settings::getInstance().getExportPath();

    // TODO Pass main window as parent
    const QString filePath = QFileDialog::getOpenFileName(nullptr, tr("Import CSV"), exportPath, QString("*.csv"));
    if (!filePath.isEmpty()) {

        QFile file(filePath);
        ok = file.open(QIODevice::ReadOnly);
        if (ok) {

            Unit unit;
            Flight &flight = Logbook::getInstance().getCurrentFlight();
            Aircraft &aircraft = flight.getUserAircraft();
            flight.clear(true);

            // Headers
            QByteArray line = file.readLine();
            // Trim away line endings (\r\n for instance)
            QByteArray data = line.trimmed();

            ok = !data.isNull();
            if (ok) {
                QList<QByteArray> headers = data.split(CSVConst::Sep);
                if (headers.first() == QString(CSVConst::TypeColumnName)) {
                    headers.removeFirst();

                    // Clear existing data
                    aircraft.blockSignals(true);
                    aircraft.clear();

                    // CSV data
                    data = file.readLine();
                    bool firstPositionData = true;
                    bool firstEngineData = true;
                    bool firstPrimaryFlightControlData = true;
                    bool firstSecondaryFlightControlData = true;
                    bool firstAircraftHandleData = true;
                    bool firstLightData = true;
                    while (!data.isNull()) {

                        PositionData positionData;
                        QList<QByteArray> values = data.split(CSVConst::Sep);

                        // Type
                        ok = values.at(0).size() > 0;
                        if (ok) {
                            CSVConst::DataType dataType = static_cast<CSVConst::DataType>(values.at(0).at(0));
                            values.removeFirst();
                            switch (dataType) {
                            case CSVConst::DataType::Aircraft:
                                ok = importPositionData(headers, values, firstPositionData, aircraft);
                                firstPositionData = false;
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
                            data = file.readLine();
                        } else {
                            break;
                        }
                    }
                    aircraft.blockSignals(false);
                    emit aircraft.dataChanged();

                    exportPath = QFileInfo(filePath).absolutePath();
                    Settings::getInstance().setExportPath(exportPath);

                    AircraftInfo info(aircraft.getId());
                    info.startDate = QFileInfo(filePath).birthTime();
                    info.endDate = info.startDate.addMSecs(aircraft.getDurationMSec());
                    aircraft.setAircraftInfo(info);
                    flight.setTitle(tr("CSV import"));
                    flight.setDescription(tr("Aircraft imported on %1 from file: %2").arg(unit.formatDateTime(QDateTime::currentDateTime()), filePath));
                    flightService.store(flight);

                } else {
                    ok = false;
                }
            }
            file.close();
        }
    } else {
        ok = true;
    }
    return ok;
}

inline bool CSVImportPlugin::importPositionData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept
{
    PositionData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

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
        aircraft.getPosition().upsert(data);
    }
    return ok;
}

inline bool CSVImportPlugin::importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept
{
    EngineData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::ThrottleLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition1 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition2 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition3 = intValue;
            }
        } else if (header == SimVar::ThrottleLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.throttleLeverPosition4 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition1 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition2 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition3 = intValue;
            }
        } else if (header == SimVar::PropellerLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.propellerLeverPosition4 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition1 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition2 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition3 = intValue;
            }
        } else if (header == SimVar::MixtureLeverPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.mixtureLeverPosition4 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition1 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition2 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition3 = intValue;
            }
        } else if (header == SimVar::RecipEngineCowlFlapPosition4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.cowlFlapPosition4 = intValue;
            }
        } else if (header == SimVar::ElectricalMasterBattery1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery1 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery2 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery3 = intValue != 0;
            }
        } else if (header == SimVar::ElectricalMasterBattery4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.electricalMasterBattery4 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter1) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter1 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter2) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter2 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter3) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter3 = intValue != 0;
            }
        } else if (header == SimVar::GeneralEngineStarter4) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.generalEngineStarter4 = intValue != 0;
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

inline bool CSVImportPlugin::importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept
{
    PrimaryFlightControlData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::RudderPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.rudderPosition = intValue;
            }
        } else if (header == SimVar::ElevatorPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.elevatorPosition = intValue;
            }
        } else if (header == SimVar::AileronPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.aileronPosition = intValue;
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

inline bool CSVImportPlugin::importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept
{
    SecondaryFlightControlData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::LeadingEdgeFlapsLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leadingEdgeFlapsLeftPercent = intValue;
            }
        } else if (header == SimVar::LeadingEdgeFlapsRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leadingEdgeFlapsRightPercent = intValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.trailingEdgeFlapsLeftPercent = intValue;
            }
        } else if (header == SimVar::TrailingEdgeFlapsRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.trailingEdgeFlapsRightPercent = intValue;
            }
        } else if (header == SimVar::SpoilersHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.spoilersHandlePosition = intValue;
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

inline bool CSVImportPlugin::importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept
{
    AircraftHandleData data;
    int columnIndex = 0;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::GearHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.gearHandlePosition = intValue == 1 ? true : false;
            }
        } else if (header == SimVar::BrakeLeftPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.brakeLeftPosition = intValue;
            }
        } else if (header == SimVar::BrakeRightPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.brakeRightPosition = intValue;
            }
        } else if (header == SimVar::WaterRudderHandlePosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.waterRudderHandlePosition = intValue;
            }
        } else if (header == SimVar::TailhookPosition) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.tailhookPosition = intValue;
            }
        } else if (header == SimVar::CanopyOpen) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.canopyOpen = intValue;
            }
        } else if (header == SimVar::FoldingWingLeftPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.leftWingFolding = intValue;
            }
        } else if (header == SimVar::FoldingWingRightPercent) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.rightWingFolding = intValue;
            }
            // Timestamp
        } else if (header == SimVar::Timestamp) {
            ok = importTimestamp(values, columnIndex, firstRow, data.timestamp, timestampDelta);
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

inline bool CSVImportPlugin::importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept
{
    LightData data;
    int columnIndex = 0;
    qint64 timestamp;
    qint64 timestampDelta = 0;
    bool ok = false;
    for (const QByteArray &header : headers) {

        if (columnIndex >= values.count()) {
            // Less values than headers
            ok = false;
            break;
        }

        int intValue;
        if (header == SimVar::LightStates) {
            intValue = values.at(columnIndex).toInt(&ok);
            if (ok) {
                data.lightStates = static_cast<SimType::LightStates>(intValue);
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

inline bool CSVImportPlugin::importTimestamp(const QList<QByteArray> &values, int columnIndex, bool firstRow, qint64 &timestamp, qint64 &timestampDelta)
{
    bool ok;
    timestamp = values.at(columnIndex).toLongLong(&ok);
    if (ok) {
        if (!firstRow) {
            timestamp += timestampDelta;
        } else {
            // The first timestamp must be 0, so shift all timestamps by
            // the timestamp delta, derived from the first timestamp
            // (which is usually 0 already)
            timestampDelta = -timestamp;
            timestamp = 0.0;
        }
    }
    return ok;
}
