/**
 * Sky Dolly - The Black Sheep for Your Location Recordings
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
#include "LocationImportPluginBaseSettings.h"
#include "LocationImportPluginBase.h"

struct LocationImportPluginBasePrivate
{
    LocationImportPluginBasePrivate()
    {}
};

// PUBLIC

LocationImportPluginBase::LocationImportPluginBase() noexcept
    : d(std::make_unique<LocationImportPluginBasePrivate>())
{}

LocationImportPluginBase::~LocationImportPluginBase() = default;

bool LocationImportPluginBase::importLocation(LocationService &locationService) noexcept
{
    bool ok {false};
    LocationImportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicLocationImportDialog> importDialog = std::make_unique<BasicLocationImportDialog>(flight, getFileFilter(), baseSettings, PluginBase::getParentWidget());
    // Transfer ownership to importDialog
    importDialog->setOptionWidget(optionWidget.release());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        QStringList selectedFilePaths;
        // Remember import (export) path
        const QString selectedPath = importDialog->getSelectedPath();
        if (baseSettings.isImportDirectoryEnabled()) {
            Settings::getInstance().setExportPath(selectedPath);
            selectedFilePaths = File::getFilePaths(selectedPath, getFileSuffix());
        } else {
            const QString directoryPath = QFileInfo(selectedPath).absolutePath();
            Settings::getInstance().setExportPath(directoryPath);
            selectedFilePaths.append(selectedPath);
        }
        d->aircraftType = importDialog->getSelectedAircraftType(&ok);
        if (ok) {
#ifdef DEBUG
            QElapsedTimer timer;
            timer.start();
#endif
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
            ok = importFlights(selectedFilePaths, flightService, flight);
            QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
            qDebug() << QFileInfo(selectedPath).fileName() << "import" << (ok ? "SUCCESS" : "FAIL") << "in" << timer.elapsed() <<  "ms";
#endif
            if (!ok && !baseSettings.isImportDirectoryEnabled()) {
                QMessageBox::warning(PluginBase::getParentWidget(), tr("Import error"), tr("The file %1 could not be imported.").arg(selectedPath));
            }
        } else {
            QMessageBox::warning(PluginBase::getParentWidget(), tr("Import error"),
                                 tr("The selected aircraft '%1' is not a known aircraft in the logbook. "
                                    "Check for spelling errors or record a flight with this aircraft first.").arg(d->aircraftType.type));
        }
    } else {
        ok = true;
    }

    // We are done with the export
    d->flight = nullptr;

    return ok;
}


// PRIVATE

void LocationImportPluginBase::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void LocationImportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void LocationImportPluginBase::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}
