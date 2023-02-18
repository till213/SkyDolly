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
#ifndef CSVIMPORTSETTINGS_H
#define CSVIMPORTSETTINGS_H

#include <memory>

#include <QObject>
#include <QString>

#include <Kernel/Settings.h>
#include <PluginManager/FlightImportPluginBaseSettings.h>

struct CsvImportSettingsPrivate;

class CsvImportSettings : public FlightImportPluginBaseSettings
{
    Q_OBJECT
public:
    /*!
     * CSV format (flavour).
     *
     * These values are peristed in the application settings.
     */
    enum struct Format {
        SkyDolly = 0,
        FlightRadar24 = 1,
        FlightRecorder = 2
    };

    CsvImportSettings() noexcept;
    CsvImportSettings(const CsvImportSettings &rhs) = delete;
    CsvImportSettings(CsvImportSettings &&rhs) = delete;
    CsvImportSettings &operator=(const CsvImportSettings &rhs) = delete;
    CsvImportSettings &operator=(CsvImportSettings &&rhs) = delete;
    ~CsvImportSettings() override;

    bool isAircraftSelectionRequired() const noexcept override;
    bool isTimeOffsetSyncSupported() const noexcept override;

    Format getFormat() const noexcept;
    void setFormat(Format format) noexcept;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
    const std::unique_ptr<CsvImportSettingsPrivate> d;
};

#endif // CSVIMPORTSETTINGS_H
