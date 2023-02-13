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
#include <vector>

#include <QWidget>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QCursor>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
#include <QUrl>
#include <QGuiApplication>
#include <QDesktopServices>

#include <Kernel/Settings.h>
#include <Kernel/File.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include "BasicFlightExportDialog.h"
#include "FlightExportPluginBaseSettings.h"
#include "FlightExportPluginBase.h"

struct FlightExportPluginBasePrivate
{ 
    std::vector<QString> exportedFilePaths;
};

// PUBLIC

FlightExportPluginBase::FlightExportPluginBase() noexcept
    : d(std::make_unique<FlightExportPluginBasePrivate>())
{}

FlightExportPluginBase::~FlightExportPluginBase() = default;

bool FlightExportPluginBase::exportFlight(const Flight &flight) const noexcept
{
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    FlightExportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<BasicFlightExportDialog> exportDialog = std::make_unique<BasicFlightExportDialog>(flight, getFileExtension(), getFileFilter(), baseSettings, PluginBase::getParentWidget());
    // Transfer ownership to exportDialog
    exportDialog->setOptionWidget(optionWidget.release());
    bool ok {true};
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString selectedFilePath = exportDialog->getSelectedFilePath();
        if (!selectedFilePath.isEmpty()) {
            const QString filePath = File::ensureExtension(selectedFilePath, getFileExtension());
            const QFileInfo fileInfo {filePath};
            const QString exportDirectoryPath = fileInfo.absolutePath();
            Settings::getInstance().setExportPath(exportDirectoryPath);

            const FlightExportPluginBaseSettings::FormationExport formationExport = getPluginSettings().getFormationExport();
            if (formationExport == FlightExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles || exportDialog->isFileDialogSelectedFile() || !fileInfo.exists()) {
                ok = exportFlight(flight, filePath);
            } else {
                std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(PluginBase::getParentWidget());
                messageBox->setIcon(QMessageBox::Question);
                QPushButton *replaceButton = messageBox->addButton(tr("&Replace"), QMessageBox::AcceptRole);
                messageBox->setWindowTitle(tr("Replace"));
                messageBox->setText(tr("A file named \"%1\" already exists. Do you want to replace it?").arg(fileInfo.fileName()));
                messageBox->setInformativeText(tr("The file already exists in \"%1\".  Replacing it will overwrite its contents.").arg(fileInfo.dir().dirName()));
                messageBox->setStandardButtons(QMessageBox::Cancel);
                messageBox->setDefaultButton(replaceButton);

                messageBox->exec();
                const QAbstractButton *clickedButton = messageBox->clickedButton();
                if (clickedButton == replaceButton) {
                    ok = exportFlight(flight, filePath);
                } else {
                    ok = true;
                }
            }
        } else {
            ok = true;
        }
    }

    return ok;
}

// PRIVATE

bool FlightExportPluginBase::exportFlight(const Flight &flight, const QString &filePath) const noexcept
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
    const FlightExportPluginBaseSettings &settings = getPluginSettings();
    switch (settings.getFormationExport()) {
    case FlightExportPluginBaseSettings::FormationExport::UserAircraftOnly:
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            ok = exportAircraft(flight.getFlightData(), flight.getUserAircraft(), file);
            d->exportedFilePaths.push_back(filePath);
        }
        file.close();
        break;
    case FlightExportPluginBaseSettings::FormationExport::AllAircraftOneFile:
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            ok = exportFlightData(flight.getFlightData(), file);
            d->exportedFilePaths.push_back(filePath);
        }
        file.close();
        break;
    case FlightExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles:
        ok = exportAllAircraft(flight, filePath);
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

bool FlightExportPluginBase::exportAllAircraft(const Flight &flight, const QString &filePath) const noexcept
{
    bool ok {true};
    bool replaceAll {false};
    int i {1};
    for (const auto &aircraft : flight) {
        // Don't append sequence numbers if flight has only one aircraft
        const QString sequencedFilePath = flight.count() > 1 ? File::getSequenceFilePath(filePath, i) : filePath;
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
            ok = exportAircraft(flight.getFlightData(), aircraft, file);
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

void FlightExportPluginBase::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void FlightExportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void FlightExportPluginBase::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}
