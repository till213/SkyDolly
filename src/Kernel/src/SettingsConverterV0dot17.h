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
#ifndef SETTINGSCONVERTERV0DOT17_H
#define SETTINGSCONVERTERV0DOT17_H

#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QUuid>

#include <Const.h>
#include <Version.h>

#include "SettingsConverterV0dot16.h"

class SettingsConverterV0dot17
{
public:
    static inline void convert(const Version &settingsVersion, QSettings &settings) noexcept
    {
        if (settingsVersion < Version(QString("0.16.0"))) {
            SettingsConverterV0dot16::convert(settingsVersion, settings);
        }
        convertPluginsV0dot17(settings);
    }

private:
    static inline void convertPluginsV0dot17(QSettings &settings) noexcept
    {
        static constexpr const char *FormatKey = "Format";
        int format {0};

        // CSV export
        settings.beginGroup(QString("Plugins/") + QUuid(Const::CsvExportPluginUuid).toByteArray());
        {
            bool ok {true};
            format = settings.value(FormatKey).toInt(&ok);
            if (ok) {
                if (format == 0) {
                    // Switch "Sky Dolly" to "Position and attitude" CSV format
                    settings.setValue(FormatKey, 2);
                }
            } else {
                // Switch "Sky Dolly" to "Position and attitude" CSV format
                settings.setValue(FormatKey, 2);
            }
        }
        settings.endGroup();
    }
};

#endif // SETTINGSCONVERTERV0DOT17_H
