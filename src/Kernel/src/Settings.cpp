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
#include <utility>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QString>
#include <QStringBuilder>
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
#include "Version.h"
#include "SettingsConverter.h"
#include "Settings.h"

struct SettingsPrivate
{
    SettingsPrivate() noexcept
    {
        auto standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
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
    bool windowStayOnTop {DefaultWindowStayOnTop};
    bool minimalUi {DefaultMinimalUi};
    bool moduleSelectorVisible {DefaultModuleSelectorVisible};
    bool replaySpeedVisible {DefaultReplaySpeedVisible};
    QByteArray windowGeometry;
    QByteArray windowState;
    QString exportPath;
    QString defaultExportPath;
    QString defaultLogbookPath;

    // Replay options
    bool absoluteSeek {false};
    double seekIntervalSeconds {DefaultSeekIntervalSeconds};
    double seekIntervalPercent {DefaultSeekIntervalPercent};
    bool replayLoop {DefaultReplayLoop};
    Replay::SpeedUnit replaySpeedUnit {DefaultReplaySpeedUnit};
    bool repeatCanopyOpen {DefaultRepeatCanopyOpen};
    int maximumSimulationRate {DefaultMaximumSimulationRate};
    Replay::TimeMode replayTimeMode {DefaultReplayTimeMode};

    QString styleKey {Settings::DefaultStyleKey};

    bool deleteFlightConfirmation {DefaultDeleteFlightConfirmation};
    bool deleteAircraftConfirmation {DefaultDeleteAircraftConfirmation};
    bool deleteLocationConfirmation {DefaultDeleteLocationConfirmation};
    bool resetTimeOffsetConfirmation {DefaultResetTimeOffsetConfirmation};

    bool defaultMinimalUiButtonTextVisible {DefaultMinimalUiButtonTextVisible};
    bool defaultMinimalUiNonEssentialButtonVisible {DefaultMinimalUiNonEssentialButtonVisible};
    bool defaultMinimalUiReplaySpeedVisible {DefaultMinimalUiReplaySpeedVisible};

    QString importAircraftType;

    int previewInfoDialogCount {DefaultPreviewInfoDialogCount};

    static inline std::once_flag onceFlag;
    static inline std::unique_ptr<Settings> instance;

    static constexpr QUuid DefaultSkyConnectPluginUuid {};
    static constexpr bool DefaultBackupBeforeMigration {true};
    static constexpr bool DefaultWindowStayOnTop {false};
    static constexpr bool DefaultMinimalUi {false};
    static constexpr bool DefaultModuleSelectorVisible {true};
    static constexpr bool DefaultReplaySpeedVisible {true};
    static constexpr bool DefaultAbsoluteSeek {true};
    static constexpr double DefaultSeekIntervalSeconds {1.0};
    static constexpr double DefaultSeekIntervalPercent {0.5};
    static constexpr bool DefaultReplayLoop {false};
    static constexpr Replay::SpeedUnit DefaultReplaySpeedUnit {Replay::SpeedUnit::Absolute};
    static constexpr Replay::TimeMode DefaultReplayTimeMode {Replay::TimeMode::SimulationTime};

    // The T-45 Goshawk properly reacts to the CANOPY_OPEN simulation variable; so there is at least
    // one well-behaving aircraft (the Fiat "Gina" G-91 still needs this option set though)
    static constexpr bool DefaultRepeatCanopyOpen {false};
    // While technically the maximum simulation rate can be up to 128 (in MSFS) this may
    // greatly impact CPU performance; a good compromise seems to be a factor of 8
    // Also refer to: https://docs.flightsimulator.com/html/Programming_Tools/Programming_APIs.htm#SIMULATION%20RATE
    static constexpr int DefaultMaximumSimulationRate {8};    

    static constexpr bool DefaultDeleteFlightConfirmation {true};
    static constexpr bool DefaultDeleteAircraftConfirmation {true};
    static constexpr bool DefaultDeleteLocationConfirmation {true};
    static constexpr bool DefaultResetTimeOffsetConfirmation {true};

    static constexpr bool DefaultMinimalUiButtonTextVisible {false};
    static constexpr bool DefaultMinimalUiNonEssentialButtonVisible {false};
    static constexpr bool DefaultMinimalUiReplaySpeedVisible {false};

    static inline const QString DefaultImportAircraftType {};

    static constexpr int DefaultPreviewInfoDialogCount {3};
    // E.g. version 0.19 -> base count 190
    static constexpr int PreviewInfoDialogBase {190};
};

// PUBLIC

Settings &Settings::getInstance() noexcept
{
    std::call_once(SettingsPrivate::onceFlag, []() {
        SettingsPrivate::instance = std::unique_ptr<Settings>(new Settings());
    });
    return *SettingsPrivate::instance;
}

void Settings::destroyInstance() noexcept
{
    if (SettingsPrivate::instance != nullptr) {
        SettingsPrivate::instance.reset();
    }
}

// ********************
// Application Settings
// ********************

const Version &Settings::getVersion() const noexcept
{
    return d->version;
}

QString Settings::getLogbookPath() const noexcept
{
    return d->logbookPath;
}

void Settings::setLogbookPath(QString logbookPath) noexcept
{
    if (d->logbookPath != logbookPath) {
        d->logbookPath = std::move(logbookPath);
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

// **********************
// Common Replay Settings
// **********************

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

Replay::TimeMode Settings::getReplayTimeMode() const noexcept
{
    return d->replayTimeMode;
}

bool Settings::isReplayTimeModeEnabled() const noexcept
{
    return getReplayTimeMode() != Replay::TimeMode::None;
}

void Settings::setReplayTimeMode(Replay::TimeMode timeMode) noexcept
{
    if (d->replayTimeMode != timeMode) {
        d->replayTimeMode = timeMode;
        emit replayTimeModeChanged(d->replayTimeMode);
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

int Settings::getMaximumSimulationRate() const noexcept
{
    return d->maximumSimulationRate;
}

void Settings::setMaximumSimulationRate(int rate) noexcept
{
    if (d->maximumSimulationRate != rate) {
        d->maximumSimulationRate = rate;
        emit maximumSimulationRateChanged(d->maximumSimulationRate);
    }
}

// ***********************
// User Interface Settings
// ***********************

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

void Settings::setWindowGeometry(QByteArray geometry) noexcept
{
    d->windowGeometry = std::move(geometry);
}

QByteArray Settings::getWindowState() const noexcept
{
    return d->windowState;
}

void Settings::setWindowState(QByteArray state) noexcept
{
    d->windowState = std::move(state);
}

QString Settings::getStyleKey() const noexcept
{
    return d->styleKey;
}

void Settings::setStyleKey(QString styleKey) noexcept
{
    if (d->styleKey != styleKey) {
        d->styleKey = std::move(styleKey);
        emit styleKeyChanged(d->styleKey);
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

void Settings::setImportAircraftType(QString type) noexcept
{
    if (d->importAircraftType != type) {
        d->importAircraftType = std::move(type);
        emit changed();
    }
}

QString Settings::getExportPath() const noexcept
{
    return d->exportPath;
}

void Settings::setExportPath(QString exportPath)
{
    if (d->exportPath != exportPath) {
        d->exportPath = std::move(exportPath);
        emit exportPathChanged(d->exportPath);
    }
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
        d->settings.setValue("MaximumSimulationRate", d->maximumSimulationRate);
        d->settings.setValue("ReplayTimeMode", Enum::underly(d->replayTimeMode));
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        // UI styles
        d->settings.setValue("StyleKey", d->styleKey);

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
        auto enumValue = d->settings.value("ReplaySpeedUnit", Enum::underly(SettingsPrivate::DefaultReplaySpeedUnit)).toInt(&ok);
        d->replaySpeedUnit = ok && Enum::contains<Replay::SpeedUnit>(enumValue) ? static_cast<Replay::SpeedUnit>(enumValue) : SettingsPrivate::DefaultReplaySpeedUnit;
        enumValue = d->settings.value("ReplayTimeMode", Enum::underly(SettingsPrivate::DefaultReplayTimeMode)).toInt(&ok);
        d->replayTimeMode = ok && Enum::contains<Replay::TimeMode>(enumValue) ? static_cast<Replay::TimeMode>(enumValue) : SettingsPrivate::DefaultReplayTimeMode;
        d->repeatCanopyOpen = d->settings.value("RepeatCanopyOpen", SettingsPrivate::DefaultRepeatCanopyOpen).toBool();
        int maximumSimulationRateValue = d->settings.value("MaximumSimulationRate", SettingsPrivate::DefaultMaximumSimulationRate).toInt(&ok);
        if (ok) {
            d->maximumSimulationRate = maximumSimulationRateValue;
        } else {
#ifdef DEBUG
            qWarning() << "The maximum simulation rate in the settings could not be parsed, so setting value to default value:" << SettingsPrivate::DefaultMaximumSimulationRate;
#endif
            d->maximumSimulationRate = SettingsPrivate::DefaultMaximumSimulationRate;
        }
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        // UI styles
        d->styleKey = d->settings.value("StyleKey", DefaultStyleKey).toString();

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
    : d {std::make_unique<SettingsPrivate>()}
{
    restore();
    frenchConnection();
}

Settings::~Settings()
{
#ifdef DEBUG
    qDebug() << "Settings::~Settings: DELETED";
#endif
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
    connect(this, &Settings::replayTimeModeChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatCanopyChanged,
            this, &Settings::changed);
    connect(this, &Settings::maximumSimulationRateChanged,
            this, &Settings::changed);
    connect(this, &Settings::styleKeyChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiButtonTextVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiNonEssentialButtonVisibilityChanged,
            this, &Settings::changed);
    connect(this, &Settings::defaultMinimalUiReplaySpeedVisibilityChanged,
            this, &Settings::changed);
}
