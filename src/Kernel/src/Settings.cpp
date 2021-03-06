/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QString>
#include <QUuid>
#include <QByteArray>

#include "Enum.h"
#include "Const.h"
#include "SampleRate.h"
#include "Version.h"
#include "Settings.h"

class SettingsPrivate
{
public:
    QSettings settings;
    Version version;

    QString logbookPath;
    QUuid skyConnectPluginUuid;
    double recordingSampleRateValue;
    bool windowStayOnTop;
    bool minimalUi;
    bool moduleSelectorVisible;
    QByteArray windowGeometry;
    QByteArray windowState;
    QString exportPath;
    QString defaultExportPath;
    QString defaultLogbookPath;
    bool absoluteSeek;
    double seekIntervalSeconds;
    double seekIntervalPercent;
    Replay::SpeedUnit replaySpeedUnit;
    bool repeatFlapsHandleIndex;
    bool repeatCanopyOpen;

    bool deleteFlightConfirmation;
    bool deleteAircraftConfirmation;

    int previewInfoDialogCount;

    static Settings *instance;
    static constexpr QUuid DefaultSkyConnectPluginUuid = QUuid();
    static constexpr double DefaultRecordingSampleRate = SampleRate::toValue(SampleRate::SampleRate::Auto);
    static constexpr bool DefaultWindowStayOnTop = false;
    static constexpr bool DefaultMinimalUi = false;
    static constexpr bool DefaultModuleSelectorVisible = true;
    static constexpr bool DefaultAbsoluteSeek = true;
    static constexpr double DefaultSeekIntervalSeconds = 1.0;
    static constexpr double DefaultSeekIntervalPercent = 0.5;
    static constexpr Replay::SpeedUnit DefaultReplaySpeedUnit = Replay::SpeedUnit::Absolute;
    static constexpr double DefaultRepeatFlapsHandleIndex = false;
    // For now the default value is true, as no known aircraft exists where the canopy values would not
    // have to be repeated
    static constexpr double DefaultRepeatCanopyOpen = true;
    static constexpr bool DefaultDeleteFlightConfirmation = true;
    static constexpr bool DefaultDeleteAircraftConfirmation = true;

    static constexpr int DefaultPreviewInfoDialogCount = 3;
    static constexpr int PreviewInfoDialogBase = 40;

    SettingsPrivate() noexcept
        : version(QCoreApplication::instance()->applicationVersion())
    {
        QStringList standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
        if (standardLocations.count() > 0) {
            defaultExportPath = standardLocations.first();
            defaultLogbookPath = standardLocations.first() + "/" + Version::getApplicationName() + "/" + Version::getApplicationName() + Const::LogbookExtension;
        } else {
            defaultExportPath = ".";
        }
    }

    ~SettingsPrivate() noexcept
    {}
};

Settings *SettingsPrivate::instance = nullptr;

// PUBLIC

Settings &Settings::getInstance() noexcept
{
    if (SettingsPrivate::instance == nullptr) {
        SettingsPrivate::instance = new Settings();
    }
    return *SettingsPrivate::instance;
}

void Settings::destroyInstance() noexcept
{
    if (SettingsPrivate::instance != nullptr) {
        delete SettingsPrivate::instance;
        SettingsPrivate::instance = nullptr;
    }
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

void Settings::setExportPath(QString exportPath)
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

void Settings::setModuleSelectorVisible(bool enable) noexcept
{
    if (d->moduleSelectorVisible != enable) {
        d->moduleSelectorVisible = enable;
        emit moduleSelectorVisibilityChanged(enable);
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

bool Settings::isRepeatFlapsHandleIndexEnabled() const noexcept
{
    return d->repeatFlapsHandleIndex;
}

void Settings::setRepeatFlapsHandleIndexEnabled(bool enable) noexcept
{
    if (d->repeatFlapsHandleIndex != enable) {
        d->repeatFlapsHandleIndex = enable;
        emit repeatFlapsPositionChanged(d->repeatFlapsHandleIndex);
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

// PUBLIC SLOTS

void Settings::store() noexcept
{
    d->settings.setValue("Version", d->version.toString());
    d->settings.beginGroup("Logbook");
    {
        d->settings.setValue("Path", d->logbookPath);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Plugins");
    {
        d->settings.setValue("SkyConnectPluginUuid", d->skyConnectPluginUuid);
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
        d->settings.setValue("ReplaySpeedUnit", Enum::toUnderlyingType(d->replaySpeedUnit));
        d->settings.setValue("RepeatFlapsHandleIndex", d->repeatFlapsHandleIndex);
        d->settings.setValue("RepeatCanopyOpen", d->repeatCanopyOpen);
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        d->settings.setValue("DeleteFlightConfirmation", d->deleteFlightConfirmation);
        d->settings.setValue("DeleteAircraftConfirmation", d->deleteAircraftConfirmation);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->settings.setValue("WindowStaysOnTop", d->windowStayOnTop);
        d->settings.setValue("MinimalUi", d->minimalUi);
        d->settings.setValue("ModuleSelectorVisible", d->moduleSelectorVisible);
        d->settings.setValue("Geometry", d->windowGeometry);
        d->settings.setValue("State", d->windowState);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Paths");
    {
        d->settings.setValue("ExportPath", d->exportPath);
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
    QString version;
    version = d->settings.value("Version", getVersion().toString()).toString();
    Version settingsVersion(version);
    if (settingsVersion < getVersion()) {
#ifdef DEBUG
        qDebug("Settings::restore: app version: %s, settings version: %s, conversion might be necessary!",
               qPrintable(getVersion().toString()), qPrintable(settingsVersion.toString()));
        /*!\todo Settings conversion as necessary */
#endif
    }

#ifdef DEBUG
    qDebug("Settings::restore: RESTORE: app name %s, organisation name: %s", qPrintable(d->settings.applicationName()), qPrintable(d->settings.organizationName()));
#endif

    bool ok;
    d->settings.beginGroup("Logbook");
    {
        d->logbookPath = d->settings.value("Path", d->defaultLogbookPath).toString();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Plugins");
    {
        d->skyConnectPluginUuid = d->settings.value("SkyConnectPluginUuid", d->DefaultSkyConnectPluginUuid).toUuid();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Recording");
    {
        d->recordingSampleRateValue = d->settings.value("RecordingSampleRate", SettingsPrivate::DefaultRecordingSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The recording sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultRecordingSampleRate);
            d->recordingSampleRateValue = SettingsPrivate::DefaultRecordingSampleRate;
        }
    }
    d->settings.endGroup();    
    d->settings.beginGroup("Replay");
    {
        d->absoluteSeek = d->settings.value("AbsoluteSeek", SettingsPrivate::DefaultAbsoluteSeek).toBool();
        d->seekIntervalSeconds = d->settings.value("SeekIntervalSeconds", SettingsPrivate::DefaultSeekIntervalSeconds).toDouble(&ok);
        if (!ok) {
            qWarning("The seek interval [seconds] in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultSeekIntervalSeconds);
            d->seekIntervalSeconds = SettingsPrivate::DefaultSeekIntervalSeconds;
        }
        d->seekIntervalPercent = d->settings.value("SeekIntervalPercent", SettingsPrivate::DefaultSeekIntervalPercent).toDouble(&ok);
        if (!ok) {
            qWarning("The seek interval [percent] in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultSeekIntervalPercent);
            d->seekIntervalPercent = SettingsPrivate::DefaultSeekIntervalPercent;
        }
        int replaySpeedUnitValue = d->settings.value("ReplaySpeedUnit", Enum::toUnderlyingType(SettingsPrivate::DefaultReplaySpeedUnit)).toInt(&ok);
        if (ok) {
            d->replaySpeedUnit = static_cast<Replay::SpeedUnit>(replaySpeedUnitValue);
        } else {
            qWarning("The replay speed unit in the settings coul dnot be parsed, so setting value to default value %d", Enum::toUnderlyingType(SettingsPrivate::DefaultReplaySpeedUnit));
            d->replaySpeedUnit = SettingsPrivate::DefaultReplaySpeedUnit;
        }
        d->repeatFlapsHandleIndex = d->settings.value("RepeatFlapsHandleIndex", SettingsPrivate::DefaultRepeatFlapsHandleIndex).toBool();
        d->repeatCanopyOpen = d->settings.value("RepeatCanopyOpen", SettingsPrivate::DefaultRepeatCanopyOpen).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("UI");
    {
        d->deleteFlightConfirmation = d->settings.value("DeleteFlightConfirmation", SettingsPrivate::DefaultDeleteFlightConfirmation).toBool();
        d->deleteAircraftConfirmation = d->settings.value("DeleteAircraftConfirmation", SettingsPrivate::DefaultDeleteAircraftConfirmation).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->windowStayOnTop = d->settings.value("WindowStaysOnTop", SettingsPrivate::DefaultWindowStayOnTop).toBool();
        d->minimalUi = d->settings.value("MinimalUi", SettingsPrivate::DefaultMinimalUi).toBool();
        d->moduleSelectorVisible = d->settings.value("ModuleSelectorVisible", SettingsPrivate::DefaultModuleSelectorVisible).toBool();
        d->windowGeometry = d->settings.value("Geometry").toByteArray();
        d->windowState = d->settings.value("State").toByteArray();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Paths");
    {
        d->exportPath = d->settings.value("ExportPath", d->defaultExportPath).toString();
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

// PROTECTED

Settings::~Settings()
{
#ifdef DEBUG
    qDebug("Settings::~Settings: DELETED");
#endif
    store();
}

const Version &Settings::getVersion() const
{
    return d->version;
}

// PRIVATE

Settings::Settings() noexcept
    : d(std::make_unique<SettingsPrivate>())
{
#ifdef DEBUG
    qDebug("Settings::Settings: CREATED");
#endif
    restore();
    frenchConnection();
}

void Settings::frenchConnection() noexcept
{
    connect(this, &Settings::logbookPathChanged,
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
    connect(this, &Settings::exportPathChanged,
            this, &Settings::changed);
    connect(this, &Settings::absoluteSeekEnabledChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalSecondsChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalPercentChanged,
            this, &Settings::changed);
    connect(this, &Settings::replaySpeedUnitChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatFlapsPositionChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatCanopyChanged,
            this, &Settings::changed);
}
