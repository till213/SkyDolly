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
#include <tuple>
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

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/File.h"
#include "BasicExportDialog.h"
#include "ExportPluginBaseSettings.h"
#include "ExportPluginBase.h"

class ExportPluginBasePrivate
{
public:
    ExportPluginBasePrivate()
    {}
};

// PUBLIC

ExportPluginBase::ExportPluginBase() noexcept
    : d(std::make_unique<ExportPluginBasePrivate>())
{
#ifdef DEBUG
    qDebug("ExportPluginBase::ExportPluginBase: CREATED");
#endif
}

ExportPluginBase::~ExportPluginBase() noexcept
{
#ifdef DEBUG
    qDebug("ExportPluginBase::~ExportPluginBase: DELETED");
#endif
}

bool ExportPluginBase::exportData() noexcept
{
    bool ok;
    Settings &settings = Settings::getInstance();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicExportDialog> exportDialog = std::make_unique<BasicExportDialog>(getFileExtension(), getFileFilter(), getSettings(), getParentWidget());
    connect(exportDialog.get(), &BasicExportDialog::restoreDefaultOptions,
            this, &ExportPluginBase::onRestoreDefaultSettings);
    // Transfer ownership to exportDialog
    exportDialog->setOptionWidget(optionWidget.release());
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString selectedFilePath = exportDialog->getSelectedFilePath();
        if (!selectedFilePath.isEmpty()) {
            const QString filePath = File::ensureSuffix(selectedFilePath, getFileExtension());
            const QFileInfo fileInfo {filePath};
            const QString exportDirectoryPath = fileInfo.absolutePath();
            Settings::getInstance().setExportPath(exportDirectoryPath);

            if (getSettings().isFileDialogSelectedFile() || !fileInfo.exists()) {
                ok = exportFile(filePath);
            } else {
                QMessageBox messageBox;
                messageBox.setIcon(QMessageBox::Question);
                QPushButton *replaceButton = messageBox.addButton(tr("&Replace"), QMessageBox::AcceptRole);
                messageBox.setText(tr("A file named \"%1\" already exists. Do you want to replace it?").arg(fileInfo.fileName()));
                messageBox.setInformativeText(tr("The file already exists in \"%1\".  Replacing it will overwrite its contents.").arg(fileInfo.dir().dirName()));
                messageBox.setStandardButtons(QMessageBox::Cancel);
                messageBox.setDefaultButton(replaceButton);

                messageBox.exec();
                const QAbstractButton *clickedButton = messageBox.clickedButton();
                if (clickedButton == replaceButton) {
                    ok = exportFile(filePath);
                } else {
                    ok = true;
                }
            }
        } else {
            ok = true;
        }
    } else {
        ok = true;
    }

    return ok;
}

// PROTECTED

void ExportPluginBase::addSettings(Settings::PluginSettings &settings) const noexcept
{
    getSettings().addSettings(settings);
    addSettingsExtn(settings);
}

void ExportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getSettings().addKeysWithDefaults(keysWithDefaults);
    addKeysWithDefaultsExtn(keysWithDefaults);
}

void ExportPluginBase::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    getSettings().restoreSettings(valuesByKey);
    restoreSettingsExtn(valuesByKey);
}

// PRIVATE SLOTS

bool ExportPluginBase::exportFile(const QString &filePath) noexcept
{
    QFile file(filePath);
    bool ok = file.open(QIODevice::WriteOnly);
    if (ok) {
#ifdef DEBUG
        QElapsedTimer timer;
        timer.start();
#endif
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        QGuiApplication::processEvents();
        ok = writeFile(file);
        QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
        qDebug("%s export %s in %lld ms", qPrintable(QFileInfo(filePath).fileName()), (ok ? qPrintable("SUCCESS") : qPrintable("FAIL")), timer.elapsed());
#endif
    }

    file.close();

    if (ok) {
        if (getSettings().isOpenExportedFileEnabled()) {
            const QString fileUrl = QString("file:///") + filePath;
            QDesktopServices::openUrl(QUrl(fileUrl));
        }
    } else {
        QMessageBox::critical(getParentWidget(), tr("Export error"), tr("An error occured during export into file %1.").arg(filePath));
    }

    return ok;
}
