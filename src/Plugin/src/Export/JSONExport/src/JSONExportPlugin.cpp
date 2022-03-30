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
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <cstdint>

#include <QIODevice>
#include <QStringBuilder>
#include <QString>
#include <QDateTime>

#include "../../../../../Kernel/src/Version.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/File.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../Export.h"
#include "JSONExportSettings.h"
#include "JSONExportPlugin.h"

class JSONExportPluginPrivate
{
public:
    JSONExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    JSONExportSettings settings;
    Flight &flight;

    static inline const QString FileExtension {QStringLiteral("json")};
};

// PUBLIC

JSONExportPlugin::JSONExportPlugin() noexcept
    : d(std::make_unique<JSONExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("JSONExportPlugin::JSONExportPlugin: PLUGIN LOADED");
#endif
}

JSONExportPlugin::~JSONExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("JSONExportPlugin::~JSONExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ExportPluginBaseSettings &JSONExportPlugin::getSettings() const noexcept
{
    return d->settings;
}

void JSONExportPlugin::addSettingsExtn(Settings::PluginSettings &settings) const noexcept
{
    d->settings.addSettings(settings);
}

void JSONExportPlugin::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    d->settings.addKeysWithDefaults(keysWithDefaults);
}

void JSONExportPlugin::restoreSettingsExtn(Settings::ValuesByKey valuesByKey) noexcept
{
    d->settings.restoreSettings(valuesByKey);
}

QString JSONExportPlugin::getFileExtension() const noexcept
{
    return JSONExportPluginPrivate::FileExtension;
}

QString JSONExportPlugin::getFileFilter() const noexcept
{
    return tr("JavaScript Object Notation(*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> JSONExportPlugin::createOptionWidget() const noexcept
{
    // No custom settings yet
    return nullptr;
}

bool JSONExportPlugin::writeFile(QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportWaypoints(io);
    }
    if (ok) {
        ok = exportAllAircraft(io);
    }
    if (ok) {
        ok = exportFooter(io);
    }

    return ok;
}

// PROTECTED SLOTS

void JSONExportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

bool JSONExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"{\n";
    return io.write(header.toUtf8());
}

bool JSONExportPlugin::exportAllAircraft(QIODevice &io) const noexcept
{
    bool ok = true;
    for (const auto &aircraft : d->flight) {
        ok = exportAircraft(*aircraft, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool JSONExportPlugin::exportAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    std::vector<PositionData> interpolatedPositionData;
    resamplePositionDataForExport(aircraft, std::back_inserter(interpolatedPositionData));
    bool ok = true;
    if (interpolatedPositionData.size() > 0) {
        // @todo IMPLEMEMENT ME!!!
    } // size

    return ok;
}

bool JSONExportPlugin::exportWaypoints(QIODevice &io) const noexcept
{
    bool ok = true;
    const FlightPlan &flightPlan = d->flight.getUserAircraft().getFlightPlanConst();
    for (const Waypoint &waypoint : flightPlan) {
        ok = exportWaypoint(waypoint, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool JSONExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"}\n";
    return io.write(footer.toUtf8());
}

inline bool JSONExportPlugin::exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept
{
    const QString trackPoint =
"  { \"todo\" : \"implement line segments\" }\n";

    // @todo IMPLEMENT ME!!!

    return io.write(trackPoint.toUtf8());
}

inline bool JSONExportPlugin::exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept
{
    const QString waypointString =
"  { \"todo\" : \"implement waypoints\" }\n";

    // @todo IMPLEMENT ME!!!

    return io.write(waypointString.toUtf8());
}
