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
#include <QCoreApplication>
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/File.h"
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
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../src/Export.h"
#include "IGCExportDialog.h"
#include "IGCExportSettings.h"
#include "IGCExportPlugin.h"

class IGCExportPluginPrivate
{
public:
    IGCExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    IGCExportSettings exportSettings;
    Flight &flight;
    Unit unit;
};

// PUBLIC

IGCExportPlugin::IGCExportPlugin() noexcept
    : d(std::make_unique<IGCExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IGCExportPlugin::IGCExportPlugin: PLUGIN LOADED");
#endif
}

IGCExportPlugin::~IGCExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IGCExportPlugin::~IGCExportPlugin: PLUGIN UNLOADED");
#endif
}

bool IGCExportPlugin::exportData() noexcept
{
    bool ok;
    std::unique_ptr<IGCExportDialog> exportDialog = std::make_unique<IGCExportDialog>(d->exportSettings, getParentWidget());
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString exportDirectoryPath = QFileInfo(exportDialog->getSelectedFilePath()).absolutePath();
        Settings::getInstance().setExportPath(exportDirectoryPath);
        const QString filePath = File::ensureSuffix(exportDialog->getSelectedFilePath(), IGCExportDialog::FileSuffix);
        if (!filePath.isEmpty()) {

            const int nofAircraft = d->flight.count();

            QFile file(filePath);
            ok = file.open(QIODevice::WriteOnly);

            file.close();

        } else {
            ok = true;
        }

        if (ok) {
            if (exportDialog->doOpenExportedFile()) {
                const QString fileUrl = QString("file:///") + filePath;
                QDesktopServices::openUrl(QUrl(fileUrl));
            }
        } else {
            QMessageBox::critical(getParentWidget(), tr("Export error"), tr("The IGC file %1 could not be exported.").arg(filePath));
        }

    } else {
        ok = true;
    }

    return ok;
}

