/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <QObject>

class QUuid;

#include <Kernel/Settings.h>
#include "PluginIntf.h"
#include "PluginManagerLib.h"

class PLUGINMANAGER_API PluginBase : public PluginIntf
{
public:
    PluginBase();
    PluginBase(const PluginBase &rhs) = delete;
    PluginBase(PluginBase &&rhs) = delete;
    PluginBase &operator=(const PluginBase &rhs) = delete;
    PluginBase &operator=(PluginBase &&rhs) = delete;
    ~PluginBase() override;

    void storeSettings(const QUuid &pluginUuid) const noexcept override;
    void restoreSettings(const QUuid &pluginUuid) noexcept override;

protected:
    /*!
     * Adds the plugin-specific settings, a key/value pair for each setting, to \c settings.
     *
     * @param keyValues
     *        the plugin-specific key/value pair settings
     */
    virtual void addSettings(Settings::KeyValues &keyValues) const noexcept;

    /*!
     * Adds the plugin-specific setting keys with corresponding default values to \c keysWithDefaults.
     *
     * @param keysWithDefaults
     *        the plugin-specific keys, with corresponding default values
     */
    virtual void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept;

    /*!
     * Sets the plugin-specific settings.
     *
     * \param valuesByKey
     *        the plugin-specific settings associated with their key
     */
    virtual void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept;
};

#endif // PLUGINBASE_H
