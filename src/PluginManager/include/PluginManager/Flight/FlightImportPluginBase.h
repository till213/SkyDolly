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
#include "../DialogPluginBase.h"
#include "../PluginManagerLib.h"

class FlightService;
class Flight;
struct FlightData;
struct AircraftType;
struct AircraftInfo;
struct FlightCondition;
class FlightImportPluginBaseSettings;
struct FlightImportPluginBasePrivate;

class PLUGINMANAGER_API FlightImportPluginBase : public QObject, public FlightImportIntf, public DialogPluginBase
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
        return DialogPluginBase::getParentWidget();
    }

    void setParentWidget(QWidget *parent) noexcept final
    {
        DialogPluginBase::setParentWidget(parent);
    }

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        DialogPluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        DialogPluginBase::restoreSettings(pluginUuid);
    }

    bool importFlights(Flight &flight) noexcept final;

protected:
    AircraftType &getSelectedAircraftType() const noexcept;

    // Re-implement
    virtual FlightImportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    /*!
     * Imports the flight data from the given \c io data source and returns the list
     * of imported FlightData. Note that \c ok is also set to \c false in case
     * no flight data was imported at all (despite the existing file not having
     * any syntax errors).
     *
     * The datasource \c io must have been properly opened for reading already.
     *
     * \param io
     *        the IO device to read from; already opened for reading
     * \param ok
     *        is set to \c true in case of success; \c false else (a parse/read error occured
     *        or otherwise no data imported)
     * \return the list of imported flight data
     */
    virtual std::vector<FlightData> importSelectedFlights(QIODevice &io, bool &ok) noexcept = 0;

    virtual FlightAugmentation::Procedures getAugmentationProcedures() const noexcept = 0;
    virtual FlightAugmentation::Aspects getAugmentationAspects() const noexcept = 0;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept final;

private:
    const std::unique_ptr<FlightImportPluginBasePrivate> d;

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
