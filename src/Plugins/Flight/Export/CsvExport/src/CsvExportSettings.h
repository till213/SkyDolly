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
#ifndef CSVEXPORTSETTINGS_H
#define CSVEXPORTSETTINGS_H

#include <memory>

#include <QObject>

#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportPluginBaseSettings.h>

struct CsvExportSettingsPrivate;

class CsvExportSettings : public FlightExportPluginBaseSettings
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
        PositionAndAttitude = 2
    };

    CsvExportSettings() noexcept;
    CsvExportSettings(const CsvExportSettings &rhs) = delete;
    CsvExportSettings(CsvExportSettings &&rhs) = delete;
    CsvExportSettings &operator=(const CsvExportSettings &rhs) = delete;
    CsvExportSettings &operator=(CsvExportSettings &&rhs) = delete;
    ~CsvExportSettings() override;

    Format getFormat() const noexcept;
    void setFormat(Format format) noexcept;

    bool isResamplingSupported() const noexcept override;
    bool isFormationExportSupported(FormationExport formationExport) const noexcept override;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
    const std::unique_ptr<CsvExportSettingsPrivate> d;
};

#endif // CSVEXPORTSETTINGS_H
