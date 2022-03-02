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
#include <QUrl>
#include <QGuiApplication>
#include <QDesktopServices>

#include "../../Kernel/src/Settings.h"
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
    std::unique_ptr<BasicExportDialog> exportDialog = std::make_unique<BasicExportDialog>(getFileFilter(), getSettings(), getParentWidget());
    connect(exportDialog.get(), &BasicExportDialog::restoreDefaultOptions,
            this, &ExportPluginBase::onRestoreDefaultSettings);
    // Transfer ownership to exportDialog
    exportDialog->setOptionWidget(optionWidget.release());
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString exportDirectoryPath = QFileInfo(exportDialog->getSelectedFilePath()).absolutePath();
        Settings::getInstance().setExportPath(exportDirectoryPath);
        // @todo Ensure file suffix
        const QString filePath = exportDialog->getSelectedFilePath();
        if (!filePath.isEmpty()) {

            QFile file(filePath);
            ok = file.open(QIODevice::WriteOnly);
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
                qDebug("%s import %s in %lld ms", qPrintable(QFileInfo(filePath).fileName()), (ok ? qPrintable("SUCCESS") : qPrintable("FAIL")), timer.elapsed());
#endif
            }

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
            QMessageBox::critical(getParentWidget(), tr("Export error"), tr("The file %1 could not be exported.").arg(filePath));
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
}

void ExportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getSettings().addKeysWithDefaults(keysWithDefaults);
}

void ExportPluginBase::applySettings(Settings::ValuesByKey valuesByKey) noexcept
{
    getSettings().applySettings(valuesByKey);
}
