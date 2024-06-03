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
#include <QFile>
#include <QString>
#include <QVariant>
#include <QCoreApplication>

#include <Kernel/Version.h>
#include <Kernel/Settings.h>

#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <PluginManager/PluginManager.h>
#include "AbstractFlightImportTest.h"

// PRIVATE

QVariant AbstractFlightImportTest::getPluginSetting(QUuid pluginUuid, const QString &key, QVariant defaultValue) noexcept
{
    Settings &settings = Settings::getInstance();

    Settings::KeysWithDefaults keysWithDefaults;
    Settings::KeyValue keyValue;
    Settings::ValuesByKey valuesByKey;

    keyValue.first = key;
    keyValue.second = std::move(defaultValue);
    keysWithDefaults.push_back(keyValue);
    valuesByKey = settings.restorePluginSettings(pluginUuid, keysWithDefaults);
    return valuesByKey[key];
}

void AbstractFlightImportTest::setPluginSetting(QUuid pluginUuid, const QString &key, QVariant value) noexcept
{
    Settings &settings = Settings::getInstance();
    Settings::KeyValues keyValues;
    Settings::KeyValue keyValue;

    keyValue.first = key;
    keyValue.second = std::move(value);
    keyValues.push_back(keyValue);
    settings.storePluginSettings(pluginUuid, keyValues);
}

// PRIVATE SLOTS

void AbstractFlightImportTest::initTestCase() noexcept
{
    QCoreApplication::setOrganizationName(Version::getOrganisationName());
    QCoreApplication::setApplicationName(Version::getApplicationName());
    onInitTestCase();
}

void AbstractFlightImportTest::cleanupTestCase() noexcept
{
    onCleanupTestCase();
}

void AbstractFlightImportTest::importSelectedFlights() noexcept
{
    bool ok {false};
    // Setup
    QFETCH_GLOBAL(QUuid, pluginUuid);
    QFETCH(QString, filepath);
    QFETCH(bool, expectedOk);
    QFETCH(bool, expectedHasRecording);
    QFETCH(int, expectedNofFlights);
    QFETCH(QDateTime, expectedCreationTimeOfFirstFlight);
    QFETCH(int, expectedUserAircraftIndexOfFirstFlight);
    QFETCH(int, expectedNofAircraftInFirstFlight);
    QFETCH(int, expectedNofUserAircraftPositionInFirstFlight);

    // Exercise
    QFile file {filepath};
    bool fileOpenOk = file.open(QIODeviceBase::ReadOnly);
    const std::vector<FlightData> flights = PluginManager::getInstance().importFlightData(pluginUuid, file, ok);
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
        QCOMPARE_EQ(firstFlight.creationTime, expectedCreationTimeOfFirstFlight);
        std::size_t nofAircraft = firstFlight.count();
        QCOMPARE_EQ(static_cast<int>(nofAircraft), expectedNofAircraftInFirstFlight);
        if (nofAircraft > 0) {
            const auto &aircraft = firstFlight.getUserAircraftConst();
            QCOMPARE_EQ(static_cast<int>(aircraft.getPosition().count()), expectedNofUserAircraftPositionInFirstFlight);
        }
    }
}
