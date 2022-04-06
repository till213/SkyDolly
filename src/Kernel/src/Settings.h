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
#include <QVariant>

class QByteArray;

#include "Replay.h"
#include "SampleRate.h"
#include "KernelLib.h"

class SettingsPrivate;
class Version;

/*!
 * The application settings. These settings are persisted to user configuration
 * files.
 */
class KERNEL_API Settings : public QObject
{
    Q_OBJECT
public:

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
    void setLogbookPath(const QString &logbookPath) noexcept;

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

    /*!
     * Returns the recording sample rate enumeration value.
     *
     * \return the SampleRate#SampleRate enumeration value.
     */
    SampleRate::SampleRate getRecordingSampleRate() const noexcept;

    /*!
     * Returns the recording sample rate in Hz.
     *
     * \return the recording sample rate in Hz
     */
    double getRecordingSampleRateValue() const noexcept;

    /*!
     * Sets the recording sample rate.
     *
     * \param sampleRate
     *        the SampleRate#SampleRate enumeration value
     * \sa recordingSampleRateChanged
     */
    void setRecordingSampleRate(SampleRate::SampleRate sampleRate) noexcept;

    /*!
     * Returns whether the \e stay \e on \e top option is enabled.
     *
     * \return \c true if the application window is kept in the foreground;
     *         \c false else
     */
    bool isWindowStaysOnTopEnabled() const noexcept;

    /*!
     * Sets the \e stay \e on \e top option.
     *
     * \param enable
     *        \c true in order to keep the application windoww in the foreground;
     *        \c false to enable default window behaviour
     * \sa stayOnTopChanged
     */
    void setWindowStaysOnTopEnabled(bool enable) noexcept;

    /*!
     * Returns whether the \e minimal user interface option is enabled.
     *
     * \return \c true if the minimal UI is shown;
     *         \c false else
     */
    bool isMinimalUiEnabled() const noexcept;

    /*!
     * Sets the \e minimal user interface option.
     *
     * \param enable
     *        \c true in order to show the minimal UI;
     *        \c false in order to show the normal UI;
     * \sa minimalUiChanged
     */
    void setMinimalUiEnabled(bool enable) noexcept;

    /*!
     * Returns whether the \e module selector is visible.
     *
     * \return \c true if the module selector is shown
     *         \c false else
     */
    bool isModuleSelectorVisible() const noexcept;

    /*!
     * Sets the \e module selector visible
     *
     * \param enable
     *        \c true in order to show the module selector
     *        \c false in order to hide the module selector
     * \sa moduleSelectorVisibilityChanged
     */
    void setModuleSelectorVisible(bool enable) noexcept;

    /*!
     * Returns whether the \e replay speed group box is visible.
     *
     * \return \c true if the replay speed group box is shown
     *         \c false else
     */
    bool isReplaySpeedVisible() const noexcept;

    /*!
     * Sets the \e replay speed group box visible
     *
     * \param enable
     *        \c true in order to show the replay speed
     *        \c false in order to hide the replay speed
     * \sa replaySpeedVisibilityChanged
     */
    void setReplaySpeedVisible(bool enable) noexcept;

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
    void setWindowGeometry(const QByteArray &geometry) noexcept;

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
    void setWindowState(const QByteArray &state) noexcept;

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
     * Enables the replay loop according to \c enable.
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
     */
    void setReplaySpeedUnit(Replay::SpeedUnit replaySpeedUnit) noexcept;

    /*!
     * Returns whether the FLAPS HANDLE INDEX simulation variable
     * is repeatedly sent or not.
     *
     * \return \c true if the FLAPS HANDLE INDEX value is repeatedly
     *         sent when its value is greater zero; \c false if
     *         the value is only to be sent when changed
     */
    bool isRepeatFlapsHandleIndexEnabled() const noexcept;

    /*!
     * Enables the value repeat for the FLAPS HANDLE INDEX simulation
     * variable.
     *
     * \param enable
     *        set to \c true in order to enable value repeat;
     *        \c false else
     * \sa repeatFlapsPositionChanged
     */
    void setRepeatFlapsHandleIndexEnabled(bool enable) noexcept;

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
     * Returns the aircraft type (name) for import.
     *
     * \return the aircraft type which was last selected for import
     */
    QString getImportAircraftType() const noexcept;

    /*!
     * Sets the aircraft type (name) for import
     *
     * \param type
     *        the (last) selected aircraft type
     * \sa changed
     */
    void setImportAircraftType(const QString &type) noexcept;

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

    typedef std::pair<QString, QVariant> KeyValue;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include "QStringHasher.h"
    typedef std::unordered_map<QString, QVariant, QStringHasher> ValuesByKey;
#else
    typedef std::unordered_map<QString, QVariant> ValuesByKey;
#endif
    typedef std::vector<KeyValue> KeyValues;
    typedef std::vector<KeyValue> KeysWithDefaults;

    void storePluginSettings(QUuid pluginUuid, const KeyValues &keyValues) const noexcept;
    ValuesByKey restorePluginSettings(QUuid pluginUuid, const KeysWithDefaults &keys) noexcept;

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
     * Emitted when the logbook directory path has changed.
     *
     * \sa changed
     */
    void logbookPathChanged(const QString &logbookPath);

    /*!
     * Emitted when the backup before migration option has changed.
     *
     * \sa changed
     */
    void backupBeforeMigrationChanged(bool enable);

    /*!
     * Emitted when the SkyConnect plugin UUID has changed.
     *
     * \sa changed
     */
    void skyConnectPluginUuidChanged(QUuid uuid);

    /*!
     * Emitted when the record sample rate has changed.
     *
     * \sa changed
     */
    void recordingSampleRateChanged(SampleRate::SampleRate sampleRate);

    /*!
     * Emitted when the stay on top option has changed.
     *
     * \sa changed
     */
    void stayOnTopChanged(bool enable);

    /*!
     * Emitted when the minimal user interface option has changed.
     *
     * \sa changed
     */
    void minimalUiChanged(bool enable);

    /*!
     * Emitted when the module selector visibility has changed.
     *
     * \sa changed
     */
    void moduleSelectorVisibilityChanged(bool enable);

    /*!
     * Emitted when the replay speed visibility has changed.
     *
     * \sa changed
     */
    void replaySpeedVisibilityChanged(bool enable);

    /*!
     * Emitted when the export path has changed.
     *
     * \sa changed
     */
    void exportPathChanged(const QString &exportPath);

    /*!
     * Emitted when the absolute/relative seek setting has changed.
     *
     * \sa changed
     */
    void absoluteSeekEnabledChanged(bool enabled);

    /*!
     * Emitted when the seek interval in secondshas changed.
     *
     * \sa changed
     */
    void seekIntervalSecondsChanged(double seconds);

    /*!
     * Emitted when the seek interval in percent has changed.
     *
     * \sa changed
     */
    void seekIntervalPercentChanged(double percent);

    /*!
     * Emitted when the repeat replay has changed.
     *
     * \sa changed
     */
    void replayLoopChanged(bool enable);

    /*!
     * Emitted when the replay speed unit has changed.
     *
     * \sa changed
     */
    void replaySpeedUnitChanged(Replay::SpeedUnit replaySpeedUnit);

    /*!
     * Emitted when the repeat flaps position has changed.
     *
     * \sa changed
     */
    void repeatFlapsPositionChanged(bool enable);

    /*!
     * Emitted when the repeat canopy has changed.
     *
     * \sa changed
     */
    void repeatCanopyChanged(bool enable);

    /*!
     * Emitted when any setting has changed.
     */
    void changed();

protected:
    virtual ~Settings();
    const Version &getVersion() const;

private:
    Q_DISABLE_COPY(Settings)
    std::unique_ptr<SettingsPrivate> d;

    Settings() noexcept;

    void frenchConnection() noexcept;
};

#endif // SETTINGS_H
