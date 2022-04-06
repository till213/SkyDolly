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

#include "../../../../../Kernel/src/System.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../Plugin/src/ExportPluginBaseSettings.h"
#include "IgcExportSettings.h"

namespace
{
    // Keys
    constexpr char PilotNameKey[] = "PilotName";
    constexpr char CoPilotNameKey[] = "CoPilotName";
}

class IgcExportSettingsPrivate
{
public:
    IgcExportSettingsPrivate()
        : pilotName(DefaultPilotName),
          coPilotName(DefaultCoPilotName)
    {}

    QString pilotName;
    QString coPilotName;

    static inline const QString DefaultPilotName {System::getUsername()};
    static inline const QString DefaultCoPilotName {QString()};
};

// PUBLIC

IgcExportSettings::IgcExportSettings() noexcept
    : ExportPluginBaseSettings(),
      d(std::make_unique<IgcExportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("IgcExportSettings::IgcExportSettings: CREATED");
#endif
}

IgcExportSettings::~IgcExportSettings() noexcept
{
#ifdef DEBUG
    qDebug("IgcExportSettings::~IgcExportSettings: DELETED");
#endif
}

QString IgcExportSettings::getPilotName() const noexcept
{
    return d->pilotName;
}

void IgcExportSettings::setPilotName(const QString &pilotName) noexcept
{
    if (d->pilotName != pilotName) {
        d->pilotName = pilotName;
        emit extendedSettingsChanged();
    }
}

QString IgcExportSettings::getCoPilotName() const noexcept
{
    return d->coPilotName;
}

void IgcExportSettings::setCoPilotName(const QString &coPilotName) noexcept
{
    if (d->coPilotName != coPilotName) {
        d->coPilotName = coPilotName;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void IgcExportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::PilotNameKey;
    keyValue.second = d->pilotName;
    keyValues.push_back(keyValue);

    keyValue.first = ::CoPilotNameKey;
    keyValue.second = d->coPilotName;
    keyValues.push_back(keyValue);
}

void IgcExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::PilotNameKey;
    keyValue.second = IgcExportSettingsPrivate::DefaultPilotName;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::CoPilotNameKey;
    keyValue.second = IgcExportSettingsPrivate::DefaultCoPilotName;
    keysWithDefaults.push_back(keyValue);
}

void IgcExportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->pilotName = valuesByKey.at(::PilotNameKey).value<QString>();
    d->coPilotName = valuesByKey.at(::CoPilotNameKey).value<QString>();

    emit extendedSettingsChanged();
}

void IgcExportSettings::restoreDefaultsExtn() noexcept
{
    d->pilotName = IgcExportSettingsPrivate::DefaultPilotName;
    d->coPilotName = IgcExportSettingsPrivate::DefaultCoPilotName;

    emit extendedSettingsChanged();
}
