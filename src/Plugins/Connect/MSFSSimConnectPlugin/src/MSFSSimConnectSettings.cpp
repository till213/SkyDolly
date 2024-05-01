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

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Connect/ConnectPluginBaseSettings.h>
#include "MSFSSimConnectSettings.h"

namespace
{
    // Keys
    constexpr const char *ConnectionTypeKey {"ConnectionType"};

    // Defaults
    constexpr auto DefaultConnectionType {MSFSSimConnectSettings::ConnectionType::Pipe};
}

struct MSFSSimConnectSettingsPrivate
{
    MSFSSimConnectSettings::ConnectionType connectionType {::DefaultConnectionType};
};

// PUBLIC

MSFSSimConnectSettings::MSFSSimConnectSettings() noexcept
    : ConnectPluginBaseSettings(),
    d(std::make_unique<MSFSSimConnectSettingsPrivate>())
{}

MSFSSimConnectSettings::~MSFSSimConnectSettings() = default;

MSFSSimConnectSettings::ConnectionType MSFSSimConnectSettings::getConnectionType() const noexcept
{
    return d->connectionType;
}

void MSFSSimConnectSettings::setConnectionType(ConnectionType connectionType) noexcept
{
    if (d->connectionType != connectionType) {
        d->connectionType = connectionType;
        emit changed(Connect::Mode::Reconnect);
    }
}

// PROTECTED

void MSFSSimConnectSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::ConnectionTypeKey);
    keyValue.second = Enum::underly(d->connectionType);
    keyValues.push_back(keyValue);
}

void MSFSSimConnectSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::ConnectionTypeKey);
    keyValue.second = Enum::underly(::DefaultConnectionType);
    keysWithDefaults.push_back(keyValue);
}

void MSFSSimConnectSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {false};
    int connectionTypeValue = valuesByKey.at(QString::fromLatin1(::ConnectionTypeKey)).toInt(&ok);
    if (ok) {
        d->connectionType = static_cast<ConnectionType>(connectionTypeValue);
    } else {
#ifdef DEBUG
        qWarning() << "The connectionType in the settings could not be parsed, so setting value to default value:" << Enum::underly(::DefaultConnectionType);
#endif
        d->connectionType  = ::DefaultConnectionType;
    }
}

void MSFSSimConnectSettings::restoreDefaultsExtn() noexcept
{
    d->connectionType = ::DefaultConnectionType;
}
