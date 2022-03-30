/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <vector>
#include <unordered_set>
#include <cstdint>

#include <QStringBuilder>
#include <QIODevice>
#include <QFlags>
#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QEasingCurve>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Flight/src/Analytics.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "IGCImportOptionWidget.h"
#include "IGCImportSettings.h"
#include "IGCParser.h"
#include "IGCImportPlugin.h"

namespace
{
    // Distance threshold beyond which two waypoints are to be considered different [meters]
    // (taking the average size of a glider airfield into account)
    constexpr double SameWaypointDistanceThreshold = 500;
}

class IGCImportPluginPrivate
{
public:
    IGCImportPluginPrivate()
        : throttleResponseCurve(QEasingCurve::OutExpo)
    {}

    IGCParser igcParser;

    enum struct EngineState {
        Unknown,
        Running,
        Shutdown
    };

    IGCImportSettings settings;
    QEasingCurve throttleResponseCurve;

    static const inline QString FileExtension {QStringLiteral("igc")};
};

// PUBLIC

IGCImportPlugin::IGCImportPlugin() noexcept
    : d(std::make_unique<IGCImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::IGCImportPlugin: PLUGIN LOADED");
#endif
}

IGCImportPlugin::~IGCImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::~IGCImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

void IGCImportPlugin::addSettings(Settings::PluginSettings &settings) const noexcept
{
    d->settings.addSettings(settings);
}

void IGCImportPlugin::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    d->settings.addKeysWithDefaults(keysWithDefaults);
}

void IGCImportPlugin::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->settings.applySettings(valuesByKey);
}

QString IGCImportPlugin::getFileFilter() const noexcept
{
    return tr("International Gliding Commission (*.%1)").arg(IGCImportPluginPrivate::FileExtension);
}

std::unique_ptr<QWidget> IGCImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<IGCImportOptionWidget>(d->settings);
}

bool IGCImportPlugin::readFile(QFile &file) noexcept
{
    bool ok = d->igcParser.parse(file);
    if (ok) {
        // Now "upsert" the position data, taking possible duplicate timestamps into account
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        const Aircraft &aircraft = flight.getUserAircraft();
        Position &position = aircraft.getPosition();

        // Engine
        Engine &engine = aircraft.getEngine();
        EngineData engineData;
        IGCImportPluginPrivate::EngineState engineState = IGCImportPluginPrivate::EngineState::Unknown;
        const double enlThresholdNorm = static_cast<double>(d->settings.m_enlThresholdPercent) / 100.0;

        for (const IGCParser::Fix &fix : d->igcParser.getFixes()) {
            // Import either GNSS or pressure altitude
            const double altitude = d->settings.m_altitude == IGCImportSettings::Altitude::GnssAltitude ? fix.gnssAltitude : fix.pressureAltitude;
            PositionData positionData {fix.latitude, fix.longitude, altitude};
            positionData.timestamp = fix.timestamp;
            positionData.indicatedAltitude = fix.pressureAltitude;
            position.upsertLast(std::move(positionData));

            if (d->igcParser.hasEnvironmentalNoiseLevel()) {
                const double enl = fix.environmentalNoiseLevel;
                const double position = noiseToPosition(enl, enlThresholdNorm);
                const bool loudNoise = enl > enlThresholdNorm;
                switch (engineState) {
                case IGCImportPluginPrivate::EngineState::Unknown:
                    // Previous engine state unknown, so initially engine in any case
                    engineData.timestamp = fix.timestamp;
                    engineData.electricalMasterBattery1 = true;
                    engineData.electricalMasterBattery2 = true;
                    engineData.electricalMasterBattery3 = true;
                    engineData.electricalMasterBattery4 = true;
                    engineData.generalEngineCombustion1 = loudNoise;
                    engineData.generalEngineCombustion2 = loudNoise;
                    engineData.generalEngineCombustion3 = loudNoise;
                    engineData.generalEngineCombustion4 = loudNoise;

                    engineData.throttleLeverPosition1 = SkyMath::fromPosition(position);
                    engineData.throttleLeverPosition2 = SkyMath::fromPosition(position);
                    engineData.throttleLeverPosition3 = SkyMath::fromPosition(position);
                    engineData.throttleLeverPosition4 = SkyMath::fromPosition(position);
                    engineData.propellerLeverPosition1 = SkyMath::fromPosition(position);
                    engineData.propellerLeverPosition2 = SkyMath::fromPosition(position);
                    engineData.propellerLeverPosition3 = SkyMath::fromPosition(position);
                    engineData.propellerLeverPosition4 = SkyMath::fromPosition(position);
                    engineData.mixtureLeverPosition1 = SkyMath::fromPosition(1.0);
                    engineData.mixtureLeverPosition2 = SkyMath::fromPosition(1.0);
                    engineData.mixtureLeverPosition3 = SkyMath::fromPosition(1.0);
                    engineData.mixtureLeverPosition4 = SkyMath::fromPosition(1.0);
                    // Elements are inserted chronologically from the start (and no other engine
                    // data exist yet), so we can use upsertLast (instead of the more general upsert)
                    engine.upsertLast(engineData);
                    engineState = loudNoise ? IGCImportPluginPrivate::EngineState::Running : IGCImportPluginPrivate::EngineState::Shutdown;
#ifdef DEBUG
    qDebug("IGCImportPlugin::readFile: engine INITIALISED, current ENL: %f threshold %f, engine RUNNING: %d", enl, enlThresholdNorm, loudNoise);
#endif
                    break;
                case IGCImportPluginPrivate::EngineState::Running:
                    if (!loudNoise) {
                        engineData.timestamp = fix.timestamp;
                        engineData.generalEngineCombustion1 = false;
                        engineData.generalEngineCombustion2 = false;
                        engineData.generalEngineCombustion3 = false;
                        engineData.generalEngineCombustion4 = false;
                        engineData.throttleLeverPosition1 = SkyMath::fromPosition(0.0);
                        engineData.throttleLeverPosition2 = SkyMath::fromPosition(0.0);
                        engineData.throttleLeverPosition3 = SkyMath::fromPosition(0.0);
                        engineData.throttleLeverPosition4 = SkyMath::fromPosition(0.0);
                        engineData.propellerLeverPosition1 = SkyMath::fromPosition(0.0);
                        engineData.propellerLeverPosition2 = SkyMath::fromPosition(0.0);
                        engineData.propellerLeverPosition3 = SkyMath::fromPosition(0.0);
                        engineData.propellerLeverPosition4 = SkyMath::fromPosition(0.0);
                        engine.upsertLast(engineData);
                        engineState = IGCImportPluginPrivate::EngineState::Shutdown;
#ifdef DEBUG
    qDebug("IGCImportPlugin::readFile: engine now SHUTDOWN, current ENL: %f < %f", enl, enlThresholdNorm);
#endif
                    }
                    break;

                case IGCImportPluginPrivate::EngineState::Shutdown:
                    if (loudNoise) {
                        engineData.timestamp = fix.timestamp;
                        engineData.generalEngineCombustion1 = true;
                        engineData.generalEngineCombustion2 = true;
                        engineData.generalEngineCombustion3 = true;
                        engineData.generalEngineCombustion4 = true;
                        engineData.throttleLeverPosition1 = SkyMath::fromPosition(position);
                        engineData.throttleLeverPosition2 = SkyMath::fromPosition(position);
                        engineData.throttleLeverPosition3 = SkyMath::fromPosition(position);
                        engineData.throttleLeverPosition4 = SkyMath::fromPosition(position);
                        engineData.propellerLeverPosition1 = SkyMath::fromPosition(position);
                        engineData.propellerLeverPosition2 = SkyMath::fromPosition(position);
                        engineData.propellerLeverPosition3 = SkyMath::fromPosition(position);
                        engineData.propellerLeverPosition4 = SkyMath::fromPosition(position);
                        engine.upsertLast(engineData);
                        engineState = IGCImportPluginPrivate::EngineState::Running;
#ifdef DEBUG
    qDebug("IGCImportPlugin::readFile: engine now RUNNING, current ENL: %f > %f", enl, enlThresholdNorm);
#endif
                    }
                    break;
                default:
                    break;
                }
            }
        }

        std::vector<IGCParser::TaskItem> tasks = d->igcParser.getTask().tasks;
        if (tasks.size() > 0) {
            updateWaypoints();
        }
    }
    return ok;
}

FlightAugmentation::Procedures IGCImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects IGCImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects {FlightAugmentation::Aspect::All};
    // Do not augment the engine data: the engine data is already derived from the
    // environmental noise level (ENL - if available)
    aspects.setFlag(FlightAugmentation::Aspect::Engine, false);
    return aspects;
}

QDateTime IGCImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->igcParser.getHeader().flightDateTimeUtc;
}

QString IGCImportPlugin::getTitle() const noexcept
{
    return d->igcParser.getHeader().gliderType;
}

void IGCImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    const IGCParser::Header &header = d->igcParser.getHeader();
    aircraftInfo.tailNumber = header.gliderId;
    aircraftInfo.flightNumber = header.flightNumber;
}

void IGCImportPlugin::updateExtendedFlightInfo(Flight &flight) noexcept
{
    const IGCParser::Header &header = d->igcParser.getHeader();
    Unit unit;
    const QString description = flight.getDescription() % "\n\n" %
                                tr("Glider type:") % " " % header.gliderType % "\n" %
                                tr("Pilot:") % " " % header.pilotName % "\n" %
                                tr("Co-Pilot:") % " " % header.coPilotName % "\n" %
                                tr("Flight date:") % " " % unit.formatDateTime(header.flightDateTimeUtc);
    flight.setDescription(description);
}

void IGCImportPlugin::updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept
{}

// PROTECTED SLOTS

void IGCImportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

void IGCImportPlugin::updateWaypoints() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();

    FlightPlan &flightPlan = aircraft.getFlightPlan();
    if (position.count() > 0) {
        Analytics analytics(aircraft);
        const QDateTime startDateTimeUtc = d->igcParser.getHeader().flightDateTimeUtc;
        const PositionData firstPositionData = position.getFirst();
        const PositionData lastPositionData = position.getLast();
        const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);
        // Typically the takeoff and landing location are repeated in the IGC task list,
        // e.g. the takeoff airport and the actual takeoff point; those points can be
        // identical. So to ensure that each waypoint gets assigned a unique timestamp
        // we increment a given timestamp for as long as it does not exist in the
        // 'timestamps' set. Also note that while the aircraft is expected to reach
        // the waypoints in order of the task list that is actually not guaranteed;
        // depending on how much fun the pilot had in the cockpit ;)
        std::unordered_set<std::int64_t> timestamps;
        const std::vector<IGCParser::TaskItem> tasks = d->igcParser.getTask().tasks;
        const int nofTasks = tasks.size();
        for (int i = 0; i < nofTasks; ++i) {

            const IGCParser::TaskItem &item = tasks[i];
            Waypoint waypoint;
            waypoint.latitude = item.latitude;
            waypoint.longitude = item.longitude;
            waypoint.identifier = item.description;

            std::int64_t uniqueTimestamp;

            // The first and last waypoint always contain the start- respectively
            // end date & time.
            // The second and second-last waypoints are special in that they are
            // typically identical ("on the same airport") with the first
            // respectively last waypoint; in fact, with the same departure and
            // arrival airport those four waypoints may define the same point; if
            // that is the case then they will be assigned the same start- respectively
            // end date & time as the first and last flown position.
            // All other waypoints are considered "turn points" and will be assigned
            // the timestamp of the closest position. The 'timestamps' set ensures
            // that all assigned timestamps are unique (in order to satisfy the
            // uniqueness requirement of the persistence layer)
            if (i == 0) {
                // First waypoint
                waypoint.altitude = firstPositionData.altitude;
                waypoint.localTime = startDateTimeUtc.toLocalTime();
                waypoint.zuluTime = startDateTimeUtc;
                uniqueTimestamp = i;
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
            } else if (i == 1 && i != nofTasks - 1) {
                // Second (but not last) waypoint
                const IGCParser::TaskItem firstItem = tasks[0];
                if (SkyMath::isSameWaypoint(SkyMath::Coordinate(item.latitude, item.longitude),
                                            SkyMath::Coordinate(firstItem.latitude, firstItem.longitude),
                                            SameWaypointDistanceThreshold)) {
                    waypoint.altitude = firstPositionData.altitude;
                    waypoint.localTime = startDateTimeUtc.toLocalTime();
                    waypoint.zuluTime = startDateTimeUtc;
                    uniqueTimestamp = i;
                    waypoint.timestamp = uniqueTimestamp;
                    timestamps.insert(uniqueTimestamp);
                }

            } else if (i == nofTasks - 2 && i != 1) {
                // Second last (but not second) waypoint
                const IGCParser::TaskItem lastItem = tasks[nofTasks - 1];
                if (SkyMath::isSameWaypoint(SkyMath::Coordinate(item.latitude, item.longitude),
                                            SkyMath::Coordinate(lastItem.latitude, lastItem.longitude),
                                            SameWaypointDistanceThreshold)) {
                    waypoint.altitude = lastPositionData.altitude;
                    waypoint.localTime = endDateTimeUtc.toLocalTime();
                    waypoint.zuluTime = endDateTimeUtc;
                    uniqueTimestamp = lastPositionData.timestamp - 1;
                    while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
                        ++uniqueTimestamp;
                    }
                    waypoint.timestamp = uniqueTimestamp;
                    timestamps.insert(uniqueTimestamp);
                }
            } else if (i == nofTasks - 1) {
                // Last waypoint
                waypoint.altitude = lastPositionData.altitude;
                waypoint.localTime = endDateTimeUtc.toLocalTime();
                waypoint.zuluTime = endDateTimeUtc;
                uniqueTimestamp = lastPositionData.timestamp;
                while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
                    ++uniqueTimestamp;
                }
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
            }

            // If at this point no date & times have been assigned (especially for
            // the second and second to last task waypoint) then the task item is
            // considered to be a "turn point", in which case we search the closest
            // flown position
            if (waypoint.timestamp == TimeVariableData::InvalidTime) {
                const PositionData &closestPositionData = analytics.closestPosition(waypoint.latitude, waypoint.longitude);
                waypoint.altitude = closestPositionData.altitude;
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(closestPositionData.timestamp);
                waypoint.localTime = dateTimeUtc.toLocalTime();
                waypoint.zuluTime = dateTimeUtc;
                uniqueTimestamp = closestPositionData.timestamp;
                while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
                    ++uniqueTimestamp;
                }
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
            }

            flightPlan.add(std::move(waypoint));
        }
    } else {
        // No positions - use timestamps 0, 1, 2, ...
        std::int64_t currentWaypointTimestamp = 0;
        for (const IGCParser::TaskItem &item : d->igcParser.getTask().tasks) {
            Waypoint waypoint;
            waypoint.latitude = item.latitude;
            waypoint.longitude = item.longitude;
            waypoint.identifier = item.description;
            waypoint.timestamp = currentWaypointTimestamp;
            ++currentWaypointTimestamp;
            flightPlan.add(std::move(waypoint));
        }
    }
}

inline double IGCImportPlugin::noiseToPosition(double environmentalNoiseLevel, double threhsold) noexcept
{
    const double linear = qMax(environmentalNoiseLevel - threhsold, 0.0) / (1.0 - threhsold);
    return d->throttleResponseCurve.valueForProgress(linear);
}
