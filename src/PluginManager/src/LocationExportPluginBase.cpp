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
#include "BasicLocationExportDialog.h"
#include "LocationExportPluginBaseSettings.h"
#include "LocationExportPluginBase.h"

struct LocationExportPluginBasePrivate
{
    QFile file;
};

// PUBLIC

LocationExportPluginBase::LocationExportPluginBase() noexcept
    : d(std::make_unique<LocationExportPluginBasePrivate>())
{}

LocationExportPluginBase::~LocationExportPluginBase() = default;

bool LocationExportPluginBase::exportLocation() noexcept
{
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    LocationExportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<BasicLocationExportDialog> exportDialog = std::make_unique<BasicLocationExportDialog>(getFileSuffix(), getFileFilter(), baseSettings, PluginBase::getParentWidget());
    // Transfer ownership to exportDialog
    exportDialog->setOptionWidget(optionWidget.release());
    bool ok {true};
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString selectedFilePath = exportDialog->getSelectedFilePath();
        if (!selectedFilePath.isEmpty()) {
            const QString filePath = File::ensureSuffix(selectedFilePath, getFileSuffix());
            const QFileInfo fileInfo {filePath};
            const QString exportDirectoryPath = fileInfo.absolutePath();
            Settings::getInstance().setExportPath(exportDirectoryPath);

            const LocationExportPluginBaseSettings::FormationExport formationExport = getPluginSettings().getFormationExport();
            ok = exportLocation(location, filePath);

        }
    }

    return ok;
}

// PRIVATE

bool LocationExportPluginBase::exportLocation(const Location &location, const QString &filePath) noexcept
{
    d->exportedFilePaths.clear();
    QFile file(filePath);
    bool ok {true};
#ifdef DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QGuiApplication::processEvents();
    const LocationExportPluginBaseSettings &settings = getPluginSettings();
    switch (settings.getFormationExport()) {
    case LocationExportPluginBaseSettings::FormationExport::UserAircraftOnly:
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            ok = exportAircraft(location, location.getUserAircraft(), file);
            d->exportedFilePaths.push_back(filePath);
        }
        file.close();
        break;
    case LocationExportPluginBaseSettings::FormationExport::AllAircraftOneFile:
        if (hasMultiAircraftSupport()) {
            ok = file.open(QIODevice::WriteOnly);
            if (ok) {
                ok = exportLocation(location, file);
                d->exportedFilePaths.push_back(filePath);
            }
            file.close();
        } else {
            ok = exportAllAircraft(location, filePath);
        }
        break;
    case LocationExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles:
        ok = exportAllAircraft(location, filePath);
        break;
    }
    QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
    qDebug() << QFileInfo(filePath).fileName() << "export" << (ok ? "SUCCESS" : "FAIL") << "in" << timer.elapsed() <<  "ms";
#endif

    if (ok) {
        if (settings.isOpenExportedFilesEnabled()) {
            for (const QString &exportedFilePath : d->exportedFilePaths) {
                const QString fileUrl = QString("file:///") + exportedFilePath;
                QDesktopServices::openUrl(QUrl(fileUrl));
            }
        }
    } else {
        QMessageBox::warning(PluginBase::getParentWidget(), tr("Export error"), tr("An error occured during export into file %1.").arg(QDir::toNativeSeparators(filePath)));
    }

    return ok;
}

bool LocationExportPluginBase::exportAllAircraft(const Location &location, const QString &filePath) noexcept
{
    bool ok {true};
    bool replaceAll {false};
    int i {1};
    for (const auto &aircraft : location) {
        // Don't append sequence numbers if location has only one aircraft
        const QString sequencedFilePath = location.count() > 1 ? File::getSequenceFilePath(filePath, i) : filePath;
        const QFileInfo fileInfo {sequencedFilePath};
        if (fileInfo.exists() && !replaceAll) {
            QGuiApplication::restoreOverrideCursor();
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(PluginBase::getParentWidget());
            messageBox->setIcon(QMessageBox::Question);
            QPushButton *replaceButton = messageBox->addButton(tr("&Replace"), QMessageBox::AcceptRole);
            QPushButton *replaceAllButton = messageBox->addButton(tr("Replace &All"), QMessageBox::YesRole);
            messageBox->setWindowTitle(tr("Replace"));
            messageBox->setText(tr("A file named \"%1\" already exists. Do you want to replace it?").arg(fileInfo.fileName()));
            messageBox->setInformativeText(tr("The file already exists in \"%1\".  Replacing it will overwrite its contents.").arg(fileInfo.dir().dirName()));
            messageBox->setStandardButtons(QMessageBox::Cancel);
            messageBox->setDefaultButton(replaceButton);

            messageBox->exec();
            const QAbstractButton *clickedButton = messageBox->clickedButton();
            if (clickedButton == replaceAllButton) {
                replaceAll = true;
            } else if (clickedButton != replaceButton) {
                break;
            }
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
        }

        QFile file {sequencedFilePath};
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            ok = exportAircraft(location, aircraft, file);
            d->exportedFilePaths.push_back(sequencedFilePath);
        }
        file.close();
        ++i;
        if (!ok) {
            break;
        }
    } // All aircraft

    return ok;
}

void LocationExportPluginBase::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void LocationExportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void LocationExportPluginBase::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}

