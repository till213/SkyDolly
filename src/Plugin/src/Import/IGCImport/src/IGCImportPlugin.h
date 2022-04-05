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
#include <QString>
#include <QWidget>

class QRegularExpression;

#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../ImportIntf.h"
#include "../../../ImportPluginBase.h"

class Flight;
struct AircraftInfo;
struct FlightCondition;
class ImportPluginBaseSettings;
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
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    virtual void restoreSettingsExtn(Settings::ValuesByKey) noexcept override;
    virtual ImportPluginBaseSettings &getSettings() const noexcept override;
    virtual QString getFileFilter() const noexcept override;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    virtual bool readFile(QFile &file) noexcept override;

    virtual FlightAugmentation::Procedures getProcedures() const noexcept override;
    virtual FlightAugmentation::Aspects getAspects() const noexcept override;
    virtual QDateTime getStartDateTimeUtc() noexcept override;
    virtual QString getTitle() const noexcept override;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept override;
    virtual void updateExtendedFlightInfo(Flight &flight) noexcept override;
    virtual void updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept override;

protected slots:
    virtual void onRestoreDefaultSettings() noexcept override;

private:
    std::unique_ptr<IGCImportPluginPrivate> d;

    void updateWaypoints() noexcept;

    // Estimates the propeller (thrust) lever position, based on the
    // environmentalNoiseLevel and the threshold
    inline double noiseToPosition(double environmentalNoiseLevel, double threshold) noexcept;
};

#endif // IGCIMPORTPLUGIN_H
