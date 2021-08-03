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
#include <QString>
#include <QRegExp>

#include "../../Kernel/src/Version.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/File.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftInfo.h"

#include "Export.h"

// PUBLIC

QString Export::suggestFilePath(const QString &suffix) noexcept
{
    QString suggestedFileName;
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Settings &settings = Settings::getInstance();

    const QString &title = flight.getTitle();
    if (title.isNull()) {
        if (flight.count() > 0) {
            const Aircraft &aircraft = flight.getUserAircraft();
            suggestedFileName = aircraft.getAircraftInfoConst().aircraftType.type;
        } else {
            suggestedFileName = Version::getApplicationName();
        }
    } else {
        suggestedFileName = title;
    }

    // https://www.codeproject.com/tips/758861/removing-characters-which-are-not-allowed-in-windo
    QRegExp illegalInFileName = QRegExp("[\\\\/:*?""<>|]");
    suggestedFileName = suggestedFileName.replace(illegalInFileName, "_");
    return settings.getExportPath() + "/" + File::ensureSuffix(suggestedFileName, suffix);
}
