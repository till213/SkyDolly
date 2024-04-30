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

#include <QString>
#include <QStringLiteral>
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
#include <QDesktopServices>

#include <Kernel/File.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/PersistenceManager.h>
#include <Location/BasicLocationExportDialog.h>
#include <Location/LocationExportPluginBaseSettings.h>
#include <Location/LocationExportPluginBase.h>

struct LocationExportPluginBasePrivate
{
    QFile file;
};

// PUBLIC

LocationExportPluginBase::LocationExportPluginBase() noexcept
    : d(std::make_unique<LocationExportPluginBasePrivate>())
{}

LocationExportPluginBase::~LocationExportPluginBase() = default;

bool LocationExportPluginBase::exportLocations(const std::vector<Location> &locations) const noexcept
{
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    LocationExportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<BasicLocationExportDialog> exportDialog = std::make_unique<BasicLocationExportDialog>(getFileExtension(), getFileFilter(), baseSettings, PluginBase::getParentWidget());
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

            if (exportDialog->isFileDialogSelectedFile() || !fileInfo.exists()) {
                ok = exportLocations(locations, filePath);
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
                    ok = exportLocations(locations, filePath);
                } else {
                    ok = true;
                }
            }
        }
    }

    return ok;
}

// PRIVATE

bool LocationExportPluginBase::exportLocations(const std::vector<Location> &locations, const QString &filePath) const noexcept
{
    QFile file(filePath);
    bool ok {true};
#ifdef DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QGuiApplication::processEvents();

    ok = file.open(QIODevice::WriteOnly);
    if (ok) {
        ok = exportLocations(locations, file);
    }
    file.close();

    QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
    qDebug() << QFileInfo(filePath).fileName() << "export" << (ok ? "SUCCESS" : "FAIL") << "in" << timer.elapsed() <<  "ms";
#endif

    if (ok) {
        const LocationExportPluginBaseSettings &settings = getPluginSettings();
        if (settings.isOpenExportedFilesEnabled()) {
            const QString fileUrl = QStringLiteral("file:///") + filePath;
            QDesktopServices::openUrl(QUrl(fileUrl));
        }
    } else {
        QMessageBox::critical(PluginBase::getParentWidget(), tr("Export Error"), tr("An error occured during export into file %1.").arg(QDir::toNativeSeparators(filePath)));
    }

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

void LocationExportPluginBase::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}
