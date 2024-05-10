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
#ifndef LOCATIONEXPORTPLUGINBASESETTINGS_H
#define LOCATIONEXPORTPLUGINBASESETTINGS_H

#include <memory>

#include <QObject>
#include <QString>

#include <Kernel/Settings.h>
#include "../PluginManagerLib.h"

struct LocationExportPluginBaseSettingsPrivate;

class PLUGINMANAGER_API LocationExportPluginBaseSettings : public QObject
{
    Q_OBJECT
public:
    LocationExportPluginBaseSettings() noexcept;
    LocationExportPluginBaseSettings(const LocationExportPluginBaseSettings &rhs) = delete;
    LocationExportPluginBaseSettings(LocationExportPluginBaseSettings &&rhs) = delete;
    LocationExportPluginBaseSettings &operator=(const LocationExportPluginBaseSettings &rhs) = delete;
    LocationExportPluginBaseSettings &operator=(LocationExportPluginBaseSettings &&rhs) = delete;
    ~LocationExportPluginBaseSettings() override;

    bool isExportSystemLocationsEnabled() const noexcept;
    void setExportSystemLocationsEnabled(bool enabled) noexcept;

    bool isOpenExportedFilesEnabled() const noexcept;
    void setOpenExportedFilesEnabled(bool enabled) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefault) const noexcept;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept;
    void restoreDefaults() noexcept;

signals:
    /*!
     * Emitted whenever the plugin settings (base settings or extended settings) have changed.
     */
    void changed();

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept = 0;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept = 0;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept = 0;
    virtual void restoreDefaultsExtn() noexcept = 0;

private:
    const std::unique_ptr<LocationExportPluginBaseSettingsPrivate> d;
};

#endif // LOCATIONEXPORTPLUGINBASESETTINGS_H
