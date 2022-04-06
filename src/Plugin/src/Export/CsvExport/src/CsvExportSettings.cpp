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

#include "../../../../../Kernel/src/Settings.h"
#include "../../Plugin/src/ExportPluginBaseSettings.h"
#include "CsvExportSettings.h"

class CsvExportSettingsPrivate
{
public:
    CsvExportSettingsPrivate()
    {}
};

// PUBLIC

CsvExportSettings::CsvExportSettings() noexcept
    : ExportPluginBaseSettings(),
      d(std::make_unique<CsvExportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("CsvExportSettings::CsvExportSettings: CREATED");
#endif
}

CsvExportSettings::~CsvExportSettings() noexcept
{
#ifdef DEBUG
    qDebug("CsvExportSettings::~CsvExportSettings: DELETED");
#endif
}

// PROTECTED

void CsvExportSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{}

void CsvExportSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{}

void CsvExportSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{}

void CsvExportSettings::restoreDefaultsExtn() noexcept
{}
