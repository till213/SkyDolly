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

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringLiteral>
#include <QStringConverter>
#include <QWidget>

#include <Kernel/Const.h>
#include <Model/Location.h>
#include "SdLogLocationImportSettings.h"
#include "SdLogLocationImportPlugin.h"

struct SdLogLocationImportPluginPrivate
{
    SdLogLocationImportSettings pluginSettings;
    static inline const QString FileExtension {Const::LogbookExtension};
};

// PUBLIC

SdLogLocationImportPlugin::SdLogLocationImportPlugin() noexcept
    : d(std::make_unique<SdLogLocationImportPluginPrivate>())
{}

SdLogLocationImportPlugin::~SdLogLocationImportPlugin() = default;

// PROTECTED

LocationImportPluginBaseSettings &SdLogLocationImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString SdLogLocationImportPlugin::getFileExtension() const noexcept
{
    return SdLogLocationImportPluginPrivate::FileExtension;
}

QString SdLogLocationImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> SdLogLocationImportPlugin::createOptionWidget() const noexcept
{
    return nullptr;
}

std::vector<Location> SdLogLocationImportPlugin::importLocations(QFile &file, bool *ok) noexcept
{
    std::vector<Location> locations;
    std::unique_ptr<LocationService> locationService;

    // TODO IMPLEMENT ME

    return locations;
}
