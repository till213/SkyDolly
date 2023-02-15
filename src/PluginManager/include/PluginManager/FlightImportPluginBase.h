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
#ifndef FLIGHTIMPORTPLUGINBASE_H
#define FLIGHTIMPORTPLUGINBASE_H

#include <memory>
#include <vector>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QIODevice;

#include <Kernel/Settings.h>
#include <Flight/FlightAugmentation.h>
#include "FlightImportIntf.h"
#include "PluginBase.h"
#include "PluginManagerLib.h"

class FlightService;
class Flight;
struct FlightData;
struct AircraftType;
struct AircraftInfo;
struct FlightCondition;
class FlightImportPluginBaseSettings;
struct FlightImportPluginBasePrivate;

class PLUGINMANAGER_API FlightImportPluginBase : public PluginBase, public FlightImportIntf
{
    Q_OBJECT
    Q_INTERFACES(FlightImportIntf)
public:
    FlightImportPluginBase() noexcept;
    FlightImportPluginBase(const FlightImportPluginBase &rhs) = delete;
    FlightImportPluginBase(FlightImportPluginBase &&rhs) = delete;
    FlightImportPluginBase &operator=(const FlightImportPluginBase &rhs) = delete;
    FlightImportPluginBase &operator=(FlightImportPluginBase &&rhs) = delete;
    ~FlightImportPluginBase() override;

    QWidget *getParentWidget() const noexcept final
    {
        return PluginBase::getParentWidget();
    }

    void setParentWidget(QWidget *parent) noexcept final
    {
        PluginBase::setParentWidget(parent);
    }

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    bool importFlights(Flight &flight) noexcept final;

protected:
    AircraftType &getSelectedAircraftType() const noexcept;

    // Re-implement
    virtual FlightImportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    virtual FlightAugmentation::Procedures getAugmentationProcedures() const noexcept = 0;
    virtual FlightAugmentation::Aspects getAugmentationAspects() const noexcept = 0;

private:
    const std::unique_ptr<FlightImportPluginBasePrivate> d;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept final;

    bool importFlights(const QStringList &filePaths, Flight &currentFlight) noexcept;
    void enrichFlightData(std::vector<FlightData> &flightData) const noexcept;
    void enrichFlightInfo(FlightData &flightData) const noexcept;
    void enrichFlightCondition(FlightData &flightData) const noexcept;
    void enrichAircraftInfo(FlightData &flightData) const noexcept;

    bool augmentFlights(std::vector<FlightData> &flightData) const noexcept;

    bool addAndStoreAircraftToCurrentFlight(const QString &sourceFilePath, std::vector<FlightData> importedFlights, Flight &currentFlight,
                                            std::size_t &totalFlightsStored, std::size_t &totalAircraftStored, bool &continueWithDirectoryImport) noexcept;
    bool storeFlightData(std::vector<FlightData> &importedFlights, std::size_t &totalFlightsStored);

    void confirmImportError(const QString &sourceFilePath, bool &ignoreFailures, bool &continueWithDirectoryImport) noexcept;
    void confirmMultiFlightImport(const QString &sourceFilePath, std::size_t nofFlights, bool &doAdd, bool &continueWithDirectoryImport);

    static void syncAircraftTimeOffset(const Flight &currentFlight, std::vector<FlightData> &importedFlights) noexcept;
};

#endif // FLIGHTIMPORTPLUGINBASE_H
