/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef PLUGINWITHOPTIONWIDGETINTF_H
#define PLUGINWITHOPTIONWIDGETINTF_H

#include <memory>
#include <optional>

#include "OptionWidgetIntf.h"

/*!
 * This is the interface for plugins that provide an option widget with plugin-specific options.
 *
 * This option widget is typically shown in the application settings dialog.
 */
class PLUGINMANAGER_API PluginWithOptionWidgetIntf
{
public:
    PluginWithOptionWidgetIntf() = default;
    PluginWithOptionWidgetIntf(const PluginWithOptionWidgetIntf &rhs) = default;
    PluginWithOptionWidgetIntf(PluginWithOptionWidgetIntf &&rhs) = default;
    PluginWithOptionWidgetIntf &operator=(const PluginWithOptionWidgetIntf &rhs) = default;
    PluginWithOptionWidgetIntf &operator=(PluginWithOptionWidgetIntf &&rhs) = default;
    virtual ~PluginWithOptionWidgetIntf() = default;

    virtual std::optional<std::unique_ptr<OptionWidgetIntf>> createOptionWidget() const noexcept = 0;
};

#endif // PLUGINWITHOPTIONWIDGETINTF_H
