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

#include <Kernel//Const.h>
#include <PluginManager/PluginManager.h>
#include "CsvFlightRecorderImportTest.h"

namespace
{
    constexpr const char *FormatKey {"Format"};
    constexpr const int FlightRecorderFormat {2};
}

// PRIVATE SLOTS

void CsvFlightRecorderImportTest::onInitTestCase() noexcept
{
    // Select the "FlightRecorder" format
    QUuid pluginUuid {Const::CsvImportPluginUuid};
    m_oldPluginFormat = getPluginSetting(pluginUuid, ::FormatKey, 0).toInt();
    setPluginSetting(pluginUuid, ::FormatKey, ::FlightRecorderFormat);

    // Initialise flight import plugins
    PluginManager &pluginManager = PluginManager::getInstance();
    const std::vector<PluginManager::Handle> flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    QVERIFY(flightImportPlugins.size() > 0);
}

void CsvFlightRecorderImportTest::onCleanupTestCase() noexcept
{
    QUuid pluginUuid {Const::CsvImportPluginUuid};
    setPluginSetting(pluginUuid, ::FormatKey, m_oldPluginFormat);
}

void CsvFlightRecorderImportTest::initTestCase_data() noexcept
{
    QUuid pluginUuid {Const::CsvImportPluginUuid};
    QTest::addColumn<QUuid>("pluginUuid");
    QTest::newRow("pluginUuid") << pluginUuid;
}

void CsvFlightRecorderImportTest::importSelectedFlights_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("expectedHasRecording");
    QTest::addColumn<int>("expectedNofFlights");
    QTest::addColumn<QDateTime>("expectedCreationTimeOfFirstFlight");
    QTest::addColumn<int>("expectedUserAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("expectedNofAircraftInFirstFlight");
    QTest::addColumn<int>("expectedNofUserAircraftPositionInFirstFlight");

    const QDateTime invalidDateTime;
    QTest::newRow("FlightRecorder-valid-1.csv")   << ":/test/csv/FlightRecorder-valid-1.csv"   << true  << true  << 1 << invalidDateTime << 0 << 1 << 2;
    QTest::newRow("Empty.csv")                    << ":/test/csv/Empty.csv"                    << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRecorder-invalid-1.csv") << ":/test/csv/FlightRecorder-invalid-1.csv" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRecorder-invalid-2.csv") << ":/test/csv/FlightRecorder-invalid-2.csv" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("FlightRecorder-invalid-3.csv") << ":/test/csv/FlightRecorder-invalid-3.csv" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
}

QTEST_MAIN(CsvFlightRecorderImportTest)
