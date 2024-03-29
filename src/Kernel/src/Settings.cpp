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
#include <mutex>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QString>
#include <QStringBuilder>
#include <QLatin1String>
#include <QUuid>
#include <QByteArray>
#include <QVariant>
#include <QDir>
#include <QFileInfo>
#ifdef DEBUG
#include <QDebug>
#endif

#include "Enum.h"
#include "Const.h"
#include "SampleRate.h"
#include "Version.h"
#include "SettingsConverter.h"
#include "Settings.h"

namespace
{
    constexpr const char *ResourceDirectoryName {"Resources"};
    // This happens to be the same directory name as when unzipping the downloaded EGM data
    // from https://geographiclib.sourceforge.io/html/geoid.html#geoidinst
    constexpr const char *EgmDirectoryName {"geoids"};
    constexpr const char *DefaultEgmFileName {"egm2008-5.pgm"};
}

struct SettingsPrivate
{
    SettingsPrivate() noexcept
    {
        QStringList standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
        if (standardLocations.count() > 0) {
            defaultExportPath = standardLocations.first();
            const QString defaultLogbookName {Version::getApplicationName() % " " % QObject::tr("Logbook")};
            defaultLogbookPath = standardLocations.first() % "/" % defaultLogbookName % "/" % defaultLogbookName % Const::DotLogbookExtension;
        } else {
            defaultExportPath = ".";
        }
    }

    QSettings settings;
    Version version;

    QString logbookPath;
    bool backupBeforeMigration {DefaultBackupBeforeMigration};
    QUuid skyConnectPluginUuid;
    double recordingSampleRateValue {DefaultRecordingSampleRate};
    bool windowStayOnTop {DefaultWindowStayOnTop};
    bool minimalUi {DefaultMinimalUi};
    bool moduleSelectorVisible {DefaultModuleSelectorVisible};
    bool replaySpeedVisible {DefaultReplaySpeedVisible};
    QByteArray windowGeometry;
    QByteArray windowState;
    QString exportPath;
    QString defaultExportPath;
    QString defaultLogbookPath;
    bool absoluteSeek {false};
    double seekIntervalSeconds {DefaultSeekIntervalSeconds};
    double seekIntervalPercent {DefaultSeekIntervalPercent};
    bool replayLoop {DefaultReplayLoop};
    Replay::SpeedUnit replaySpeedUnit {DefaultReplaySpeedUnit};
    bool repeatCanopyOpen {DefaultRepeatCanopyOpen};

    bool deleteFlightConfirmation {DefaultDeleteFlightConfirmation};
    bool deleteAircraftConfirmation {DefaultDeleteAircraftConfirmation};
    bool deleteLocationConfirmation {DefaultDeleteLocationConfirmation};
    bool resetTimeOffsetConfirmation {DefaultResetTimeOffsetConfirmation};

    bool defaultMinimalUiButtonTextVisible {DefaultMinimalUiButtonTextVisible};
    bool defaultMinimalUiNonEssentialButtonVisible {DefaultMinimalUiNonEssentialButtonVisible};
    bool defaultMinimalUiReplaySpeedVisible {DefaultMinimalUiReplaySpeedVisible};

    QString importAircraftType;
    QFileInfo earthGravityModelFileInfo;
    FlightSimulatorShortcuts flightSimulatorShortcuts {DefaultFlightSimulatorShortcuts};

    int previewInfoDialogCount {DefaultPreviewInfoDialogCount};

    static inline std::once_flag onceFlag;
    static inline Settings *instance {nullptr};

    static constexpr QUuid DefaultSkyConnectPluginUuid {};
    static constexpr bool DefaultBackupBeforeMigration {true};
    static constexpr double DefaultRecordingSampleRate {SampleRate::toValue(SampleRate::SampleRate::Auto)};
    static constexpr bool DefaultWindowStayOnTop {false};
    static constexpr bool DefaultMinimalUi {false};
    static constexpr bool DefaultModuleSelectorVisible {true};
    static constexpr bool DefaultReplaySpeedVisible {true};
    static constexpr bool DefaultAbsoluteSeek {true};
    static constexpr double DefaultSeekIntervalSeconds {1.0};
    static constexpr double DefaultSeekIntervalPercent {0.5};
    static constexpr bool DefaultReplayLoop {false};
    static constexpr Replay::SpeedUnit DefaultReplaySpeedUnit {Replay::SpeedUnit::Absolute};
    // For now the default value is true, as no known aircraft exists where the canopy values would not
    // have to be repeated
    static constexpr bool DefaultRepeatCanopyOpen {true};
    static constexpr bool DefaultDeleteFlightConfirmation {true};
    static constexpr bool DefaultDeleteAircraftConfirmation {true};
    static constexpr bool DefaultDeleteLocationConfirmation {true};
    static constexpr bool DefaultResetTimeOffsetConfirmation {true};

    static constexpr bool DefaultMinimalUiButtonTextVisible {false};
    static constexpr bool DefaultMinimalUiNonEssentialButtonVisible {false};
    static constexpr bool DefaultMinimalUiReplaySpeedVisible {false};

    static inline const QString DefaultImportAircraftType {};

    static constexpr int DefaultPreviewInfoDialogCount {3};
    static constexpr int PreviewInfoDialogBase {150};

            // TODO Select proper defaults (non-assigned by default in MSFS)
    static inline const QKeySequence DefaultRecordShortcut {"Ctrl+R"};
    static inline const QKeySequence DefaultReplayShortcut {"Ctrl+E"};
    static inline const QKeySequence DefaultPauseShortcut {"Ctrl+P"};
    static inline const QKeySequence DefaultStopShortcut {"Ctrl+S"};
    static inline const QKeySequence DefaultBackwardShortcut {"Ctrl+B"};
    static inline const QKeySequence DefaultForwardShortcut {"Ctrl+F"};
    static inline const QKeySequence DefaultRewindShortcut {"Ctrl+T"};
    static inline const FlightSimulatorShortcuts DefaultFlightSimulatorShortcuts {
        DefaultRecordShortcut,
        DefaultReplayShortcut,
        DefaultPauseShortcut,
        DefaultStopShortcut,
        DefaultBackwardShortcut,
        DefaultForwardShortcut,
        DefaultRewindShortcut
    };
};

// PUBLIC

Settings &Settings::getInstance() noexcept
{
    std::call_once(SettingsPrivate::onceFlag, []() {
        SettingsPrivate::instance = new Settings();
    });
    return *SettingsPrivate::instance;
}

void Settings::destroyInstance() noexcept
{
    if (SettingsPrivate::instance != nullptr) {
        delete SettingsPrivate::instance;
        SettingsPrivate::instance = nullptr;
    }
}

const Version &Settings::getVersion() const noexcept
{
    return d->version;
}

QString Settings::getLogbookPath() const noexcept
{
    return d->logbookPath;
}

void Settings::setLogbookPath(const QString &logbookPath) noexcept
{
    if (d->logbookPath != logbookPath) {
        d->logbookPath = logbookPath;
        emit logbookPathChanged(d->logbookPath);
    }
}

bool Settings::isBackupBeforeMigrationEnabled() const noexcept
{
    return d->backupBeforeMigration;
}

void Settings::setBackupBeforeMigrationEnabled(bool enable) noexcept
{
    if (d->backupBeforeMigration != enable) {
        d->backupBeforeMigration = enable;
        emit backupBeforeMigrationChanged(d->backupBeforeMigration);
    }
}

QUuid Settings::getSkyConnectPluginUuid() const noexcept
{
    return d->skyConnectPluginUuid;
}

void Settings::setSkyConnectPluginUuid(QUuid uuid) noexcept
{
    if (d->skyConnectPluginUuid != uuid) {
        d->skyConnectPluginUuid = uuid;
        emit skyConnectPluginUuidChanged(d->skyConnectPluginUuid);
    }
}

QString Settings::getExportPath() const noexcept
{
    return d->exportPath;
}

void Settings::setExportPath(const QString &exportPath)
{
    if (d->exportPath != exportPath) {
        d->exportPath = exportPath;
        emit exportPathChanged(d->exportPath);
    }
}

SampleRate::SampleRate Settings::getRecordingSampleRate() const noexcept
{
    return SampleRate::fromValue(d->recordingSampleRateValue);
}

double Settings::getRecordingSampleRateValue() const noexcept
{
    return d->recordingSampleRateValue;
}

void Settings::setRecordingSampleRate(SampleRate::SampleRate sampleRate) noexcept
{
    double sampleRateValue = SampleRate::toValue(sampleRate);
    if (d->recordingSampleRateValue != sampleRateValue) {
        d->recordingSampleRateValue = sampleRateValue;
        emit recordingSampleRateChanged(sampleRate);
    }
}

bool Settings::isWindowStaysOnTopEnabled() const noexcept
{
    return d->windowStayOnTop;
}

void Settings::setWindowStaysOnTopEnabled(bool enable) noexcept
{
    if (d->windowStayOnTop != enable) {
        d->windowStayOnTop = enable;
        emit stayOnTopChanged(enable);
    }
}

bool Settings::isMinimalUiEnabled() const noexcept
{
    return d->minimalUi;
}

void Settings::setMinimalUiEnabled(bool enable) noexcept
{
    if (d->minimalUi != enable) {
        d->minimalUi = enable;
        emit minimalUiChanged(enable);
    }
}

bool Settings::isModuleSelectorVisible() const noexcept
{
    return d->moduleSelectorVisible;
}

void Settings::setModuleSelectorVisible(bool visible) noexcept
{
    if (d->moduleSelectorVisible != visible) {
        d->moduleSelectorVisible = visible;
        emit moduleSelectorVisibilityChanged(visible);
    }
}

bool Settings::isReplaySpeedVisible() const noexcept
{
    return d->replaySpeedVisible;
}

void Settings::setReplaySpeedVisible(bool visible) noexcept
{
    if (d->replaySpeedVisible != visible) {
        d->replaySpeedVisible = visible;
        emit replaySpeedVisibilityChanged(visible);
    }
}

QByteArray Settings::getWindowGeometry() const noexcept
{
    return d->windowGeometry;
}

void Settings::setWindowGeometry(const QByteArray &geometry) noexcept
{
    d->windowGeometry = geometry;
}

QByteArray Settings::getWindowState() const noexcept
{
    return d->windowState;
}

void Settings::setWindowState(const QByteArray &state) noexcept
{
    d->windowState = state;
}

bool Settings::isAbsoluteSeekEnabled() const noexcept
{
    return d->absoluteSeek;
}

void Settings::setAbsoluteSeekEnabled(bool enable) noexcept
{
    if (d->absoluteSeek != enable) {
        d->absoluteSeek = enable;
        emit absoluteSeekEnabledChanged(d->absoluteSeek);
    }
}

double Settings::getSeekIntervalSeconds() const noexcept
{
    return d->seekIntervalSeconds;
}

void Settings::setSeekIntervalSeconds(double seconds) noexcept
{
    if (d->seekIntervalSeconds != seconds) {
        d->seekIntervalSeconds = seconds;
        emit seekIntervalSecondsChanged(d->seekIntervalSeconds);
    }
}

double Settings::getSeekIntervalPercent() const noexcept
{
    return d->seekIntervalPercent;
}

void Settings::setSeekIntervalPercent(double percent) noexcept
{
    if (d->seekIntervalPercent != percent) {
        d->seekIntervalPercent = percent;
        emit seekIntervalPercentChanged(d->seekIntervalPercent);
    }
}

bool Settings::isReplayLoopEnabled() const noexcept
{
    return d->replayLoop;
}

void Settings::setLoopReplayEnabled(bool enable) noexcept
{
    if (d->replayLoop != enable) {
        d->replayLoop = enable;
        emit replayLoopChanged(d->replayLoop);
    }
}

Replay::SpeedUnit Settings::getReplaySpeeedUnit() const noexcept
{
    return d->replaySpeedUnit;
}

void Settings::setReplaySpeedUnit(Replay::SpeedUnit replaySpeedUnit) noexcept
{
    if (d->replaySpeedUnit != replaySpeedUnit) {
        d->replaySpeedUnit = replaySpeedUnit;
        emit replaySpeedUnitChanged(d->replaySpeedUnit);
    }
}

bool Settings::isRepeatCanopyOpenEnabled() const noexcept
{
    return d->repeatCanopyOpen;
}

void Settings::setRepeatCanopyOpenEnabled(bool enable) noexcept
{
    if (d->repeatCanopyOpen != enable) {
        d->repeatCanopyOpen = enable;
        emit repeatCanopyChanged(d->repeatCanopyOpen);
    }
}

bool Settings::isDeleteFlightConfirmationEnabled() const noexcept
{
    return d->deleteFlightConfirmation;
}

void Settings::setDeleteFlightConfirmationEnabled(bool enable) noexcept
{
    if (d->deleteFlightConfirmation != enable) {
        d->deleteFlightConfirmation = enable;
        emit changed();
    }
}

bool Settings::isDeleteAircraftConfirmationEnabled() const noexcept
{
    return d->deleteAircraftConfirmation;
}

void Settings::setDeleteAircraftConfirmationEnabled(bool enable) noexcept
{
    if (d->deleteAircraftConfirmation != enable) {
        d->deleteAircraftConfirmation = enable;
        emit changed();
    }
}

bool Settings::isDeleteLocationConfirmationEnabled() const noexcept
{
    return d->deleteLocationConfirmation;
}

void Settings::setDeleteLocationConfirmationEnabled(bool enable) noexcept
{
    if (d->deleteLocationConfirmation != enable) {
        d->deleteLocationConfirmation = enable;
        emit changed();
    }
}

bool Settings::isResetTimeOffsetConfirmationEnabled() const noexcept
{
    return d->resetTimeOffsetConfirmation;
}

void Settings::setResetTimeOffsetConfirmationEnabled(bool enable) noexcept
{
    if (d->resetTimeOffsetConfirmation != enable) {
        d->resetTimeOffsetConfirmation = enable;
        emit changed();
    }
}

bool Settings::getDefaultMinimalUiButtonTextVisibility() const noexcept
{
    return d->defaultMinimalUiButtonTextVisible;
}

void Settings::setDefaultMinimalUiButtonTextVisibility(bool visible) noexcept
{
    if (d->defaultMinimalUiButtonTextVisible != visible) {
        d->defaultMinimalUiButtonTextVisible = visible;
        emit defaultMinimalUiButtonTextVisibilityChanged(visible);
    }
}

bool Settings::getDefaultMinimalUiNonEssentialButtonVisibility() const noexcept
{
    return d->defaultMinimalUiNonEssentialButtonVisible;
}

void Settings::setDefaultMinimalUiNonEssentialButtonVisibility(bool visible) noexcept
{
    if (d->defaultMinimalUiNonEssentialButtonVisible != visible) {
        d->defaultMinimalUiNonEssentialButtonVisible = visible;
        emit defaultMinimalUiNonEssentialButtonVisibilityChanged(visible);
    }
}

bool Settings::getDefaultMinimalUiReplaySpeedVisibility() const noexcept
{
    return d->defaultMinimalUiReplaySpeedVisible;
}

void Settings::setDefaultMinimalUiReplaySpeedVisibility(bool visible) noexcept
{
    if (d->defaultMinimalUiReplaySpeedVisible != visible) {
        d->defaultMinimalUiReplaySpeedVisible = visible;
        emit defaultMinimalUiReplaySpeedVisibilityChanged(visible);
    }
}

QString Settings::getImportAircraftType() const noexcept
{
    return d->importAircraftType;
}

void Settings::setImportAircraftType(const QString &type) noexcept
{
    if (d->importAircraftType != type) {
        d->importAircraftType = type;
        emit changed();
    }
}

FlightSimulatorShortcuts Settings::getFlightSimulatorShortcuts() const noexcept
{
    return d->flightSimulatorShortcuts;
}

void Settings::setFlightSimulatorShortcuts(FlightSimulatorShortcuts shortcuts) noexcept
{
    if (d->flightSimulatorShortcuts != shortcuts) {
        d->flightSimulatorShortcuts = std::move(shortcuts);
        emit flightSimulatorShortcutsChanged(d->flightSimulatorShortcuts);
    }
}

QFileInfo Settings::getEarthGravityModelFileInfo() const noexcept
{
    return d->earthGravityModelFileInfo;
}

bool Settings::hasEarthGravityModel() const noexcept
{
    return d->earthGravityModelFileInfo.exists();
}

int Settings::getPreviewInfoDialogCount() const noexcept
{
    return d->previewInfoDialogCount - SettingsPrivate::PreviewInfoDialogBase;
}

void Settings::setPreviewInfoDialogCount(int count) noexcept
{
    if (d->previewInfoDialogCount != count) {
        d->previewInfoDialogCount = count + SettingsPrivate::PreviewInfoDialogBase;
        emit changed();
    }
}

void Settings::storePluginSettings(QUuid pluginUuid, const KeyValues &keyValues) const noexcept
{
    d->settings.beginGroup("Plugins/" + pluginUuid.toByteArray());
    {
        for (const auto &keyValue : keyValues) {
            d->settings.setValue(keyValue.first, keyValue.second);
        }
    }
    d->settings.endGroup();
}

Settings::ValuesByKey Settings::restorePluginSettings(QUuid pluginUuid, const KeysWithDefaults &keys) noexcept
{
    ValuesByKey values;
    d->settings.beginGroup("Plugins/" + pluginUuid.toByteArray());
    {
        for (const auto &key : keys) {
            values[key.first] = d->settings.value(key.first, key.second);
        }
    }
    d->settings.endGroup();

    return values;
}

void Settings::storeModuleSettings(QUuid moduleUuid, const KeyValues &keyValues) const noexcept
{
    d->settings.beginGroup("Plugins/Modules/" + moduleUuid.toByteArray());
    {
        for (const auto &keyValue : keyValues) {
            d->settings.setValue(keyValue.first, keyValue.second);
        }
    }
    d->settings.endGroup();
}

Settings::ValuesByKey Settings::restoreModuleSettings(QUuid moduleUuid, const KeysWithDefaults &keys) noexcept
{
    ValuesByKey values;
    d->settings.beginGroup("Plugins/Modules/" + moduleUuid.toByteArray());
    {
        for (const auto &key : keys) {
            values[key.first] = d->settings.value(key.first, key.second);
        }
    }
    d->settings.endGroup();

    return values;
}

// PUBLIC SLOTS

void Settings::store() const noexcept
{
    d->settings.setValue("Version", d->version.toString());
    d->settings.beginGroup("Logbook");
    {
        d->settings.setValue("Path", d->logbookPath);
        d->settings.setValue("BackupBeforeMigration", d->backupBeforeMigration);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Plugins");
    {
        d->settings.setValue("SkyConnectPluginUuid", d->skyConnectPluginUuid);
        d->settings.beginGroup("FlightSimulatorShortcuts");
        {
            d->settings.setValue("Record", d->flightSimulatorShortcuts.record);
            d->settings.setValue("Replay", d->flightSimulatorShortcuts.replay);
            d->settings.setValue("Pause", d->flightSimulatorShortcuts.pause);
            d->settings.setValue("Stop", d->flightSimulatorShortcuts.stop);
            d->settings.setValue("Backward", d->flightSimulatorShortcuts.backward);
            d->settings.setValue("Forward", d->flightSimulatorShortcuts.forward);
            d->settings.setValue("Rewind", d->flightSimulatorShortcuts.rewind);
        }
        d->settings.endGroup();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Recording");
    {
        d->settings.setValue("RecordingSampleRate", d->recordingSampleRateValue);        
    }
    d->settings.endGroup();
    d->settings.beginGroup("Replay");
    {
        d->settings.setValue("AbsoluteSeek", d->absoluteSeek);
        d->settings.setValue("SeekIntervalSeconds", d->seekIntervalSeconds);
        d->settings.setValue("SeekIntervalPercent", d->seekIntervalPercent);
        d->settings.setValue("ReplayLoop", d->replayLoop);
        d->settings.setValue("ReplaySpeedUnit", Enum::underly(d->replaySpeedUnit));
        d->settings.setValue("RepeatCanopyOpen", d->repeatCanopyOpen);
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        // Confirmations
        d->settings.setValue("DeleteFlightConfirmation", d->deleteFlightConfirmation);
        d->settings.setValue("DeleteAircraftConfirmation", d->deleteAircraftConfirmation);
        d->settings.setValue("DeleteLocationConfirmation", d->deleteLocationConfirmation);
        d->settings.setValue("ResetTimeOffsetConfirmation", d->resetTimeOffsetConfirmation);

        // Minimal UI
        d->settings.setValue("DefaultMinimalUiButtonTextVisible", d->defaultMinimalUiButtonTextVisible);
        d->settings.setValue("DefaultMinimalUiNonEssentialButtonVisible", d->defaultMinimalUiNonEssentialButtonVisible);
        d->settings.setValue("DefaultMinimalUiReplaySpeedVisible", d->defaultMinimalUiReplaySpeedVisible);
    }
    d->settings.endGroup();
    d->settings.beginGroup("View");
    {
        d->settings.setValue("ModuleSelectorVisible", d->moduleSelectorVisible);
        d->settings.setValue("ReplaySpeedVisible", d->replaySpeedVisible);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->settings.setValue("WindowStaysOnTop", d->windowStayOnTop);
        d->settings.setValue("MinimalUi", d->minimalUi);
        d->settings.setValue("Geometry", d->windowGeometry);
        d->settings.setValue("State", d->windowState);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Paths");
    {
        d->settings.setValue("ExportPath", d->exportPath);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Import");
    {
        d->settings.setValue("AircraftType", d->importAircraftType);
    }
    d->settings.endGroup();
    d->settings.beginGroup("_Preview");
    {
        d->settings.setValue("PreviewInfoDialogCount", d->previewInfoDialogCount);
    }
    d->settings.endGroup();
}

void Settings::restore() noexcept
{
    QString versionString;
    versionString = d->settings.value("Version", getVersion().toString()).toString();
    Version settingsVersion {versionString};
    if (settingsVersion < getVersion()) {
#ifdef DEBUG
        qDebug() << "Settings::restore: app version:" << getVersion().toString() << "settings version:" << settingsVersion.toString() << "converting...";
#endif
        SettingsConverter::convertToCurrent(settingsVersion, d->settings);
    }

#ifdef DEBUG
    qDebug() << "Settings::restore: app name:" << d->settings.applicationName() <<  "organisation name:" << d->settings.organizationName();
#endif

    bool ok {true};
    d->settings.beginGroup("Logbook");
    {
        d->logbookPath = d->settings.value("Path", d->defaultLogbookPath).toString();
        d->backupBeforeMigration = d->settings.value("BackupBeforeMigration", SettingsPrivate::DefaultBackupBeforeMigration).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Plugins");
    {
        d->skyConnectPluginUuid = d->settings.value("SkyConnectPluginUuid", d->DefaultSkyConnectPluginUuid).toUuid();
        d->settings.beginGroup("FlightSimulatorShortcuts");
        {
            d->flightSimulatorShortcuts.record = d->settings.value("Record", d->DefaultRecordShortcut).toString();
            d->flightSimulatorShortcuts.replay = d->settings.value("Replay", d->DefaultReplayShortcut).toString();
            d->flightSimulatorShortcuts.pause = d->settings.value("Pause", d->DefaultPauseShortcut).toString();
            d->flightSimulatorShortcuts.stop = d->settings.value("Stop", d->DefaultStopShortcut).toString();
            d->flightSimulatorShortcuts.backward = d->settings.value("Backward", d->DefaultBackwardShortcut).toString();
            d->flightSimulatorShortcuts.forward = d->settings.value("Forward", d->DefaultForwardShortcut).toString();
            d->flightSimulatorShortcuts.rewind = d->settings.value("Rewind", d->DefaultRewindShortcut).toString();
        }
        d->settings.endGroup();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Recording");
    {
        d->recordingSampleRateValue = d->settings.value("RecordingSampleRate", SettingsPrivate::DefaultRecordingSampleRate).toDouble(&ok);
        if (!ok) {
#ifdef DEBUG
            qWarning() << "The recording sample rate in the settings could not be parsed, so setting value to default value:" << SettingsPrivate::DefaultRecordingSampleRate;
#endif
            d->recordingSampleRateValue = SettingsPrivate::DefaultRecordingSampleRate;
        }
    }
    d->settings.endGroup();    
    d->settings.beginGroup("Replay");
    {
        d->absoluteSeek = d->settings.value("AbsoluteSeek", SettingsPrivate::DefaultAbsoluteSeek).toBool();
        d->seekIntervalSeconds = d->settings.value("SeekIntervalSeconds", SettingsPrivate::DefaultSeekIntervalSeconds).toDouble(&ok);
        if (!ok) {
#ifdef DEBUG
            qWarning() << "The seek interval [seconds] in the settings could not be parsed, so setting value to default value:" << SettingsPrivate::DefaultSeekIntervalSeconds;
#endif
            d->seekIntervalSeconds = SettingsPrivate::DefaultSeekIntervalSeconds;
        }
        d->seekIntervalPercent = d->settings.value("SeekIntervalPercent", SettingsPrivate::DefaultSeekIntervalPercent).toDouble(&ok);
        if (!ok) {
#ifdef DEBUG
            qWarning() << "The seek interval [percent] in the settings could not be parsed, so setting value to default value:" << SettingsPrivate::DefaultSeekIntervalPercent;
#endif
            d->seekIntervalPercent = SettingsPrivate::DefaultSeekIntervalPercent;
        }
        d->replayLoop = d->settings.value("ReplayLoop", SettingsPrivate::DefaultReplayLoop).toBool();
        int replaySpeedUnitValue = d->settings.value("ReplaySpeedUnit", Enum::underly(SettingsPrivate::DefaultReplaySpeedUnit)).toInt(&ok);
        if (ok) {
            d->replaySpeedUnit = static_cast<Replay::SpeedUnit>(replaySpeedUnitValue);
        } else {
#ifdef DEBUG
            qWarning() << "The replay speed unit in the settings could not be parsed, so setting value to default value:" << Enum::underly(SettingsPrivate::DefaultReplaySpeedUnit);
#endif
            d->replaySpeedUnit = SettingsPrivate::DefaultReplaySpeedUnit;
        }
        d->repeatCanopyOpen = d->settings.value("RepeatCanopyOpen", SettingsPrivate::DefaultRepeatCanopyOpen).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        // Confirmations
        d->deleteFlightConfirmation = d->settings.value("DeleteFlightConfirmation", SettingsPrivate::DefaultDeleteFlightConfirmation).toBool();
        d->deleteAircraftConfirmation = d->settings.value("DeleteAircraftConfirmation", SettingsPrivate::DefaultDeleteAircraftConfirmation).toBool();
        d->deleteLocationConfirmation = d->settings.value("DeleteLocationConfirmation", SettingsPrivate::DefaultDeleteLocationConfirmation).toBool();
        d->resetTimeOffsetConfirmation = d->settings.value("ResetTimeOffsetConfirmation", SettingsPrivate::DefaultResetTimeOffsetConfirmation).toBool();

        d->defaultMinimalUiButtonTextVisible = d->settings.value("DefaultMinimalUiButtonTextVisible", SettingsPrivate::DefaultMinimalUiButtonTextVisible).toBool();
        d->defaultMinimalUiNonEssentialButtonVisible = d->settings.value("DefaultMinimalUiNonEssentialButtonVisible", SettingsPrivate::DefaultMinimalUiNonEssentialButtonVisible).toBool();
        d->defaultMinimalUiReplaySpeedVisible = d->settings.value("DefaultMinimalUiReplaySpeedVisible", SettingsPrivate::DefaultMinimalUiReplaySpeedVisible).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("View");
    {
        d->moduleSelectorVisible = d->settings.value("ModuleSelectorVisible", SettingsPrivate::DefaultModuleSelectorVisible).toBool();
        d->replaySpeedVisible = d->settings.value("ReplaySpeedVisible", SettingsPrivate::DefaultReplaySpeedVisible).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->windowStayOnTop = d->settings.value("WindowStaysOnTop", SettingsPrivate::DefaultWindowStayOnTop).toBool();
        d->minimalUi = d->settings.value("MinimalUi", SettingsPrivate::DefaultMinimalUi).toBool();
        d->windowGeometry = d->settings.value("Geometry").toByteArray();
        d->windowState = d->settings.value("State").toByteArray();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Paths");
    {
        d->exportPath = d->settings.value("ExportPath", d->defaultExportPath).toString();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Import");
    {
        d->importAircraftType = d->settings.value("AircraftType", SettingsPrivate::DefaultImportAircraftType).toString();
    }
    d->settings.endGroup();
    d->settings.beginGroup("_Preview");
    {
        d->previewInfoDialogCount = d->settings.value("PreviewInfoDialogCount", SettingsPrivate::DefaultPreviewInfoDialogCount + SettingsPrivate::PreviewInfoDialogBase).toInt(&ok);
        if (!ok) {
            d->previewInfoDialogCount = SettingsPrivate::DefaultPreviewInfoDialogCount + SettingsPrivate::PreviewInfoDialogBase;
        }
        if (d->previewInfoDialogCount < SettingsPrivate::PreviewInfoDialogBase) {
            d->previewInfoDialogCount = SettingsPrivate::DefaultPreviewInfoDialogCount + SettingsPrivate::PreviewInfoDialogBase;
        }
    }
    d->settings.endGroup();
}

// PRIVATE

Settings::Settings() noexcept
    : d(std::make_unique<SettingsPrivate>())
{
    restore();
    frenchConnection();
    updateEgmFilePath();
}

Settings::~Settings()
{
    store();
}

void Settings::frenchConnection() noexcept
{
    connect(this, &Settings::logbookPathChanged,
            this, &Settings::changed);
    connect(this, &Settings::backupBeforeMigrationChanged,
            this, &Settings::changed);
    connect(this, &Settings::skyConnectPluginUuidChanged,
            this, &Settings::changed);
    connect(this, &Settings::recordingSampleRateChanged,
            this, &Settings::changed);
    connect(this, &Settings::stayOnTopChanged,
            this, &Settings::changed);
    connect(this, &Settings::minimalUiChanged,
            this, &Settings::changed);
    connect(this, &Settings::moduleSelectorVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::replaySpeedVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::exportPathChanged,
            this, &Settings::changed);
    connect(this, &Settings::absoluteSeekEnabledChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalSecondsChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalPercentChanged,
            this, &Settings::changed);
    connect(this, &Settings::replayLoopChanged,
            this, &Settings::changed);
    connect(this, &Settings::replaySpeedUnitChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatCanopyChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiButtonTextVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiNonEssentialButtonVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiReplaySpeedVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::flightSimulatorShortcutsChanged,
            this, &Settings::changed);
}

// PRIVATE SLOTS

void Settings::updateEgmFilePath() noexcept
{
    d->earthGravityModelFileInfo = QFileInfo();
    QDir egmDirectory = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (egmDirectory.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            egmDirectory.cdUp();
        }
#endif
    if (egmDirectory.cd(::ResourceDirectoryName)) {
        if (egmDirectory.cd(::EgmDirectoryName)) {
            if (egmDirectory.exists(::DefaultEgmFileName)) {
                d->earthGravityModelFileInfo = QFileInfo(egmDirectory.absoluteFilePath(::DefaultEgmFileName));
            }
        }
    }
}
