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
#include <exception>

#include <QCoreApplication>
#include <QSysInfo>
#include <QApplication>
#include <QStringList>
#include <QString>
#include <QStyleFactory>
#include <QStringBuilder>
#include <QMessageBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Version.h>
#include <Kernel/StackTrace.h>
#include <Kernel/Settings.h>
#include <Kernel/System.h>
#include <Kernel/RecentFile.h>
#include <Model/Logbook.h>
#include <PluginManager/SkyConnectManager.h>
#include <Persistence/PersistenceManager.h>
#include <PluginManager/PluginManager.h>
#include <UserInterface/MainWindow.h>
#include "ExceptionHandler.h"
#include "SignalHandler.h"
#include "ErrorCodes.h"

static void destroySingletons() noexcept
{
    Logbook::destroyInstance();
    PersistenceManager::destroyInstance();
    PluginManager::destroyInstance();
    SkyConnectManager::destroyInstance();
    RecentFile::destroyInstance();

    // Destroying the settings singleton also persists the settings; destroy this instance
    // last, as previous plugin managers such as the SkyConnectManager may still want
    // to store their plugin settings
    Settings::destroyInstance();
}

// Refer e.g. to https://bugreports.qt.io/browse/QTBUG-124286 (fix expected latest in Qt 6.8)
[[deprecated("Do not use once the new Windows 11 style is ready for prime time.")]]
static void applyWindows11DefaultStyleWorkaround() noexcept
{
    QApplication::setStyle("Fusion");
}

int main(int argc, char **argv) noexcept
{
    std::set_terminate(ExceptionHandler::onTerminate);

    QCoreApplication::setOrganizationName(Version::getOrganisationName());
    QCoreApplication::setApplicationName(Version::getApplicationName());
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);

    QApplication application(argc, argv);

    // Set the user interface style (if not default)
    // Implementation note: must be set AFTER QApplication instantiation
    const QString styleKey = Settings::getInstance().getStyleKey();
    if (styleKey != Settings::DefaultStyleKey) {
        QApplication::setStyle(styleKey);
    } else if (System::isWindows11()) {
        applyWindows11DefaultStyleWorkaround();
    }

    // Signals must be registered after the QApplication instantiation, due
    // to the QSocketNotifier
    SignalHandler signalHandler;
    signalHandler.registerSignals();

    // Simplistic command line parsing: first arg is assumed to be a file path
    QStringList args = application.arguments();
    QString filePath;
    if (args.count() > 1) {
        filePath = args.at(1);
    }

    int res {ErrorCodes::Ok};
    try {
        // Main window scope
        {
            std::unique_ptr<MainWindow> mainWindow = std::make_unique<MainWindow>(filePath);
            mainWindow->show();
            res = application.exec();
        }
        // Destroy singletons after main window has been deleted
        destroySingletons();
    } catch (const std::exception &ex) {
        const QString stackTrace = StackTrace::generate();
        ExceptionHandler::onError("Exception", stackTrace, ex);
        res = ErrorCodes::StandardException;
    } catch (...) {
        const QString stackTrace = StackTrace::generate();
        ExceptionHandler::onError("Exception", stackTrace, "Non std::exception");
        res = ErrorCodes::UnknownException;
    }

    return res;
}
