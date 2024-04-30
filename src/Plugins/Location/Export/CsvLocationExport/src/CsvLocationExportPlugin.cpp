/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation ios (the "Software"), to deal in the Software
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
#include <QWidget>
#include <QFile>
#include <QString>
#include <QStringLiteral>
// Implements the % operator for string concatenation
#include <QStringBuilder>

#include <Model/Location.h>
#include <Model/Aircraft.h>
#include "CsvLocationExportSettings.h"
#include "CsvLocationExportOptionWidget.h"
#include "CsvLocationWriterIntf.h"
#include "SkyDollyCsvLocationWriter.h"
#include "LittleNavmapCsvLocationWriter.h"
#include "CsvLocationExportPlugin.h"

struct CsvLocationExportPluginPrivate
{
    CsvLocationExportSettings pluginSettings;
    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

CsvLocationExportPlugin::CsvLocationExportPlugin() noexcept
    : d(std::make_unique<CsvLocationExportPluginPrivate>())
{}

CsvLocationExportPlugin::~CsvLocationExportPlugin() = default;

// PROTECTED

LocationExportPluginBaseSettings &CsvLocationExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString CsvLocationExportPlugin::getFileExtension() const noexcept
{
    return CsvLocationExportPluginPrivate::FileExtension;
}

QString CsvLocationExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> CsvLocationExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CsvLocationExportOptionWidget>(d->pluginSettings);
}

bool CsvLocationExportPlugin::exportLocations(const std::vector<Location> &locations, QIODevice &io) const noexcept
{
    std::unique_ptr<CsvLocationWriterIntf> writer;
    switch (d->pluginSettings.getFormat()) {
    case CsvLocationExportSettings::Format::SkyDolly:
        writer = std::make_unique<SkyDollyCsvLocationWriter>(d->pluginSettings);
        break;
    case CsvLocationExportSettings::Format::LittleNavmap:
        writer = std::make_unique<LittleNavmapCsvLocationWriter>(d->pluginSettings);
        break;
    }

    bool ok {false};
    if (writer != nullptr) {
        io.setTextModeEnabled(true);
        ok = writer->write(locations, io);
    }

    return ok;
}
