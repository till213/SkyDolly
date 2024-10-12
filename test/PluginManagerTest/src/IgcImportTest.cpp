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
#include <QtTest>
#include <QUuid>
#include <QDateTime>

#include <Kernel//Const.h>
#include <PluginManager/PluginManager.h>
#include "IgcImportTest.h"

namespace
{    
    constexpr const char *AltitudeKey {"Altitude"};
    constexpr const char *EnlThresholdKey {"EnlThreshold"};

    constexpr const int AltitudeSelection {0};
    constexpr const int EnlSelection {40};
}

// PRIVATE SLOTS

void IgcImportTest::onInitTestCase() noexcept
{
    // Select the "FlightRecorder" format
    QUuid pluginUuid {Const::IgcImportPluginUuid};
    m_oldAltitudeSelection = getPluginSetting(pluginUuid, ::AltitudeKey, 0).toInt();
    setPluginSetting(pluginUuid, ::AltitudeKey, ::AltitudeSelection);
    m_oldEnlSelection = getPluginSetting(pluginUuid, ::EnlThresholdKey, 0).toInt();
    setPluginSetting(pluginUuid, ::EnlThresholdKey, ::EnlSelection);

    // Initialise flight import plugins
    PluginManager &pluginManager = PluginManager::getInstance();
    const std::vector<PluginManager::Handle> flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    QVERIFY(flightImportPlugins.size() > 0);
}

void IgcImportTest::onCleanupTestCase() noexcept
{
    QUuid pluginUuid {Const::IgcImportPluginUuid};
    setPluginSetting(pluginUuid, ::AltitudeKey, m_oldAltitudeSelection);
    setPluginSetting(pluginUuid, ::EnlThresholdKey, m_oldEnlSelection);
}

void IgcImportTest::initTestCase_data() noexcept
{
    QUuid pluginUuid {Const::IgcImportPluginUuid};
    QTest::addColumn<QUuid>("pluginUuid");
    QTest::newRow("pluginUuid") << pluginUuid;
}

void IgcImportTest::importSelectedFlights_data() noexcept
{
    QTest::addColumn<QString>("filepath");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("expectedHasRecording");
    QTest::addColumn<int>("expectedNofFlights");
    QTest::addColumn<QDateTime>("expectedCreationTimeOfFirstFlight");
    QTest::addColumn<int>("expectedUserAircraftIndexOfFirstFlight");
    QTest::addColumn<int>("expectedNofAircraftInFirstFlight");
    QTest::addColumn<int>("expectedNofUserAircraftPositionInFirstFlight");

    const QDateTime validDateTime {QDateTime::fromString("2024-10-12T11:05:25Z", Qt::ISODate)};
    const QDateTime invalidDateTime;
    QTest::newRow("Valid-1.igc")   << ":/test/igc/Valid-1.igc"   << true  << true  << 1 << validDateTime   << 0 << 1 << 3;
    QTest::newRow("Empty.igc")     << ":/test/igc/Empty.igc"     << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-1.igc") << ":/test/igc/Invalid-1.igc" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-2.igc") << ":/test/igc/Invalid-2.igc" << false << false << 0 << invalidDateTime << 0 << 0 << 0;
    QTest::newRow("Invalid-3.igc") << ":/test/igc/Invalid-3.igc" << false << false << 0 << invalidDateTime << 0 << 0 << 0;

}

QTEST_MAIN(IgcImportTest)
