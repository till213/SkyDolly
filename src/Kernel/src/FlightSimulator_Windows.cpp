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
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include "FlightSimulator.h"

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1
static bool isProcessRunning(DWORD pid, const QString &processName)
{
    TCHAR actualProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process
    HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    // Get the process name
    if (processHandle != nullptr) {
        HMODULE moduleHandle;
        DWORD actualSize;
        if (EnumProcessModules(processHandle, &moduleHandle, sizeof(moduleHandle), &actualSize)) {
            GetModuleBaseName(processHandle, moduleHandle, actualProcessName, sizeof(actualProcessName) / sizeof(TCHAR));
        }
    }

#ifdef UNICODE
    QString actualName = QString::fromUtf16(actualProcessName);
#else
    QString actualName = QString::fromLocal8Bit(actualProcessName);
#endif

    // Release the handle to the process
    CloseHandle(processHandle);

    return actualName == processName;
}

// PUBLIC

bool FlightSimulator::isRunning(Id id) noexcept
{
    bool running;

    QString processName;
    switch (id) {
    case Id::FS2020:
        processName = "FlightSimulator.exe";
        break;
    case Id::Prepar3Dv5:
        processName = "Prepar3D.exe";
        break;
    default:
        processName = "";
        break;
    }

    if (!processName.isEmpty()) {
        // https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes
        // Get the list of process identifiers
        DWORD pidTable[1024], actualSize, nofPIds;
        unsigned int i;
        if (EnumProcesses(pidTable, sizeof(pidTable), &actualSize)) {
            // Calculate how many process identifiers were returned
            nofPIds = actualSize / sizeof(DWORD);

            // Check if process given by its name is running
            running = false;
            for (i = 0; i < nofPIds && !running; ++i) {
                if( pidTable[i] != 0) {
                    running = isProcessRunning(pidTable[i], processName);
                }
            }
        } else {
            running = false;
        }
    } else {
        running = false;
    }
    return running;
}

bool FlightSimulator::isInstalled(Id id) noexcept
{
    bool installed;
    // Search the community folder: if found then we assume that FS2020 is installed, too
    QString appDataPath = QString::fromLocal8Bit(qgetenv("APPDATA"));

    // MS Store edition
    QDir communityFolderPath(appDataPath + "/Local/Packages/Microsoft.FlightSimulator_8wekyb3d8bbwe");
    installed = communityFolderPath.exists();

    if (!installed) {
        // Steam edition
        communityFolderPath.setPath(appDataPath + "/Microsoft Flight Simulator");
        installed = communityFolderPath.exists();
    }
    return installed;
}
