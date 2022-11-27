/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <memory>
#include <array>
#include <unordered_map>
#include <cstdint>

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCodec>

#include <Kernel/Convert.h>
#include <Kernel/CsvParser.h>
#include <Kernel/Enum.h>
#include <Kernel/SkyMath.h>
#include <Model/SimType.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include <PluginManager/Csv.h>
#include "FlightRecorderCsvParser.h"

namespace
{
    constexpr const char *FlightRecorderCsvHeader {"Milliseconds,Latitude,Longitude,Altitude"};

    // Column names (also add them to FlightRecorderCsvParserPrivate::HeaderNames, for validation)

    // Position
    constexpr const char *Milliseconds {"Milliseconds"};
    constexpr const char *Latitude {"Latitude"};
    constexpr const char *Longitude {"Longitude"};
    constexpr const char *Altitude {"Altitude"};
    constexpr const char *Pitch {"Pitch"};
    constexpr const char *Bank {"Bank"};
    constexpr const char *TrueHeading {"TrueHeading"};
    constexpr const char *VelocityBodyX {"VelocityBodyX"};
    constexpr const char *VelocityBodyY {"VelocityBodyY"};
    constexpr const char *VelocityBodyZ {"VelocityBodyZ"};
    constexpr const char *RotationVelocityBodyX {"RotationVelocityBodyX"};
    constexpr const char *RotationVelocityBodyY {"RotationVelocityBodyY"};
    constexpr const char *RotationVelocityBodyZ {"RotationVelocityBodyZ"};

    // Engine
    constexpr const char *ThrottleLeverPosition1 {"ThrottleLeverPosition1"};
    constexpr const char *ThrottleLeverPosition2 {"ThrottleLeverPosition2"};
    constexpr const char *ThrottleLeverPosition3 {"ThrottleLeverPosition3"};
    constexpr const char *ThrottleLeverPosition4 {"ThrottleLeverPosition4"};
    constexpr const char *PropellerLeverPosition1 {"PropellerLeverPosition1"};
    constexpr const char *PropellerLeverPosition2 {"PropellerLeverPosition2"};
    constexpr const char *PropellerLeverPosition3 {"PropellerLeverPosition3"};
    constexpr const char *PropellerLeverPosition4 {"PropellerLeverPosition4"};

    // Primary flight controls
    constexpr const char *RudderPosition {"RudderPosition"};
    constexpr const char *ElevatorPosition {"ElevatorPosition"};
    constexpr const char *AileronPosition {"AileronPosition"};

    // Secondary flight controls
    constexpr const char *LeadingEdgeFlapsLeftPercent {"LeadingEdgeFlapsLeftPercent"};
    constexpr const char *LeadingEdgeFlapsRightPercent {"LeadingEdgeFlapsRightPercent"};
    constexpr const char *TrailingEdgeFlapsLeftPercent {"TrailingEdgeFlapsLeftPercent"};
    constexpr const char *TrailingEdgeFlapsRightPercent {"TrailingEdgeFlapsRightPercent"};
    constexpr const char *SpoilerHandlePosition {"SpoilerHandlePosition"};
    constexpr const char *FlapsHandleIndex {"FlapsHandleIndex"};

    // Aircraft handles
    constexpr const char *BrakeLeftPosition {"BrakeLeftPosition"};
    constexpr const char *BrakeRightPosition {"BrakeRightPosition"};
    constexpr const char *WaterRudderHandlePosition {"WaterRudderHandlePosition"};
    constexpr const char *GearHandlePosition {"GearHandlePosition"};

    // Lights
    constexpr const char *LightTaxi {"LightTaxi"};
    constexpr const char *LightLanding {"LightLanding"};
    constexpr const char *LightStrobe {"LightStrobe"};
    constexpr const char *LightBeacon {"LightBeacon"};
    constexpr const char *LightNav {"LightNav"};
    constexpr const char *LightWing {"LightWing"};
    constexpr const char *LightLogo {"LightLogo"};
    constexpr const char *LightRecognition {"LightRecognition"};
    constexpr const char *LightCabin {"LightCabin"};
}

struct FlightRecorderCsvParserPrivate
{
    FlightRecorderCsvParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    Flight *flight {nullptr};
    QDateTime firstDateTimeUtc;
    std::int64_t timestampDelta {0};
    CsvParser::Headers headers;

    static constexpr std::array<const char *, 43> HeaderNames {
                ::Milliseconds,
                ::Latitude,
                ::Longitude,
                ::Altitude,
                ::Pitch,
                ::Bank,
                ::TrueHeading,
                ::VelocityBodyX,
                ::VelocityBodyY,
                ::VelocityBodyZ,
                ::RotationVelocityBodyX,
                ::RotationVelocityBodyY,
                ::RotationVelocityBodyZ,
                ::ThrottleLeverPosition1,
                ::ThrottleLeverPosition2,
                ::ThrottleLeverPosition3,
                ::ThrottleLeverPosition4,
                ::PropellerLeverPosition1,
                ::PropellerLeverPosition2,
                ::PropellerLeverPosition3,
                ::PropellerLeverPosition4,
                ::RudderPosition,
                ::ElevatorPosition,
                ::AileronPosition,
                ::LeadingEdgeFlapsLeftPercent,
                ::LeadingEdgeFlapsRightPercent,
                ::TrailingEdgeFlapsLeftPercent,
                ::TrailingEdgeFlapsRightPercent,
                ::SpoilerHandlePosition,
                ::FlapsHandleIndex,
                ::BrakeLeftPosition,
                ::BrakeRightPosition,
                ::WaterRudderHandlePosition,
                ::GearHandlePosition,
                ::LightTaxi,
                ::LightLanding,
                ::LightStrobe,
                ::LightBeacon,
                ::LightNav,
                ::LightWing,
                ::LightLogo,
                ::LightRecognition,
                ::LightCabin
    };
    static constexpr int InvalidIdx {-1};
};

// PUBLIC

FlightRecorderCsvParser::FlightRecorderCsvParser() noexcept
    : d(std::make_unique<FlightRecorderCsvParserPrivate>())
{}

FlightRecorderCsvParser::~FlightRecorderCsvParser() = default;

bool FlightRecorderCsvParser::parse(QIODevice &io, QDateTime &firstDateTimeUtc, [[maybe_unused]] QString &flightNumber, Flight &flight) noexcept
{
    d->flight = &flight;
    QFile *file = qobject_cast<QFile *>(&io);
    firstDateTimeUtc = (file != nullptr) ? QFileInfo(*file).birthTime().toUTC() : QDateTime::currentDateTimeUtc();
    flightNumber = QString();

    CsvParser csvParser;
    QTextStream textStream(&io);
    textStream.setCodec(QTextCodec::codecForName("UTF-8"));
    CsvParser::Rows rows = csvParser.parse(textStream, ::FlightRecorderCsvHeader);
    d->headers = csvParser.getHeaders();
    bool ok = validateHeaders();
    if (ok) {
        ok = Csv::validate(rows, d->headers.size());
    }
    if (ok) {
        Aircraft &aircraft = flight.getUserAircraft();

        aircraft.getPosition().reserve(rows.size());
        aircraft.getEngine().reserve(rows.size());
        aircraft.getPrimaryFlightControl().reserve(rows.size());
        aircraft.getSecondaryFlightControl().reserve(rows.size());
        aircraft.getAircraftHandle().reserve(rows.size());
        aircraft.getLight().reserve(rows.size());

#ifdef DEBUG
        qDebug() << "parse::parse, total CSV rows:" << rows.size() << "\n"
                 << "Position size:" << aircraft.getPosition().capacity() << "\n"
                 << "Engine size:" << aircraft.getEngine().capacity() << "\n"
                 << "Primary flight controls size:" << aircraft.getPrimaryFlightControl().capacity() << "\n"
                 << "Secondary flight controls size:" << aircraft.getSecondaryFlightControl().capacity() << "\n"
                 << "Aircraft handles size:" << aircraft.getAircraftHandle().capacity() << "\n"
                 << "Light size:" << aircraft.getLight().capacity() << "\n";
#endif

        bool firstRow {true};
        for (const auto &row : rows) {

            if (firstRow) {
                // The first position timestamp must be 0, so shift all timestamps by
                // the timestamp delta, derived from the first timestamp
                // (that is usually 0 already)
                d->timestampDelta = row.at(d->headers.at(::Milliseconds)).toLongLong(&ok);
                firstRow = false;
            }
            if (ok) {
                ok = parseRow(row);
            } else {
                break;
            }
        }
    }

    // We are done with the export
    d->flight = nullptr;
    return ok;
}

// PRIVATE

 bool FlightRecorderCsvParser::validateHeaders() noexcept
 {
     bool ok {true};

     for (auto val : d->HeaderNames) {
         auto it = d->headers.find(val);
         ok = it != d->headers.end();
         if (!ok) {
             break;
         }
     }

     return ok;
 }

bool FlightRecorderCsvParser::parseRow(const CsvParser::Row &row) noexcept
{
    Aircraft &aircraft = d->flight->getUserAircraft();
    Position &position = aircraft.getPosition();
    Engine &engine = aircraft.getEngine();
    PrimaryFlightControl &primaryFlightControl = aircraft.getPrimaryFlightControl();
    SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
    Light &light = aircraft.getLight();

    // Position
    PositionData positionData;
    bool ok {true};
    const std::int64_t timestamp = row.at(d->headers.at(::Milliseconds)).toLongLong(&ok) - d->timestampDelta;
    if (ok) {
        positionData.timestamp = timestamp;
        positionData.latitude = row.at(d->headers.at(::Latitude)).toDouble(&ok);
    }
    if (ok) {
        positionData.longitude = row.at(d->headers.at(::Longitude)).toDouble(&ok);
    }
    if (ok) {
        positionData.altitude = row.at(d->headers.at(::Altitude)).toDouble(&ok);
        positionData.indicatedAltitude = positionData.altitude;
    }
    if (ok) {
        positionData.pitch = row.at(d->headers.at(::Pitch)).toDouble(&ok);
    }
    if (ok) {
        positionData.bank = row.at(d->headers.at(::Bank)).toDouble(&ok);
    }
    if (ok) {
        positionData.trueHeading = row.at(d->headers.at(::TrueHeading)).toDouble(&ok);
    }
    if (ok) {
        positionData.velocityBodyX = row.at(d->headers.at(::VelocityBodyX)).toDouble(&ok);
    }
    if (ok) {
        positionData.velocityBodyY = row.at(d->headers.at(::VelocityBodyY)).toDouble(&ok);
    }
    if (ok) {
        positionData.velocityBodyZ = row.at(d->headers.at(::VelocityBodyZ)).toDouble(&ok);
    }
    if (ok) {
        positionData.rotationVelocityBodyX = row.at(d->headers.at(::RotationVelocityBodyX)).toDouble(&ok);
    }
    if (ok) {
        positionData.rotationVelocityBodyY = row.at(d->headers.at(::RotationVelocityBodyY)).toDouble(&ok);
    }
    if (ok) {
        positionData.rotationVelocityBodyZ = row.at(d->headers.at(::RotationVelocityBodyZ)).toDouble(&ok);
    }

    if (ok) {
        position.upsertLast(positionData);
    }

    // Engine
    EngineData engineData;
    engineData.timestamp = timestamp;
    double throttleLeverPosition1 {0.0};
    double throttleLeverPosition2 {0.0};
    double throttleLeverPosition3 {0.0};
    double throttleLeverPosition4 {0.0};
    if (ok) {
        throttleLeverPosition1 = row.at(d->headers.at(::ThrottleLeverPosition1)).toDouble(&ok);
    }
    if (ok) {
        throttleLeverPosition2 = row.at(d->headers.at(::ThrottleLeverPosition2)).toDouble(&ok);
    }
    if (ok) {
        throttleLeverPosition3 = row.at(d->headers.at(::ThrottleLeverPosition3)).toDouble(&ok);
    }
    if (ok) {
        throttleLeverPosition4 = row.at(d->headers.at(::ThrottleLeverPosition4)).toDouble(&ok);
    }
    if (ok) {
        engineData.throttleLeverPosition1 = SkyMath::fromPosition(throttleLeverPosition1);
        engineData.throttleLeverPosition2 = SkyMath::fromPosition(throttleLeverPosition2);
        engineData.throttleLeverPosition3 = SkyMath::fromPosition(throttleLeverPosition3);
        engineData.throttleLeverPosition4 = SkyMath::fromPosition(throttleLeverPosition4);
        // Flight Recorder does not support all Sky Dolly simulation variables, so we initialise them to "engine on"
        initEngineDefaultValues(engineData);
        engine.upsertLast(engineData);
    }

    double propellerLeverPosition1 {0.0};
    double propellerLeverPosition2 {0.0};
    double propellerLeverPosition3 {0.0};
    double propellerLeverPosition4 {0.0};
    if (ok) {
        propellerLeverPosition1 = row.at(d->headers.at(::PropellerLeverPosition1)).toDouble(&ok);
    }
    if (ok) {
        propellerLeverPosition2 = row.at(d->headers.at(::PropellerLeverPosition2)).toDouble(&ok);
    }
    if (ok) {
        propellerLeverPosition3 = row.at(d->headers.at(::PropellerLeverPosition3)).toDouble(&ok);
    }
    if (ok) {
        propellerLeverPosition4 = row.at(d->headers.at(::PropellerLeverPosition4)).toDouble(&ok);
    }
    if (ok) {
        engineData.propellerLeverPosition1 = SkyMath::fromPosition(propellerLeverPosition1);
        engineData.propellerLeverPosition2 = SkyMath::fromPosition(propellerLeverPosition2);
        engineData.propellerLeverPosition3 = SkyMath::fromPosition(propellerLeverPosition3);
        engineData.propellerLeverPosition4 = SkyMath::fromPosition(propellerLeverPosition4);
        // Flight Recorder does not support all Sky Dolly simulation variables, so we initialise them to "engine on"
        initEngineDefaultValues(engineData);
        engine.upsertLast(engineData);
    }

    // Primary flight controls
    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.timestamp = timestamp;

    double rudderPosition {0.0};
    double elevatorPosition {0.0};
    double aileronPosition {0.0};
    if (ok) {
        rudderPosition = row.at(d->headers.at(::RudderPosition)).toDouble(&ok);
    }
    if (ok) {
        elevatorPosition = row.at(d->headers.at(::ElevatorPosition)).toDouble(&ok);
    }
    if (ok) {
        aileronPosition = row.at(d->headers.at(::AileronPosition)).toDouble(&ok);
    }
    if (ok) {
        primaryFlightControlData.rudderPosition = SkyMath::fromPosition(rudderPosition);
        primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(elevatorPosition);
        primaryFlightControlData.aileronPosition = SkyMath::fromPosition(aileronPosition);
        primaryFlightControl.upsertLast(primaryFlightControlData);
    }

    // Secondary flight controls
    SecondaryFlightControlData secondaryFlightControlData;
    secondaryFlightControlData.timestamp = timestamp;

    double leadingEdgeFlapsLeftPosition {0.0};
    double leadingEdgeFlapsRightPosition {0.0};
    double trailingEdgeFlapsLeftPosition {0.0};
    double trailingEdgeFlapsRightPosition {0.0};
    double spoilerHandlePositionPercent {0.0};
    if (ok) {
        leadingEdgeFlapsLeftPosition = row.at(d->headers.at(::LeadingEdgeFlapsLeftPercent)).toDouble(&ok);
    }
    if (ok) {
        leadingEdgeFlapsRightPosition = row.at(d->headers.at(::LeadingEdgeFlapsRightPercent)).toDouble(&ok);
    }
    if (ok) {
        trailingEdgeFlapsLeftPosition = row.at(d->headers.at(::TrailingEdgeFlapsLeftPercent)).toDouble(&ok);
    }
    if (ok) {
        trailingEdgeFlapsRightPosition = row.at(d->headers.at(::TrailingEdgeFlapsRightPercent)).toDouble(&ok);
    }
    if (ok) {
        spoilerHandlePositionPercent = row.at(d->headers.at(::SpoilerHandlePosition)).toDouble(&ok);
    }
    if (ok) {
        secondaryFlightControlData.flapsHandleIndex = row.at(d->headers.at(::FlapsHandleIndex)).toInt(&ok);
    }
    if (ok) {
        secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromPosition(leadingEdgeFlapsLeftPosition);
        secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromPosition(leadingEdgeFlapsRightPosition);
        secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromPosition(trailingEdgeFlapsLeftPosition);
        secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromPosition(trailingEdgeFlapsRightPosition);
        secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(spoilerHandlePositionPercent);
        secondaryFlightControl.upsertLast(secondaryFlightControlData);
    }

    // Aircraft handle
    AircraftHandleData aircraftHandleData;
    aircraftHandleData.timestamp = timestamp;

    double brakeLeftPosition {0.0};
    double brakeRightPosition {0.0};
    double waterRudderHandlePosition {0.0};
    if (ok) {
        brakeLeftPosition = row.at(d->headers.at(::BrakeLeftPosition)).toDouble(&ok);
    }
    if (ok) {
        brakeRightPosition = row.at(d->headers.at(::BrakeRightPosition)).toDouble(&ok);
    }
    if (ok) {
        waterRudderHandlePosition = row.at(d->headers.at(::WaterRudderHandlePosition)).toDouble(&ok);
    }
    if (ok) {
        aircraftHandleData.gearHandlePosition = row.at(d->headers.at(::GearHandlePosition)).toInt(&ok) == 1;
    }
    if (ok) {
        aircraftHandleData.brakeLeftPosition = SkyMath::fromPosition(brakeLeftPosition);
        aircraftHandleData.brakeRightPosition = SkyMath::fromPosition(brakeRightPosition);
        aircraftHandleData.waterRudderHandlePosition = SkyMath::fromPosition(waterRudderHandlePosition);
        // Flight Recorder does not support all Sky Dolly simulation variables, so we initialise them to
        // some reasonable values
        initAircraftHandleDefaultValues(aircraftHandleData);
        aircraftHandle.upsertLast(aircraftHandleData);
    }

    // Lights
    LightData lightData;
    lightData.timestamp = timestamp;

    bool lightTaxi {false};
    bool lightLanding {false};
    bool lightStrobe {false};
    bool lightBeacon {false};
    bool lightNav {false};
    bool lightWing {false};
    bool lightLogo {false};
    bool lightRecognition {false};
    bool lightCabin {false};
    if (ok) {
        lightTaxi = row.at(d->headers.at(::LightTaxi)).toInt(&ok) == 1;
    }
    if (ok) {
        lightLanding = row.at(d->headers.at(::LightLanding)).toInt(&ok) == 1;
    }
    if (ok) {
        lightStrobe = row.at(d->headers.at(::LightStrobe)).toInt(&ok) == 1;
    }
    if (ok) {
        lightBeacon = row.at(d->headers.at(::LightBeacon)).toInt(&ok) == 1;
    }
    if (ok) {
        lightNav = row.at(d->headers.at(::LightNav)).toInt(&ok) == 1;
    }
    if (ok) {
        lightWing = row.at(d->headers.at(::LightWing)).toInt(&ok) == 1;
    }
    if (ok) {
        lightLogo = row.at(d->headers.at(::LightLogo)).toInt(&ok) == 1;
    }
    if (ok) {
        lightRecognition = row.at(d->headers.at(::LightRecognition)).toInt(&ok) == 1;
    }
    if (ok) {
        lightCabin = row.at(d->headers.at(::LightCabin)).toInt(&ok) == 1;
    }
    if (ok) {
        lightData.lightStates.setFlag(SimType::LightState::Taxi, lightTaxi);
        lightData.lightStates.setFlag(SimType::LightState::Landing, lightLanding);
        lightData.lightStates.setFlag(SimType::LightState::Strobe, lightStrobe);
        lightData.lightStates.setFlag(SimType::LightState::Beacon, lightBeacon);
        lightData.lightStates.setFlag(SimType::LightState::Navigation, lightNav);
        lightData.lightStates.setFlag(SimType::LightState::Wing, lightWing);
        lightData.lightStates.setFlag(SimType::LightState::Logo, lightLogo);
        lightData.lightStates.setFlag(SimType::LightState::Recognition, lightRecognition);
        lightData.lightStates.setFlag(SimType::LightState::Cabin, lightCabin);
        light.upsertLast(lightData);
    }

    return ok;
}

inline void FlightRecorderCsvParser::initEngineDefaultValues(EngineData &engineData) noexcept
{
    // Turn "engine on" and "full propeller", no cowl flaps
    engineData.mixtureLeverPosition1 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition2 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition3 = SkyMath::PercentMax8;
    engineData.mixtureLeverPosition4 = SkyMath::PercentMax8;
    engineData.cowlFlapPosition1 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition2 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition3 = SkyMath::PercentMin8;
    engineData.cowlFlapPosition4 = SkyMath::PercentMin8;
    engineData.electricalMasterBattery1 = true;
    engineData.electricalMasterBattery2 = true;
    engineData.electricalMasterBattery3 = true;
    engineData.electricalMasterBattery4 = true;
    engineData.generalEngineStarter1 = true;
    engineData.generalEngineStarter2 = true;
    engineData.generalEngineStarter3 = true;
    engineData.generalEngineStarter4 = true;
    engineData.generalEngineCombustion1 = true;
    engineData.generalEngineCombustion2 = true;
    engineData.generalEngineCombustion3 = true;
    engineData.generalEngineCombustion4 = true;
}

inline void FlightRecorderCsvParser::initAircraftHandleDefaultValues(AircraftHandleData &aircraftHandleData) noexcept
{
    aircraftHandleData.tailhookPosition = 0;
    aircraftHandleData.canopyOpen = 0;
    aircraftHandleData.leftWingFolding = 0;
    aircraftHandleData.rightWingFolding = 0;
    aircraftHandleData.smokeEnabled = false;
}
