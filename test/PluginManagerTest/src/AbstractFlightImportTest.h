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
#ifndef ABSTRACTFLIGHTIMPORTTEST_H
#define ABSTRACTFLIGHTIMPORTTEST_H

#include <QObject>
#include <QString>
#include <QVariant>

/*!
 * Abstract base class for flight import plugins, allowing to
 * modify plugin settings before instantiating the plugins.
 */
class AbstractFlightImportTest : public QObject
{
    Q_OBJECT
protected:
    static QVariant getPluginSetting(QUuid pluginUuid, const QString &key, QVariant defaultValue) noexcept;
    static void setPluginSetting(QUuid pluginUuid, const QString &key, QVariant value) noexcept;

    virtual void onInitTestCase() noexcept = 0;
    virtual void onCleanupTestCase() noexcept = 0;

private slots:
    virtual void initTestCase() noexcept final;
    virtual void cleanupTestCase() noexcept final;

    /*!
     * Set the following global test data:
     *
     * - QUuid: "pluginUuid"
     */
    virtual void initTestCase_data() noexcept = 0;

    /*!
     * Set the following test data:
     *
     * - QString: "filepath"
     * - bool: "expectedOk";
     * - bool: "expectedHasRecording"
     * - int: "expectedNofFlights"
     * - QDateTime: "expectedCreationTimeOfFirstFlight"
     * - int: "expectedUserAircraftIndexOfFirstFlight"
     * - int: "expectedNofAircraftInFirstFlight"
     * - int: "expectedNofUserAircraftPositionInFirstFlight"
     */
    virtual void importSelectedFlights_data() noexcept = 0;

    /*!
     * Tests the PluginManager#importSelectedFlights method with
     * the test data defined in initTestCase_data() (-> plugin UUID)
     * and importSelectedFlights_data().
     */
    void importSelectedFlights() noexcept;
};

#endif // ABSTRACTFLIGHTIMPORTTEST_H
