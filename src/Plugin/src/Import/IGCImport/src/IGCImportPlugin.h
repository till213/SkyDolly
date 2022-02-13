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
#ifndef IGCIMPORTPLUGIN_H
#define IGCIMPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QDateTime>
#include <QtPlugin>
#include <QStringView>

class QRegularExpression;

#include "../../../ImportIntf.h"
#include "../../../ImportPluginBase.h"

class IGCImportPluginPrivate;

class IGCImportPlugin : public ImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IMPORT_INTERFACE_IID FILE "IGCImportPlugin.json")
    Q_INTERFACES(ImportIntf)
public:
    IGCImportPlugin() noexcept;
    virtual ~IGCImportPlugin() noexcept;

protected:
    virtual bool readFile(QFile &file) noexcept override;
    virtual QDateTime getStartDateTimeUtc() noexcept override;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept override;
    virtual void updateFlight(const QFile &file) noexcept override;

private:
    std::unique_ptr<IGCImportPluginPrivate> d;

    // A record, containing manufacturer ID
    bool readManufacturer() noexcept;
    // All records
    bool readRecords() noexcept;

    bool parseHeader(const QByteArray &line) noexcept;
    bool parseHeaderDate(const QByteArray &line) noexcept;
    bool parseHeaderText(const QByteArray &line, const QRegularExpression &regExp, QString &text) noexcept;
    bool parseHeaderPilot(const QByteArray &line) noexcept;
    bool parseHeaderCoPilot(const QByteArray &line) noexcept;
    bool parseHeaderGliderType(const QByteArray &line) noexcept;
    bool parseHeaderGliderId(const QByteArray &line) noexcept;

    bool parseTask(const QByteArray &line) noexcept;
    bool parseFix(const QByteArray &line) noexcept;
    inline double parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text);

    void updateFlightInfo(const QFile &file) noexcept;
    void updateFlightCondition() noexcept;
    void updateWaypoints() noexcept;

    // Estimates the propeller (thrust) lever position, based on the
    // environmentalNoiseLevel and the threshold
    inline double noiseToPosition(double environmentalNoiseLevel) noexcept;
};

#endif // IGCIMPORTPLUGIN_H
