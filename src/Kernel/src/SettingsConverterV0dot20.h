/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SETTINGSCONVERTERV0DOT20_H
#define SETTINGSCONVERTERV0DOT20_H

#include "SettingsConverterV0dot18.h"

class SettingsConverterV0dot20
{
public:
    static inline void convert(const Version &settingsVersion, QSettings &settings) noexcept
    {
        if (settingsVersion < Version("0.18.0")) {
            SettingsConverterV0dot18::convert(settingsVersion, settings);
        }
        convertPlugins(settings);
    }

private:
    static inline void convertPlugins(QSettings &settings) noexcept
    {
        static constexpr const char *ExportSystemLocationsEnabledKey {"ExportSystemLocationsEnabled"};

        // CSV location export
        settings.beginGroup("Plugins/");
        {
            settings.beginGroup(QUuid(Const::CsvLocationExportPluginUuid).toByteArray());
            {
                const bool enabled = settings.value(ExportSystemLocationsEnabledKey).toBool();
                settings.setValue("ExportPresetLocationsEnabledKey", enabled);
                settings.remove(ExportSystemLocationsEnabledKey);
            }
            settings.endGroup();

            settings.beginGroup(QStringLiteral("Modules/") % QUuid(Const::LocationModuleUuid).toByteArray());
            {
                settings.setValue("LocationTableState", QVariant());
            }
            settings.endGroup();
        }
        settings.endGroup();
    }
};

#endif // SETTINGSCONVERTERV0DOT20_H
