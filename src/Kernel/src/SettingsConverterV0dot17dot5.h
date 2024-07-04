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
#ifndef SETTINGSCONVERTERV0DOT17DOT5_H
#define SETTINGSCONVERTERV0DOT17DOT5_H

#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QSettings>
#include <QByteArray>
#include <QUuid>

#include <Const.h>
#include <Version.h>

#include "SettingsConverterV0dot17.h"

class SettingsConverterV0dot17dot5
{
public:
    static inline void convert(const Version &settingsVersion, QSettings &settings) noexcept
    {
        if (settingsVersion < Version(QStringLiteral("0.17.0"))) {
            SettingsConverterV0dot17::convert(settingsVersion, settings);
        }
        // The regression ("off by one" connection type) has been introduced with v0.17.1
        if (settingsVersion >= Version(QStringLiteral("0.17.1"))) {
            convertMsfsSimConnectPlugin(settings);
        }
    }

private:
    static inline void convertMsfsSimConnectPlugin(QSettings &settings) noexcept
    {
        static const QString ConnectionTypeKey {"ConnectionType"};
        int connectionType {0};

        // MSFS SimConnect
        settings.beginGroup(QStringLiteral("Plugins/") % QUuid(Const::MsfsSimConnectPluginUuid).toByteArray());
        {
            bool ok {true};
            connectionType = settings.value(ConnectionTypeKey).toInt(&ok);
            if (ok) {
                // Fix existing "off by one" value
                const int newConnectionType = connectionType > 0 ? connectionType - 1 : 0;
                settings.setValue(ConnectionTypeKey, newConnectionType);
            }
        }
        settings.endGroup();
    }
};

#endif // SETTINGSCONVERTERV0DOT17DOT5_H
