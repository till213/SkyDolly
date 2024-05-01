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
#ifndef CONNECTPLUGINBASESETTINGS_H
#define CONNECTPLUGINBASESETTINGS_H

#include <memory>

#include <QObject>
#include <QString>

#include <Kernel/Settings.h>
#include <Kernel/FlightSimulatorShortcuts.h>
#include "Connect.h"
#include "../PluginManagerLib.h"

struct ConnectPluginBaseSettingsPrivate;

class PLUGINMANAGER_API ConnectPluginBaseSettings : public QObject
{
    Q_OBJECT
public:
    ConnectPluginBaseSettings() noexcept;
    ConnectPluginBaseSettings(const ConnectPluginBaseSettings &rhs) = delete;
    ConnectPluginBaseSettings(ConnectPluginBaseSettings &&rhs) = delete;
    ConnectPluginBaseSettings &operator=(const ConnectPluginBaseSettings &rhs) = delete;
    ConnectPluginBaseSettings &operator=(ConnectPluginBaseSettings &&rhs) = delete;
    ~ConnectPluginBaseSettings() override;

    /*!
     * Returns the flight simulator shortcuts that can be triggered in the connected flight simulator.
     *
     * \return the flight simulator shortcuts
     */
    FlightSimulatorShortcuts getFlightSimulatorShortcuts() const noexcept;

    /*!
     * Sets the flight simulator shortcuts that can be triggered in the connected flight simulator.
     *
     * \param shortcuts
     *        the client event shortcuts
     * \sa flightSimulatorShortcutsChanged
     */
    void setFlightSimulatorShortcuts(FlightSimulatorShortcuts shortcuts) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefault) const noexcept;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept;
    void restoreDefaults() noexcept;

signals:
    /*!
     * Emitted whenever the plugin settings (base settings or extended settings) have changed.
     *
     * \param mode
     *        indicates whether a reconnect is required or not
     */
    void changed(Connect::Mode mode);

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept = 0;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept = 0;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept = 0;
    virtual void restoreDefaultsExtn() noexcept = 0;

private:
    const std::unique_ptr<ConnectPluginBaseSettingsPrivate> d;
};

#endif // CONNECTPLUGINBASESETTINGS_H
