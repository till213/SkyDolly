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
#include <QFile>
#include <QString>

#include <Model/FlightData.h>
#include <PluginManager/PluginManager.h>
#include "CsvFlightImportTest.h"

namespace Uuid
{
    // CsvImportPlugin.json
    constexpr const char *Csv = "077448de-4909-4c5e-8957-2347afee6708";
}

// PRIVATE SLOTS

void CsvFlightImportTest::initTestCase() noexcept
{
    std::vector<PluginManager::Handle> flightImportPlugins;

    PluginManager &pluginManager = PluginManager::getInstance();

    // Flight import
    flightImportPlugins = pluginManager.initialiseFlightImportPlugins();

    QVERIFY(flightImportPlugins.size() > 0);

    QSettings settings;
    settings.beginGroup(QString("Plugins/") + Uuid::Csv);
    {
        settings.setValue("Format", 1);
    }
    settings.endGroup();
}

void CsvFlightImportTest::cleanupTestCase() noexcept
{}

void CsvFlightImportTest::parseFlightRadar24_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("hasRecording");
    // Only relevant if 'expectedOk' is TRUE
    QTest::addColumn<int>("nofFlights");
    QTest::addColumn<int>("nofAircraftInFirstFlight");
    QTest::addColumn<int>("userAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("nofUserAircraftPositionInFirstFlight");

    QTest::newRow("FlightRadar24-valid-1.csv")   << ":/test/csv/FlightRadar24-valid-1.csv"   << true << true << 1 << 1 << 1 << 2;
    QTest::newRow("Empty.csv")                   << ":/test/csv/Empty.csv"                   << false << false << 0 << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-1.csv") << ":/test/csv/FlightRadar24-invalid-1.csv" << false << false << 0 << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-2.csv") << ":/test/csv/FlightRadar24-invalid-2.csv" << false << false << 0 << 0 << 0 << 0;
    QTest::newRow("FlightRadar24-invalid-3.csv") << ":/test/csv/FlightRadar24-invalid-3.csv" << false << false << 0 << 0 << 0 << 0;
}

void CsvFlightImportTest::parseFlightRadar24() noexcept
{
    bool ok {false};
    // Setup
    QFETCH(QString, filepath);
    QFETCH(bool, expectedOk);
    QFETCH(bool, hasRecording);
    QFETCH(int, nofFlights);
    QFETCH(int, nofAircraftInFirstFlight);
    QFETCH(int, userAircraftIndexOfFirstFlight);
    QFETCH(int, nofUserAircraftPositionInFirstFlight);

    // Exercise
    QFile file {filepath};
    bool fileOpenOk = file.open(QIODeviceBase::ReadOnly);
    const std::vector<FlightData> flights = PluginManager::getInstance().importSelectedFlights(QUuid(Uuid::Csv), file, ok);
    file.close();

    // Verify
    QVERIFY(fileOpenOk);
    QVERIFY(ok == expectedOk);
    for (const FlightData &flightData : flights) {
        QVERIFY(flightData.hasRecording() == hasRecording);
    }

    if (ok) {
        QCOMPARE_EQ(static_cast<int>(flights.size()), nofFlights);
    }
}

QTEST_MAIN(CsvFlightImportTest)
