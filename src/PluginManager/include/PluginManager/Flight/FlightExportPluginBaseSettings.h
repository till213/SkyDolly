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
#ifndef FLIGHTEXPORTPLUGINBASESETTINGS_H
#define FLIGHTEXPORTPLUGINBASESETTINGS_H

#include <memory>
#include <cstdint>

#include <QObject>
#include <QString>

#include <Kernel/Settings.h>
#include <Kernel/SampleRate.h>
#include "../PluginManagerLib.h"

struct FlightExportPluginBaseSettingsPrivate;

class PLUGINMANAGER_API FlightExportPluginBaseSettings : public QObject
{
    Q_OBJECT
public:
    /*!
     * Defines how formation flights should be exported.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct FormationExport: std::uint8_t {
        First = 0,
        /*! Only the user aircraft is to be exported */
        UserAircraftOnly = First,
        /*! All aircraft are to be exported, into one file if possible (depending on the actual file format);
            otherwise into separate files */
        AllAircraftOneFile,
        /*! All aircraft are to be exported, into separate files each */
        AllAircraftSeparateFiles,
        Last = AllAircraftSeparateFiles
    };

    FlightExportPluginBaseSettings() noexcept;
    FlightExportPluginBaseSettings(const FlightExportPluginBaseSettings &rhs) = delete;
    FlightExportPluginBaseSettings(FlightExportPluginBaseSettings &&rhs) = delete;
    FlightExportPluginBaseSettings &operator=(const FlightExportPluginBaseSettings &rhs) = delete;
    FlightExportPluginBaseSettings &operator=(FlightExportPluginBaseSettings &&rhs) = delete;
    ~FlightExportPluginBaseSettings() override;

    /*!
     * Returns whether the plugin supports resampling of the (position) data to be exported
     *
     * \return \c true if the \p plugin supports position data resampling: \c false else
     */
    virtual bool isResamplingSupported() const noexcept = 0;
    SampleRate::ResamplingPeriod getResamplingPeriod() const noexcept;
    void setResamplingPeriod(SampleRate::ResamplingPeriod resamplingPeriod) noexcept;

    /*!
     * Returns whether the plugin supports the given \p formationExport option.
     *
     * \param formationExport
     *        the formation export option to test
     * \return \c true if the \p formationExport option is supported by this plugin; \c false else
     */
    virtual bool isFormationExportSupported(FormationExport formationExport) const noexcept = 0;
    FormationExport getFormationExport() const noexcept;
    void setFormationExport(FormationExport formationExport) noexcept;

    bool isOpenExportedFilesEnabled() const noexcept;
    void setOpenExportedFilesEnabled(bool enabled) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefault) const noexcept;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept;
    void restoreDefaults() noexcept;

signals:
    /*!
     * Emitted whenever the plugin settings (base settings or extended settings) have changed.
     */
    void changed();

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept = 0;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept = 0;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept = 0;
    virtual void restoreDefaultsExtn() noexcept = 0;

private:
    const std::unique_ptr<FlightExportPluginBaseSettingsPrivate> d;
};

#endif // FLIGHTEXPORTPLUGINBASESETTINGS_H
