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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

#include <QObject>
#include <QSettings>
#include <QUuid>
#include <QString>
#include <QStringLiteral>
#include <QVariant>
#include <QFileInfo>
#include <QByteArray>

#include "Replay.h"
#include "KernelLib.h"

class Version;
struct SettingsPrivate;

/*!
 * The application settings. These settings are persisted to user configuration
 * files.
 */
class KERNEL_API Settings final : public QObject
{
    Q_OBJECT
public:
    Settings(const Settings &rhs) = delete;
    Settings(Settings &&rhs) = delete;
    Settings &operator=(const Settings &rhs) = delete;
    Settings &operator=(Settings &&rhs) = delete;

    /*!
     * Returns the singleton Settings instance.
     *
     * \return a reference to the singleton
     */
    static Settings &getInstance() noexcept;

    /*!
     * Destroys the singleton instance. To be called when the application is just
     * about to terminate. The settings are persisted just before destruction
     * as well, in configuration files (or the registry).
     */
    static void destroyInstance() noexcept;

    // ********************
    // Application Settings
    // ********************

    /*!
     * Returns the version of the application (Sky Dolly) that wrote these settings last.
     *
     * \return the version of the application that wrote these settings last
     */
    const Version &getVersion() const noexcept;

    /*!
     * Returns the logbook path.
     *
     * \return the path of the logbook (database)
     */
    QString getLogbookPath() const noexcept;

    /*!
     * Sets the logbook path.
     *
     * \param logbookPath
     *        the path of the logbook
     * \sa logbookPathChanged
     */
    void setLogbookPath(QString logbookPath) noexcept;

    /*!
     * Returns whether a backup of the logbook to be migrated should be done
     * before the migration.
     *
     * \return \c true if a backup should be done before migration; \c false
     *         if migration should proceed without prior backup
     */
    bool isBackupBeforeMigrationEnabled() const noexcept;

    /*!
     * Enables or disables backups before migration.
     *
     * \param enable
     *        set to \c true if a backup should be done before logbook migration;
     *        set to \c false if migration should proceed without prior backup
     * \sa backupBeforeMigrationChanged
     */
    void setBackupBeforeMigrationEnabled(bool enable) noexcept;

    /*!
     * Returns the SkyConnect plugin UUID: an attempt to instantiate and use this plugin
     * is made upon application launch.
     *
     * \return the UUID of the last used SkyConnect plugin
     */
    QUuid getSkyConnectPluginUuid() const noexcept;

    /*!
     * Sets the SkyConnect plugin UUID.
     *
     * \param uuid
     *        the UUID of the SkyConnect plugin
     * \sa skyConnectPluginUuidChanged
     */
    void setSkyConnectPluginUuid(QUuid uuid) noexcept;

    // **********************
    // Common Replay Settings
    // **********************

    /*!
     * Returns whether the fast-forward / backward interval is an absolute value (in milliseconds).
     * \return \c true if the seek interval is an absolute value; \c false if the interval is
     *         relative (in percent of the duration)
     */
    bool isAbsoluteSeekEnabled() const noexcept;

    /*!
     * Sets whether the fast-forward / backward interval is an absolute value (in milliseconds).
     *
     * \param enable
     *        set to \c true in order to set the seek interval as an absolute value; \c false to
     *        set the seek interval as relative value (in percent of the duration)
     * \sa absoluteSeekEnabledChanged
     */
    void setAbsoluteSeekEnabled(bool enable) noexcept;

    /*!
     * Returns the absolute seek interval.
     *
     * \return the absolute seek interval in seconds
     */
    double getSeekIntervalSeconds() const noexcept;

    /*!
     * Sets the absolute seek interval.
     *
     * \param seconds
     *        the absolute seek interval in seconds
     * \sa seekIntervalSecondsChanged
     */
    void setSeekIntervalSeconds(double seconds) noexcept;

    /*!
     * Returns the relative seek interval.
     *
     * \return the relative seek interval in percent of the duration
     */
    double getSeekIntervalPercent() const noexcept;

    /*!
     * Sets the relative seek interval in percent.
     *
     * \param percent
     *        the seek interval in percent of the duration
     * \sa seekIntervalPercentChanged
     */
    void setSeekIntervalPercent(double percent) noexcept;

    /*!
     * Returns whether the replay loop is enabled or not
     *
     * \return \c true if repeated replay is enabled; \c false else
     */
    bool isReplayLoopEnabled() const noexcept;

    /*!
     * Enables the replay loop according to \p enable.
     *
     * \param enable
     *        set to \c true in order to enable repeated replay; \c false else
     * \sa replayLoopChanged
     */
    void setLoopReplayEnabled(bool enable) noexcept;

    /*!
     * Returns the replay speed unit: absolute or percent.
     *
     * \return the replay speed unit
     */
    Replay::SpeedUnit getReplaySpeeedUnit() const noexcept;

    /*!
     * Sets the replay speed unit.
     *
     * \param replaySpeedUnit
     *        the replay speed unit
     * \sa replaySpeedUnitChanged
     */
    void setReplaySpeedUnit(Replay::SpeedUnit replaySpeedUnit) noexcept;

    /*!
     * Returns the time mode.
     *
     * \return the replay time mode
     */
    Replay::TimeMode getReplayTimeMode() const noexcept;

    /*!
     * Returns whether the simulation time should be synchronised during replay.
     *
     * \return \c true if the simulation time should be synchronised during replay;
     *         \c false else (\c Replay::TimeMode::None)
     * \sa Replay::TimeMode
     */
    bool isReplayTimeModeEnabled() const noexcept;

    /*!
     * Sets the replay time mode.
     *
     * \param timeMode
     *        the replay time mode
     * \sa replayTimeModeChanged
     */
    void setReplayTimeMode(Replay::TimeMode timeMode) noexcept;

    /*!
     * Returns whether the CANOPY OPEN simulation variable
     * is repeatedly sent or not.
     *
     * \return \c true if the CANOPY OPEN value is repeatedly
     *         sent when its value is greater zero; \c false if
     *         the value is only to be sent when changed
     */
    bool isRepeatCanopyOpenEnabled() const noexcept;

    /*!
     * Enables the value repeat for the CANOPY OPEN simulation
     * variable.
     *
     * \param enable
     *        set to \c true in order to enable value repeat;
     *        \c false else
     * \sa repeatCanopyChanged
     */
    void setRepeatCanopyOpenEnabled(bool enable) noexcept;

    /*!
     * Returns the maximum simulation rate.
     *
     * \return the maximum simulation rate [1, 128]
     */
    int getMaximumSimulationRate() const noexcept;

    /*!
     * Sets the maximum simulation rate.
     *
     * \param rate
     *        the maximum simulation rate
     * \sa maximumSimulationRateChanged
     */
    void setMaximumSimulationRate(int rate) noexcept;

    // ***********************
    // User Interface Settings
    // ***********************

    /*!
     * Returns whether the  stay on top option is enabled.
     *
     * \return \c true if the application window is kept in the foreground; \c false else
     */
    bool isWindowStaysOnTopEnabled() const noexcept;

    /*!
     * Sets the  stay on top option.
     *
     * \param enable
     *        \c true in order to keep the application windoww in the foreground; \c false to
     *        enable default window behaviour
     * \sa stayOnTopChanged
     */
    void setWindowStaysOnTopEnabled(bool enable) noexcept;

    /*!
     * Returns whether the minimal user interface option is enabled.
     *
     * \return \c true if the minimal UI is shown; \c false else
     */
    bool isMinimalUiEnabled() const noexcept;

    /*!
     * Sets the minimal user interface option.
     *
     * \param enable
     *        \c true in order to show the minimal UI; \c false in order to show the normal UI;
     * \sa minimalUiChanged
     */
    void setMinimalUiEnabled(bool enable) noexcept;

    /*!
     * Returns whether the \e module selector is visible.
     *
     * \return \c true if the module selector is shown; \c false else
     */
    bool isModuleSelectorVisible() const noexcept;

    /*!
     * Sets the \e module selector visible according to \p visible.
     *
     * \param visible
     *        \c true in order to show the module selector; \c false in order to hide the module selector
     * \sa moduleSelectorVisibilityChanged
     */
    void setModuleSelectorVisible(bool visible) noexcept;

    /*!
     * Returns whether the \e replay speed group box is visible.
     *
     * \return \c true if the replay speed group box is shown
     *         \c false else
     */
    bool isReplaySpeedVisible() const noexcept;

    /*!
     * Sets the \e replay speed group box visible according to \p visible.
     *
     * \param visible
     *        \c true in order to show the replay speed
     *        \c false in order to hide the replay speed
     * \sa replaySpeedVisibilityChanged
     */
    void setReplaySpeedVisible(bool visible) noexcept;

    /*!
     * Returns the saved window geometry.
     *
     * \return the window geometry; a \e null QByteArray if not saved before
     */
    QByteArray getWindowGeometry() const noexcept;

    /*!
     * Stores the window geometry.
     *
     * \param geometry
     *        the window geometry encoded in the QByteAarray
     */
    void setWindowGeometry(QByteArray geometry) noexcept;

    /*!
     * Returns the saved window state.
     *
     * \return the window state; a \e null QByteArray if not saved before
     */
    QByteArray getWindowState() const noexcept;

    /*!
     * Stores the window state.
     *
     * \param state
     *        the window state encoded in the QByteAarray
     */
    void setWindowState(QByteArray state) noexcept;

    /*!
     * Returns the user interface style key.
     *
     * \return the user interface style key
     */
    QString getStyleKey() const noexcept;

    /*!
     * Sets the user interface style key
     *
     * \param styleKey
     *        the user interface style key
     * \sa DefaultStyleKey
     * \sa changed
     */
    void setStyleKey(QString styleKey) noexcept;

    /*!
     * Returns whether the flight deletion confirmation is enabled or not.
     *
     * \return \c true if the flight deletion confirmation is enabled; \c false else
     */
    bool isDeleteFlightConfirmationEnabled() const noexcept;

    /*!
     * Enables the flight deletion confirmation.
     *
     * \param enable
     *        set to \c true to set the flight deletion confirmation enabled;
     *        \c false else
     * \sa changed
     */
    void setDeleteFlightConfirmationEnabled(bool enable) noexcept;

    /*!
     * Returns whether the aircraft deletion confirmation is enabled or not.
     *
     * \return \c true if the aircraft deletion confirmation is enabled; \c false else
     */
    bool isDeleteAircraftConfirmationEnabled() const noexcept;

    /*!
     * Enables the aircraft deletion confirmation.
     *
     * \param enable
     *        set to \c true to set the aircraft deletion confirmation enabled;
     *        \c false else
     * \sa changed
     */
    void setDeleteAircraftConfirmationEnabled(bool enable) noexcept;

    /*!
     * Returns whether the location deletion confirmation is enabled or not.
     *
     * \return \c true if the location deletion confirmation is enabled; \c false else
     */
    bool isDeleteLocationConfirmationEnabled() const noexcept;

    /*!
     * Enables the location deletion confirmation.
     *
     * \param enable
     *        set to \c true to set the location deletion confirmation enabled;
     *        \c false else
     * \sa changed
     */
    void setDeleteLocationConfirmationEnabled(bool enable) noexcept;

    /*!
     * Returns whether the reset time offset confirmation is enabled or not.
     *
     * \return \c true if the reset time offset confirmation is enabled; \c false else
     */
    bool isResetTimeOffsetConfirmationEnabled() const noexcept;

    /*!
     * Enables the reset time offset confirmation.
     *
     * \param enable
     *        set to \c true to set the reset time offset confirmation enabled;
     *        \c false else
     * \sa changed
     */
    void setResetTimeOffsetConfirmationEnabled(bool enable) noexcept;

    /*!
     * Returns the default visibility of button text in minimal UI mode.
     *
     * \return \c true if the button text should be visible by default
     *         in minimal UI mode; \c false else
     */
    bool getDefaultMinimalUiButtonTextVisibility() const noexcept;

    /*!
     * Sets the default visibility of button text in minimal UI mode to \p visible.
     *
     * \param visible
     *        set to \c true in order to show button texts by default
     *        in minimal UI mode; \c false else
     * \sa defaultMinimalUiButtonTextVisibilityChanged
     */
    void setDefaultMinimalUiButtonTextVisibility(bool visible) noexcept;

    /*!
     * Returns the default visibility of non-essential buttons in minimal UI mode.
     *
     * \return \c true if non-essential buttons should be visible by default
     *         in minimal UI mode; \c false else
     */
    bool getDefaultMinimalUiNonEssentialButtonVisibility() const noexcept;

    /*!
     * Sets the default visibility of non-essential buttons in minimal UI mode to \p visible.
     *
     * \param visible
     *        set to \c true in order to show non-essential buttons by default
     *        in minimal UI mode \c false else
     * \sa defaultMinimalUiNonEssentialButtonVisibilityChanged
     */
    void setDefaultMinimalUiNonEssentialButtonVisibility(bool visible) noexcept;

    /*!
     * Returns the default visibility of the replay speed group in minimal UI mode.
     *
     * \return \c true if the replay speed group should be visible by default
     *         in minimal UI mode; \c false else
     */
    bool getDefaultMinimalUiReplaySpeedVisibility() const noexcept;

    /*!
     * Sets the default visibility of the replay speed group in minimal UI mode to \p visible.
     *
     * Note that the replay speed group visibility can be separately toggled by the user
     * via the View menu.
     *
     * \param visible
     *        set to \c true in order to show the replay speed group by default
     *        in minimal UI mode \c false else
     * \sa defaultMinimalUiReplaySpeedVisibilityChanged
     * \sa setReplaySpeedVisible
     */
    void setDefaultMinimalUiReplaySpeedVisibility(bool visible) noexcept;

    // ******************************
    // Common Import / Export Setings
    // ******************************

    /*!
     * Returns the aircraft type (name) for import.
     *
     * \return the aircraft type which was last selected for import
     */
    QString getImportAircraftType() const noexcept;

    /*!
     * Sets the aircraft type (name) for import.
     *
     * \param type
     *        the (last) selected aircraft type
     * \sa changed
     */
    void setImportAircraftType(QString type) noexcept;

    /*!
     * Returns the path of the directory which was last accessed during export or import.
     *
     * \return the path of the last export / import directory
     */
    QString getExportPath() const noexcept;

    /*!
     * Sets the path of the directory which was last accessed during export or import.
     *
     * \param exportPath
     *        the path of the last export / import directory
     * \sa exportPathChanged
     */
    void setExportPath(QString exportPath);

    /*!
     * Returns the count of how many times the "preview" dialog is still
     * to be shown.
     *
     * \deprecated
     */
    [[deprecated("Do not use once version 1.0 has been reached.")]]
    int getPreviewInfoDialogCount() const noexcept;

    /*!
     * Sets the count of how many times the "preview" dialog is still
     * to be shown.
     *
     * \deprecated
     */
    [[deprecated("Do not use once version 1.0 has been reached.")]]
    void setPreviewInfoDialogCount(int count) noexcept;

    using KeyValue = std::pair<QString, QVariant>;
    using ValuesByKey = std::unordered_map<QString, QVariant>;
    using KeyValues = std::vector<KeyValue>;
    using KeysWithDefaults = std::vector<KeyValue>;

    void storePluginSettings(QUuid pluginUuid, const KeyValues &keyValues) const noexcept;
    ValuesByKey restorePluginSettings(QUuid pluginUuid, const KeysWithDefaults &keys) noexcept;

    void storeModuleSettings(QUuid moduleUuid, const KeyValues &keyValues) const noexcept;
    ValuesByKey restoreModuleSettings(QUuid moduleUuid, const KeysWithDefaults &keys) noexcept;

    // Implementation note: we need to use QStringLiteral here for static inline const QStrings
    // https://forum.qt.io/topic/102312/very-strange-heap-corruption-exit-code-1073740940-0xc0000374-with-static-inline-const-qstring-release-only
    // We cannot use "static constexpr const char *" either, as this does not get exported from the DLL
    /*! The key for the default user interface style (QApplication picks the most approprate style) */
    static inline const QString DefaultStyleKey {QStringLiteral("Default")};

public slots:
    /*!
     * Stores these Settings to a user configuration file.
     */
    void store() const noexcept;

    /*!
     * Restores these Settings from a user configuration file. If no user
     * configuration is present the settings are set to default values.
     *
     * \sa #changed
     */
    void restore() noexcept;

signals:
    /*!
     * Emitted wheneverthe logbook directory path has changed.
     *
     * \sa changed
     */
    void logbookPathChanged(QString logbookPath);

    /*!
     * Emitted wheneverthe backup before migration option has changed.
     *
     * \sa changed
     */
    void backupBeforeMigrationChanged(bool enable);

    /*!
     * Emitted wheneverthe SkyConnect plugin UUID has changed.
     *
     * \sa changed
     */
    void skyConnectPluginUuidChanged(QUuid uuid);

    /*!
     * Emitted wheneverthe stay on top option has changed.
     *
     * \sa changed
     */
    void stayOnTopChanged(bool enable);

    /*!
     * Emitted wheneverthe minimal user interface option has changed.
     *
     * \sa changed
     */
    void minimalUiChanged(bool enable);

    /*!
     * Emitted wheneverthe module selector visibility has changed.
     *
     * \sa changed
     */
    void moduleSelectorVisibilityChanged(bool enable);

    /*!
     * Emitted wheneverthe replay speed visibility has changed.
     *
     * \sa changed
     */
    void replaySpeedVisibilityChanged(bool enable);

    /*!
     * Emitted wheneverthe export path has changed.
     *
     * \sa changed
     */
    void exportPathChanged(QString exportPath);

    /*!
     * Emitted wheneverthe absolute/relative seek setting has changed.
     *
     * \sa changed
     */
    void absoluteSeekEnabledChanged(bool enabled);

    /*!
     * Emitted wheneverthe seek interval in secondshas changed.
     *
     * \sa changed
     */
    void seekIntervalSecondsChanged(double seconds);

    /*!
     * Emitted wheneverthe seek interval in percent has changed.
     *
     * \sa changed
     */
    void seekIntervalPercentChanged(double percent);

    /*!
     * Emitted wheneverthe repeat replay has changed.
     *
     * \sa changed
     */
    void replayLoopChanged(bool enable);

    /*!
     * Emitted wheneverthe replay speed unit has changed.
     *
     * \sa changed
     */
    void replaySpeedUnitChanged(Replay::SpeedUnit replaySpeedUnit);

    /*!
     * Emitted wheneverthe replay time mode has changed.
     *
     * \sa changed
     */
    void replayTimeModeChanged(Replay::TimeMode replayTimeMode);

    /*!
     * Emitted wheneverthe repeat canopy has changed.
     *
     * \sa changed
     */
    void repeatCanopyChanged(bool enable);

    /*!
     * Emitted wheneverthe maximum simulation rate has changed.
     *
     * \sa changed
     */
    void maximumSimulationRateChanged(int rate);

    /*!
     * Emitted wheneverthe user interface style key has changed
     *
     * \sa changed
     */
    void styleKeyChanged(const QString &key);

    /*!
     * Emitted wheneverthe default button text visibility for the minimal UI has changed.
     *
     * \sa changed
     */
    void defaultMinimalUiButtonTextVisibilityChanged(bool hidden);

    /*!
     * Emitted wheneverthe default non-essential button visibility for the minimal UI has changed.
     *
     * \sa changed
     */
    void defaultMinimalUiNonEssentialButtonVisibilityChanged(bool hidden);

    /*!
     * Emitted wheneverthe default replay speed visibility for the minimal UI has changed.
     *
     * \sa replaySpeedVisibilityChanged
     * \sa changed
     */
    void defaultMinimalUiReplaySpeedVisibilityChanged(bool hidden);

    /*!
     * Emitted wheneverany setting has changed.
     */
    void changed();

private:
    const std::unique_ptr<SettingsPrivate> d;

    Settings() noexcept;
    friend std::unique_ptr<Settings>::deleter_type;
    ~Settings() override;

    void frenchConnection() noexcept;
};

#endif // SETTINGS_H
