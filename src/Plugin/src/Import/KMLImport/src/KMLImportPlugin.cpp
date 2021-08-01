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
#include <memory>

#include <QIODevice>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QFlags>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../../../Persistence/src/Service/FlightService.h"
#include "../../../../../Persistence/src/Service/AircraftService.h"
#include "KMLImportDialog.h"
#include "KMLImportPlugin.h"

class KMLImportPluginPrivate
{
public:
    KMLImportPluginPrivate()
        : aircraftService(std::make_unique<AircraftService>())
    {}

    std::unique_ptr<AircraftService> aircraftService;
};

// PUBLIC

KMLImportPlugin::KMLImportPlugin() noexcept
    : d(std::make_unique<KMLImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::KMLImportPlugin: PLUGIN LOADED");
#endif
}

KMLImportPlugin::~KMLImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::~KMLImportPlugin: PLUGIN UNLOADED");
#endif
}

bool KMLImportPlugin::importData(FlightService &flightService) const noexcept
{
    bool ok;
    std::unique_ptr<KMLImportDialog> kmlImportDialog = std::make_unique<KMLImportDialog>(getParentWidget());
    const int choice = kmlImportDialog->exec();
    if (choice == QDialog::Accepted) {
        AircraftType aircraftType;
        ok = kmlImportDialog->getSelectedAircraftType(aircraftType);
        const bool addToCurrentFlight = kmlImportDialog->isAddToFlightEnabled();
        if (ok) {
            ok = import(kmlImportDialog->getSelectedFilePath(), aircraftType, flightService, addToCurrentFlight);
            if (ok && addToCurrentFlight) {
                std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
                if (skyConnect) {
                    skyConnect->get().updateAIObjects();
                }
            }
        }
    } else {
        ok = true;
    }
    return ok;
}

// PRIVATE

bool KMLImportPlugin::import(const QString &filePath, const AircraftType &aircraftType, FlightService &flightService, bool addToCurrentFlight) const noexcept
{
    bool ok = true;
    return ok;
}
