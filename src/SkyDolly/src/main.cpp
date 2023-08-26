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
#include <csignal>

#include <QCoreApplication>
#include <QApplication>
#include <QStringList>
#include <QString>
#include <QStyleFactory>
#include <QStringBuilder>
#include <QMessageBox>

#include <Kernel/Version.h>
#include <Kernel/StackTrace.h>
#include <Kernel/Settings.h>
#include <Kernel/RecentFile.h>
#include <Model/Logbook.h>
#include <PluginManager/SkyConnectManager.h>
#include <Persistence/PersistenceManager.h>
#include <PluginManager/PluginManager.h>
#include <UserInterface/MainWindow.h>
#include "ExceptionHandler.h"

static void destroySingletons() noexcept
{
    // Destroying the settings singleton also persists the settings
    Settings::destroyInstance();
    Logbook::destroyInstance();
    PersistenceManager::destroyInstance();
    PluginManager::destroyInstance();
    SkyConnectManager::destroyInstance();
    RecentFile::destroyInstance();
}

int main(int argc, char **argv) noexcept
{
    qDebug() << StackTrace::generate();
    std::set_terminate(ExceptionHandler::handleTerminate);
    std::signal(SIGSEGV, ExceptionHandler::signalHandler);

    static const int ErrorCode = -1;
    QCoreApplication::setOrganizationName(Version::getOrganisationName());
    QCoreApplication::setApplicationName(Version::getApplicationName());
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);

    QApplication application(argc, argv);

    // Simplistic command line parsing: first arg is assumed to be a file path
    QStringList args = application.arguments();
    QString filePath;
    if (args.count() > 1) {
        filePath = args.at(1);
    }

    int res {0};
    try {
        // Main window scope
        {
            std::unique_ptr<MainWindow> mainWindow = std::make_unique<MainWindow>(filePath);
            mainWindow->show();
            res = application.exec();
        }
        if (ExceptionHandler::getSignal() == SIGSEGV) {
            QMessageBox::critical(nullptr, "Segmentation fault", "A segmentation fault occurred during applicaiton execution.");
        }
        // Destroy singletons after main window has been deleted
        destroySingletons();
    } catch (const std::exception &ex) {
        //const QString stackTrace = StackTrace::generate();
        const QString stackTrace;
        ExceptionHandler::handle("Exception", stackTrace, ex);
        res = ErrorCode;
    } catch (...) {
        //const QString stackTrace = StackTrace::generate();
        const QString stackTrace;
        ExceptionHandler::handle("Exception", stackTrace, "Non std::exception");
        res = ErrorCode;
    }

    return res;
}
