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
#include <vector>

#include <QtTest>
#include <QUuid>
#include <QDateTime>

#include <Kernel/Const.h>
#include <PluginManager/PluginManager.h>
#include "KmlFlightRadar24ImportTest.h"

namespace
{
    constexpr const char *FormatKey {"Format"};
    constexpr const int FlightRadar24Format {1};
}

// PRIVATE SLOTS

void KmlFlightRadar24ImportTest::onInitTestCase() noexcept
{
    // Select the "FlightRadar24" format
    QUuid pluginUuid {Const::KmlImportPluginUuid};
    m_oldPluginFormat = getPluginSetting(pluginUuid, ::FormatKey, 0).toInt();
    setPluginSetting(pluginUuid, ::FormatKey, ::FlightRadar24Format);

    // Initialise flight import plugins
    PluginManager &pluginManager = PluginManager::getInstance();
    const std::vector<PluginManager::Handle> flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    QVERIFY(flightImportPlugins.size() > 0);
}

void KmlFlightRadar24ImportTest::onCleanupTestCase() noexcept
{
    QUuid pluginUuid {Const::CsvImportPluginUuid};
    setPluginSetting(pluginUuid, ::FormatKey, m_oldPluginFormat);
}

void KmlFlightRadar24ImportTest::initTestCase_data() noexcept
{
    QUuid pluginUuid {Const::KmlImportPluginUuid};

    QTest::addColumn<QUuid>("pluginUuid");
    QTest::newRow("pluginUuid") << pluginUuid;
}

void KmlFlightRadar24ImportTest::importSelectedFlights_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("expectedHasRecording");
    QTest::addColumn<int>("expectedNofFlights");
    QTest::addColumn<QDateTime>("expectedCreationTimeOfFirstFlight");
    QTest::addColumn<int>("expectedUserAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("expectedNofAircraftInFirstFlight");
    QTest::addColumn<int>("expectedNofUserAircraftPositionInFirstFlight");

    const QDateTime validDateTime {QDateTime::fromString("2024-10-11T20:20:00Z", Qt::ISODate)};
    const QDateTime invalidDateTime;
    QTest::newRow("FlightRadar24-valid-1.kml")   << ":/test/kml/FlightRadar24-valid-1.kml"   << true  << true  << 1 << validDateTime   << 0 << 1 << 3;
    QTest::newRow("Empty.kml")                   << ":/test/kml/Empty.kml"                   << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-1.kml") << ":/test/kml/FlightRadar24-invalid-1.kml" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-2.kml") << ":/test/kml/FlightRadar24-invalid-2.kml" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-3.kml") << ":/test/kml/FlightRadar24-invalid-3.kml" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
}

QTEST_MAIN(KmlFlightRadar24ImportTest)
