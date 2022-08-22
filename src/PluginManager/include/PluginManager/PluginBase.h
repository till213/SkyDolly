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
#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <memory>

#include <QObject>
#include <QUuid>

#include <Kernel/Settings.h>
#include "PluginIntf.h"
#include "PluginManagerLib.h"

struct PluginBasePrivate;

class PLUGINMANAGER_API PluginBase : public QObject, public PluginIntf
{
    Q_OBJECT
public:
    PluginBase();
    virtual ~PluginBase();

    virtual QWidget *getParentWidget() const noexcept override;
    virtual void setParentWidget(QWidget *parent) noexcept override;

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override;
    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override;

protected:
    /*!
     * Adds the plugin-specific settings, a key/value pair for each setting, to \c settings.
     *
     * @param settings
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
    virtual void restoreSettings(Settings::ValuesByKey valuesByKey) noexcept;

private:
    const std::unique_ptr<PluginBasePrivate> d;
};

#endif // PLUGINBASE_H
