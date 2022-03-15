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

#include <QString>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/System.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimType.h"
#include "../../Plugin/src/ExportPluginBaseSettings.h"
#include "IGCExportSettings.h"

namespace
{
    // Keys
    constexpr char PilotNameKey[] = "PilotName";
    constexpr char CoPilotNameKey[] = "CoPilotName";
}

class IGCExportSettingsPrivate
{
public:
    IGCExportSettingsPrivate()
        : pilotName(DefaultPilotName),
          coPilotName(DefaultCoPilotName)
    {}

    QString pilotName;
    QString coPilotName;

    static inline const QString DefaultPilotName {System::getUsername()};
    static inline const QString DefaultCoPilotName {QString()};
};

// PUBLIC

IGCExportSettings::IGCExportSettings() noexcept
    : ExportPluginBaseSettings(),
      d(std::make_unique<IGCExportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("IGCExportSettings::IGCExportSettings: CREATED");
#endif
}

IGCExportSettings::~IGCExportSettings() noexcept
{
#ifdef DEBUG
    qDebug("IGCExportSettings::~IGCExportSettings: DELETED");
#endif
}

QString IGCExportSettings::getPilotName() const noexcept
{
    return d->pilotName;
}

void IGCExportSettings::setPilotName(const QString &pilotName) noexcept
{
    if (d->pilotName != pilotName) {
        d->pilotName = pilotName;
        emit extendedSettingsChanged();
    }
}

QString IGCExportSettings::getCoPilotName() const noexcept
{
    return d->coPilotName;
}

void IGCExportSettings::setCoPilotName(const QString &coPilotName) noexcept
{
    if (d->coPilotName != coPilotName) {
        d->coPilotName = coPilotName;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void IGCExportSettings::addSettingsExtn(Settings::PluginSettings &settings) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::PilotNameKey;
    keyValue.second = d->pilotName;
    settings.push_back(keyValue);

    keyValue.first = ::CoPilotNameKey;
    keyValue.second = d->coPilotName;
    settings.push_back(keyValue);
}

void IGCExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::PilotNameKey;
    keyValue.second = IGCExportSettingsPrivate::DefaultPilotName;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::CoPilotNameKey;
    keyValue.second = IGCExportSettingsPrivate::DefaultCoPilotName;
    keysWithDefaults.push_back(keyValue);
}

void IGCExportSettings::restoreSettingsExtn(Settings::ValuesByKey valuesByKey) noexcept
{
    d->pilotName = valuesByKey[::PilotNameKey].value<QString>();
    d->coPilotName = valuesByKey[::CoPilotNameKey].value<QString>();

    emit extendedSettingsChanged();
}

void IGCExportSettings::restoreDefaultsExtn() noexcept
{
    d->pilotName = IGCExportSettingsPrivate::DefaultPilotName;
    d->coPilotName = IGCExportSettingsPrivate::DefaultCoPilotName;

    emit extendedSettingsChanged();
}
