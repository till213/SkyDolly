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

#include "SampleRate.h"
#include "Version.h"
#include "Settings.h"

class SettingsPrivate
{
public:
    QSettings settings;
    Version version;

    QString libraryPath;
    double recordSampleRateValue;
    bool windowStayOnTopEnabled;
    QString exportPath;
    QString defaultExportPath;
    bool absoluteSeek;
    double seekIntervalSeconds;
    double seekIntervalPercent;
    bool repeatFlapsHandleIndex;
    bool repeatCanopyOpen;

    int previewInfoDialogCount;

    static Settings *instance;
    static const QString DefaultDbPath;
    static constexpr double DefaultRecordSampleRate = SampleRate::toValue(SampleRate::SampleRate::Auto);
    static constexpr bool DefaultWindowStayOnTopEnabled = false;
    static constexpr bool DefaultAbsoluteSeek = true;
    static constexpr double DefaultSeekIntervalSeconds = 1.0;
    static constexpr double DefaultSeekIntervalPercent = 0.5;
    static constexpr double DefaultRepeatFlapsHandleIndex = false;
    // For now the default value is true, as no known aircraft exists where the canopy values would not
    // have to be repeated
    static constexpr double DefaultRepeatCanopyOpen = true;

    static constexpr int DefaultPreviewInfoDialogCount = 3;
    static constexpr int PreviewInfoDialogBase = 20;

    SettingsPrivate() noexcept
        : version(QCoreApplication::instance()->applicationVersion())
    {
        QStringList standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
        if (standardLocations.count() > 0) {
            defaultExportPath = standardLocations.first();
        } else {
            defaultExportPath = ".";
        }
    }

    ~SettingsPrivate() noexcept
    {}
};

const QString SettingsPrivate::DefaultDbPath = QString();

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

QString Settings::getLibraryPath() const noexcept
{
    return d->libraryPath;
}

void Settings::setLibraryPath(const QString &libraryPath) noexcept
{
    if (d->libraryPath != libraryPath) {
        d->libraryPath = libraryPath;
        emit libraryPathChanged(d->libraryPath);
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

SampleRate::SampleRate Settings::getRecordSampleRate() const noexcept
{
    return SampleRate::fromValue(d->recordSampleRateValue);
}

double Settings::getRecordSampleRateValue() const noexcept
{
    return d->recordSampleRateValue;
}

void Settings::setRecordSampleRate(SampleRate::SampleRate sampleRate) noexcept
{
    double sampleRateValue = SampleRate::toValue(sampleRate);
    if (d->recordSampleRateValue != sampleRateValue) {
        d->recordSampleRateValue = sampleRateValue;
        emit recordSampleRateChanged(sampleRate);
    }
}

bool Settings::isWindowStaysOnTopEnabled() const noexcept
{
    return d->windowStayOnTopEnabled;
}

void Settings::setWindowStaysOnTopEnabled(bool enable) noexcept
{
    if (d->windowStayOnTopEnabled != enable) {
        d->windowStayOnTopEnabled = enable;
        emit changed();
    }
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
    if (d->seekIntervalPercent!= percent) {
        d->seekIntervalPercent = percent;
        emit seekIntervalPercentChanged(d->seekIntervalPercent);
    }
}

bool Settings::isRepeatFlapsHandleIndexEnabled() const noexcept
{
    return d->repeatFlapsHandleIndex;
}

void Settings::setRepeatFlapsHandleIndexEnabled(bool enable) noexcept
{
    if (d->repeatFlapsHandleIndex!= enable) {
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
    if (d->repeatCanopyOpen!= enable) {
        d->repeatCanopyOpen = enable;
        emit repeatCanopyChanged(d->repeatCanopyOpen);
    }
}

int Settings::getPreviewInfoDialogCount() const noexcept
{
    return d->seekIntervalSeconds;
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
    d->settings.beginGroup("Library");
    {
        d->settings.setValue("DbPath", d->libraryPath);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Recording");
    {
        d->settings.setValue("RecordSampleRate", d->recordSampleRateValue);
    }
    d->settings.endGroup();    
    d->settings.beginGroup("Replay");
    {
        d->settings.setValue("AbsoluteSeek", d->absoluteSeek);
        d->settings.setValue("SeekIntervalSeconds", d->seekIntervalSeconds);
        d->settings.setValue("SeekIntervalPercent", d->seekIntervalPercent);
        d->settings.setValue("RepeatFlapsHandleIndex", d->repeatFlapsHandleIndex);
        d->settings.setValue("RepeatCanopyOpen", d->repeatCanopyOpen);
    }
    d->settings.endGroup();    
    d->settings.beginGroup("Window");
    {
        d->settings.setValue("WindowStaysOnTopEnabled", d->windowStayOnTopEnabled);
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

    bool ok;
    d->settings.beginGroup("Library");
    {
        d->libraryPath = d->settings.value("DbPath", SettingsPrivate::DefaultDbPath).toString();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Recording");
    {
        d->recordSampleRateValue = d->settings.value("RecordSampleRate", SettingsPrivate::DefaultRecordSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The record sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultRecordSampleRate);
            d->recordSampleRateValue = SettingsPrivate::DefaultRecordSampleRate;
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
        d->repeatFlapsHandleIndex = d->settings.value("RepeatFlapsHandleIndex", SettingsPrivate::DefaultRepeatFlapsHandleIndex).toBool();
        d->repeatCanopyOpen = d->settings.value("RepeatCanopyOpen", SettingsPrivate::DefaultRepeatCanopyOpen).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->windowStayOnTopEnabled = d->settings.value("WindowStaysOnTopEnabled", SettingsPrivate::DefaultWindowStayOnTopEnabled).toBool();
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
    restore();
    frenchConnection();
}

void Settings::frenchConnection() noexcept
{
    connect(this, &Settings::libraryPathChanged,
            this, &Settings::changed);
    connect(this, &Settings::recordSampleRateChanged,
            this, &Settings::changed);
    connect(this, &Settings::exportPathChanged,
            this, &Settings::changed);
    connect(this, &Settings::absoluteSeekEnabledChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalSecondsChanged,
            this, &Settings::changed);
    connect(this, &Settings::seekIntervalPercentChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatFlapsPositionChanged,
            this, &Settings::changed);
    connect(this, &Settings::repeatCanopyChanged,
            this, &Settings::changed);
}
