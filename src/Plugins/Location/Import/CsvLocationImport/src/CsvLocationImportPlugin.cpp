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
#include <memory.h>

#include <QFile>
#include <QTextStream>
#include <QTextCodec>

#include <Model/Location.h>
#include "CsvLocationImportOptionWidget.h"
#include "CsvLocationImportSettings.h"
#include "CsvLocationParserIntf.h"
#include "LittleNavmapCsvParser.h"
#include "CsvLocationImportPlugin.h"

struct CsvLocationImportPluginPrivate
{
    CsvLocationImportSettings pluginSettings;
    static constexpr const char *FileSuffix {"csv"};
};

// PUBLIC

CsvLocationImportPlugin::CsvLocationImportPlugin() noexcept
    : d(std::make_unique<CsvLocationImportPluginPrivate>())
{}

CsvLocationImportPlugin::~CsvLocationImportPlugin() = default;

// PROTECTED

LocationImportPluginBaseSettings &CsvLocationImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString CsvLocationImportPlugin::getFileExtension() const noexcept
{
    return CsvLocationImportPluginPrivate::FileSuffix;
}

QString CsvLocationImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> CsvLocationImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CsvLocationImportOptionWidget>(d->pluginSettings);
}

std::vector<Location> CsvLocationImportPlugin::importLocations(QFile &file, bool *ok) noexcept
{
    std::vector<Location> locations;
    std::unique_ptr<CsvLocationParserIntf> parser;
    switch (d->pluginSettings.getFormat()) {
    case CsvLocationImportSettings::Format::LittleNavmap:
        parser = std::make_unique<LittleNavmapCsvParser>();
        break;
    }
    bool success {false};
    if (parser != nullptr) {
        QTextStream textStream(&file);
        textStream.setCodec(QTextCodec::codecForName("UTF-8"));
        locations = parser->parse(textStream, &success);
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}
