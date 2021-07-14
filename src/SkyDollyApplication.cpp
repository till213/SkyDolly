/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QApplication>
#include <QWidgetList>
#include <QWidget>

#include "../../Kernel/src/Settings.h"
#include "../../Model/src/Logbook.h"
#include "../../SkyConnect/src/SkyConnectManager.h"
#include "../../Persistence/src/ConnectionManager.h"
#include "../../Plugin/src/PluginManager.h"
#include "SkyDollyApplication.h"

// PUBLIC

SkyDollyApplication::SkyDollyApplication(int &argc, char **argv) noexcept
    : QApplication(argc, argv)
{
    frenchConnection();
}

// PRIVATE

void SkyDollyApplication::frenchConnection() noexcept
{
    connect(this, &QApplication::aboutToQuit,
            this, &SkyDollyApplication::handleAboutToQuit);
}

// PRIVATE SLOTS

void SkyDollyApplication::handleAboutToQuit() noexcept
{
    // Some widgets try to disconnect from the below singleton instances upon
    // a "hide" event, so make sure that all windows and dialogs are closed first
    QWidgetList widgetList = topLevelWidgets();
    for (QWidget *widget : widgetList) {
        if (widget->inherits("QMainWindow") || widget->inherits("QDialog")) {
            widget->close();
        }
    }

    // Destroying the settings singleton also persists the settings
    Settings::destroyInstance();
    SkyConnectManager::destroyInstance();
    Logbook::destroyInstance();
    ConnectionManager::destroyInstance();
    PluginManager::destroyInstance();
}
