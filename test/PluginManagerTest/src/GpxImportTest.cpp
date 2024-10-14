/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QtTest>
#include <QUuid>
#include <QDateTime>

#include <Kernel//Const.h>
#include <PluginManager/PluginManager.h>
#include "GpxImportTest.h"

namespace
{
    constexpr const char *WaypointSelectionKey {"WaypointSelection"};
    constexpr const char *PositionSelectionKey {"PositionSelection"};

    constexpr const int WaypointSelection {0};
    constexpr const int RouteSelection {1};
    constexpr const int TrackSelection {2};
}

// PRIVATE SLOTS

void GpxImportTest::onInitTestCase() noexcept
{
    // Select the "FlightRecorder" format
    QUuid pluginUuid {Const::GpxImportPluginUuid};
    m_oldWaypointSelection = getPluginSetting(pluginUuid, ::WaypointSelectionKey, 0).toInt();
    setPluginSetting(pluginUuid, ::WaypointSelectionKey, ::WaypointSelection);
    m_oldPositionSelection = getPluginSetting(pluginUuid, ::PositionSelectionKey, 0).toInt();
    setPluginSetting(pluginUuid, ::PositionSelectionKey, ::TrackSelection);

    // Initialise flight import plugins
    PluginManager &pluginManager = PluginManager::getInstance();
    const std::vector<PluginManager::Handle> flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    QVERIFY(flightImportPlugins.size() > 0);
}

void GpxImportTest::onCleanupTestCase() noexcept
{
    QUuid pluginUuid {Const::GpxImportPluginUuid};
    setPluginSetting(pluginUuid, ::WaypointSelectionKey, m_oldWaypointSelection);
    setPluginSetting(pluginUuid, ::PositionSelectionKey, m_oldPositionSelection);
}

void GpxImportTest::initTestCase_data() noexcept
{
    QUuid pluginUuid {Const::GpxImportPluginUuid};
    QTest::addColumn<QUuid>("pluginUuid");
    QTest::newRow("pluginUuid") << pluginUuid;
}

void GpxImportTest::importSelectedFlights_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("expectedHasRecording");
    QTest::addColumn<int>("expectedNofFlights");
    QTest::addColumn<QDateTime>("expectedCreationTimeOfFirstFlight");
    QTest::addColumn<int>("expectedUserAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("expectedNofAircraftInFirstFlight");
    QTest::addColumn<int>("expectedNofUserAircraftPositionInFirstFlight");

    const QDateTime validDateTime {QDateTime::fromString("2023-02-18T16:10:06Z", Qt::ISODate)};
    const QDateTime invalidDateTime;
    QTest::newRow("Valid-1.gpx")   << ":/test/gpx/Valid-1.gpx"   << true  << true  << 1 << validDateTime   << 0 << 1 << 3;
    QTest::newRow("Empty.gpx")     << ":/test/gpx/Empty.gpx"     << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-1.gpx") << ":/test/gpx/Invalid-1.gpx" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-2.gpx") << ":/test/gpx/Invalid-2.gpx" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-3.gpx") << ":/test/gpx/Invalid-3.gpx" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
}

QTEST_MAIN(GpxImportTest)
