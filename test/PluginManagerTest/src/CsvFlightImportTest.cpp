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
#include <QCoreApplication>

#include <Kernel/Settings.h>
#include <Kernel/Version.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <PluginManager/PluginManager.h>
#include "CsvFlightImportTest.h"

namespace
{
    constexpr const char *FormatKey = "Format";
}

namespace Uuid
{
    // CsvImportPlugin.json
    constexpr const char *Csv = "077448de-4909-4c5e-8957-2347afee6708";
}

// PRIVATE SLOTS

void CsvFlightImportTest::initTestCase() noexcept
{
    QCoreApplication::setOrganizationName(Version::getOrganisationName());
    QCoreApplication::setApplicationName(Version::getApplicationName());

    m_oldPluginFormat = getPluginFormat();
    setPluginFormat(1);

    PluginManager &pluginManager = PluginManager::getInstance();

    // Flight import
    const std::vector<PluginManager::Handle> flightImportPlugins = pluginManager.initialiseFlightImportPlugins();

    QVERIFY(flightImportPlugins.size() > 0);
}

void CsvFlightImportTest::cleanupTestCase() noexcept
{
    setPluginFormat(m_oldPluginFormat);
}

void CsvFlightImportTest::parseFlightRadar24_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("expectedHasRecording");
    QTest::addColumn<int>("expectedNofFlights");
    QTest::addColumn<int>("expectedUserAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("expectedNofAircraftInFirstFlight");
    QTest::addColumn<int>("expectedNofUserAircraftPositionInFirstFlight");

    QTest::newRow("FlightRadar24-valid-1.csv")   << ":/test/csv/FlightRadar24-valid-1.csv"   << true  << true  << 1 << 0 << 1 << 2;
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
    QFETCH(bool, expectedHasRecording);
    QFETCH(int, expectedNofFlights);
    QFETCH(int, expectedUserAircraftIndexOfFirstFlight);
    QFETCH(int, expectedNofAircraftInFirstFlight);
    QFETCH(int, expectedNofUserAircraftPositionInFirstFlight);

    // Exercise
    QFile file {filepath};
    bool fileOpenOk = file.open(QIODeviceBase::ReadOnly);
    const std::vector<FlightData> flights = PluginManager::getInstance().importSelectedFlights(QUuid(Uuid::Csv), file, ok);
    file.close();

    // Verify
    QVERIFY(fileOpenOk);
    QVERIFY(ok == expectedOk);
    for (const FlightData &flightData : flights) {
        QVERIFY(flightData.hasRecording() == expectedHasRecording);
    }

    std::size_t nofFlights = flights.size();
    QCOMPARE_EQ(static_cast<int>(nofFlights), expectedNofFlights);
    if (nofFlights > 0) {
        const FlightData &firstFlight = flights.front();
        QCOMPARE_EQ(static_cast<int>(firstFlight.userAircraftIndex), expectedUserAircraftIndexOfFirstFlight);
        std::size_t nofAircraft = firstFlight.count();
        QCOMPARE_EQ(static_cast<int>(nofAircraft), expectedNofAircraftInFirstFlight);
        if (nofAircraft > 0) {
            const Aircraft &aircraft = firstFlight.getUserAircraftConst();
            QCOMPARE_EQ(static_cast<int>(aircraft.getPosition().count()), expectedNofUserAircraftPositionInFirstFlight);
        }
    }
}

// PRIVATE

int CsvFlightImportTest::getPluginFormat() noexcept
{
    int format;
    Settings &settings = Settings::getInstance();

    Settings::KeysWithDefaults keysWithDefaults;
    Settings::KeyValue keyValue;
    Settings::ValuesByKey valuesByKey;

    keyValue.first = ::FormatKey;
    keyValue.second = 0;
    keysWithDefaults.push_back(keyValue);
    valuesByKey = settings.restorePluginSettings(QUuid(Uuid::Csv), keysWithDefaults);
    return valuesByKey[::FormatKey].toInt();
}

void CsvFlightImportTest::setPluginFormat(int format) noexcept
{
    Settings &settings = Settings::getInstance();
    Settings::KeyValues keyValues;
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = format;
    keyValues.push_back(keyValue);
    settings.storePluginSettings(QUuid(Uuid::Csv), keyValues);
}

QTEST_MAIN(CsvFlightImportTest)
