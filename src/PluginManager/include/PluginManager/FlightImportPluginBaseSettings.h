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
#ifndef FLIGHTIMPORTPLUGINBASESETTINGS_H
#define FLIGHTIMPORTPLUGINBASESETTINGS_H

#include <memory>

#include <QObject>
#include <QString>

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include <Kernel/SampleRate.h>
#include "PluginManagerLib.h"

struct FlightImportPluginBaseSettingsPrivate;

class PLUGINMANAGER_API FlightImportPluginBaseSettings : public QObject
{
    Q_OBJECT
public:
    /*!
     * Defines how aircraft are to be imported.
     *
     * These values are peristed in the application settings.
     */
    enum struct AircraftImportMode {
        /*! All aircraft are added to the existing flight (loaded in memory). */
        AddToCurrentFlight = 0,
        /*! A new flight is generated and all aircraft are added to it. */
        AddToNewFlight = 1,
        /*! For each imported aircraft a new flight is generated. */
        SeparateFlights  = 2
    };

    FlightImportPluginBaseSettings() noexcept;
    FlightImportPluginBaseSettings(const FlightImportPluginBaseSettings &rhs) = delete;
    FlightImportPluginBaseSettings(FlightImportPluginBaseSettings &&rhs) = delete;
    FlightImportPluginBaseSettings &operator=(const FlightImportPluginBaseSettings &rhs) = delete;
    FlightImportPluginBaseSettings &operator=(FlightImportPluginBaseSettings &&rhs) = delete;
    ~FlightImportPluginBaseSettings() override;

    /*!
     * Returns whether the plugin requires the selection of the aircraft type to be
     * imported, that is the import format itself does not specify an aircraft type.
     *
     * \return \c true if the \c plugin requires aircraft selection; \c false else
     *         (the format itself specifies the aircraft type)
     */
    virtual bool isAircraftSelectionRequired() const noexcept = 0;

    /*!
     * Returns whether the plugin supports automated time offset synchronisation,
     * that is whether the reported flight creation time is reliably reported, based
     * on whether the format has real-world absolute timestamps (typically in UTC) or not.
     *
     * Note that the time offset synchronisation (creation time) support may
     * depend on the actual "flavour" of the given format, for example:
     *
     * - FlightRadar24 CSV: \e does support time offset synchronisation (has UTC timestamps)
     * - FlightRecorder CSV: does \e not support time offset synchronisation (only relative timestamps)
     *
     * \return \c true if the \c plugin supports timestamp offset synchronisation;
     *         \c false else (the format itself does not have real-world timestamps)
     */
    virtual bool isTimeOffsetSyncSupported() const noexcept = 0;

    bool isImportDirectoryEnabled() const noexcept;
    void setImportDirectoryEnabled(bool enabled) noexcept;

    AircraftImportMode getAircraftImportMode() const noexcept;
    void setAircraftImportMode(AircraftImportMode mode) noexcept;

    SkyMath::TimeOffsetSync getTimeOffsetSync() const noexcept;
    void setTimeOffsetSync(SkyMath::TimeOffsetSync timeOffsetSync) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefault) const noexcept;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept;
    void restoreDefaults() noexcept;

signals:
    /*!
     * Emitted whenever the base settings have changed.
     */
    void baseSettingsChanged();

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept = 0;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept = 0;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept = 0;
    virtual void restoreDefaultsExtn() noexcept = 0;

private:
    const std::unique_ptr<FlightImportPluginBaseSettingsPrivate> d;
};

#endif // FLIGHTIMPORTPLUGINBASESETTINGS_H
