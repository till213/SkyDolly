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
#ifndef PLUGININTF_H
#define PLUGININTF_H

class QUuid;

#include "PluginManagerLib.h"

/*!
 * This is the interface for plugins that support plugin-specific settings.
 */
class PLUGINMANAGER_API PluginIntf
{
public:
    PluginIntf() = default;
    PluginIntf(const PluginIntf &rhs) = default;
    PluginIntf(PluginIntf &&rhs) = default;
    PluginIntf &operator=(const PluginIntf &rhs) = default;
    PluginIntf &operator=(PluginIntf &&rhs) = default;
    virtual ~PluginIntf() = default;

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept = 0;
    virtual void restoreSettings(const QUuid &pluginUuid) noexcept = 0;
};

#endif // PLUGININTF_H
