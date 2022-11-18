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
#include <memory>

#include <QWidget>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QDateTime>
#include <QElapsedTimer>
#include <QCursor>
#include <QGuiApplication>

#include <Kernel/File.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/PersistenceManager.h>
#include "BasicLocationImportDialog.h"
#include "LocationImportPluginBaseSettings.h"
#include "LocationImportPluginBase.h"

struct LocationImportPluginBasePrivate
{
    QFile file;
};

// PUBLIC

LocationImportPluginBase::LocationImportPluginBase() noexcept
    : d(std::make_unique<LocationImportPluginBasePrivate>())
{}

LocationImportPluginBase::~LocationImportPluginBase() = default;

bool LocationImportPluginBase::importLocations(LocationService &locationService) noexcept
{
    bool ok {true};
    LocationImportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicLocationImportDialog> importDialog = std::make_unique<BasicLocationImportDialog>(getFileFilter(), baseSettings, PluginBase::getParentWidget());
    // Transfer ownership to importDialog
    importDialog->setOptionWidget(optionWidget.release());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        QStringList selectedFilePaths;
        // Remember import (export) path
        const QString selectedPath = importDialog->getSelectedPath();
        if (baseSettings.isImportDirectoryEnabled()) {
            Settings::getInstance().setExportPath(selectedPath);
            selectedFilePaths = File::getFilePaths(selectedPath, getFileExtension());
        } else {
            const QString directoryPath = QFileInfo(selectedPath).absolutePath();
            Settings::getInstance().setExportPath(directoryPath);
            selectedFilePaths.append(selectedPath);
        }

#ifdef DEBUG
        QElapsedTimer timer;
        timer.start();
#endif
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        QGuiApplication::processEvents();
        ok = importLocations(selectedFilePaths, locationService);
        QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
        qDebug() << QFileInfo(selectedPath).fileName() << "import" << (ok ? "SUCCESS" : "FAIL") << "in" << timer.elapsed() <<  "ms";
#endif
        if (!ok && !baseSettings.isImportDirectoryEnabled()) {
            QMessageBox::warning(PluginBase::getParentWidget(), tr("Import error"), tr("The file %1 could not be imported.").arg(selectedPath));
        }

    }

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

bool LocationImportPluginBase::importLocations(const QStringList &filePaths, LocationService &locationService) noexcept
{
    const LocationImportPluginBaseSettings &pluginSettings = getPluginSettings();
    const bool importDirectory = pluginSettings.isImportDirectoryEnabled();

    bool ok {true};
    bool ignoreFailures {false};
    for (const QString &filePath : filePaths) {
        d->file.setFileName(filePath);
        ok = d->file.open(QIODevice::ReadOnly);
        if (ok) {
            std::vector<Location> locations = importLocations(d->file, &ok);
            d->file.close();
            if (ok) {
                ok = storeLocations(locations, locationService);
            }
        }

        if (!ok && importDirectory && !ignoreFailures) {
            QGuiApplication::restoreOverrideCursor();
            QFileInfo fileInfo {filePath};
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(PluginBase::getParentWidget());
            messageBox->setIcon(QMessageBox::Warning);
            QPushButton *proceedButton = messageBox->addButton(tr("&Proceed"), QMessageBox::AcceptRole);
            QPushButton *ignoreAllButton = messageBox->addButton(tr("&Ignore All Failures"), QMessageBox::YesRole);
            messageBox->setWindowTitle(tr("Import Failure"));
            messageBox->setText(tr("The file %1 could not be imported. Do you want to proceed with the remaining files in directory %2?").arg(fileInfo.fileName(), fileInfo.dir().dirName()));
            messageBox->setInformativeText(tr("Aborting will keep the already successfully imported flights and aircraft."));
            messageBox->setStandardButtons(QMessageBox::Cancel);
            messageBox->setDefaultButton(proceedButton);

            messageBox->exec();
            const QAbstractButton *clickedButton = messageBox->clickedButton();
            if (clickedButton == ignoreAllButton) {
                ignoreFailures = true;
            } else if (clickedButton != proceedButton) {
                break;
            }
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
        }

    } // All files

    return ok;
}

bool LocationImportPluginBase::storeLocations(std::vector<Location> &locations, LocationService &locationService) const noexcept
{
    // TODO Make the mode a setting
    const bool ok = locationService.storeAll(locations, LocationService::Mode::Update);
    if (ok) {
         emit PersistenceManager::getInstance().locationsImported();
    }
    return ok;
}
