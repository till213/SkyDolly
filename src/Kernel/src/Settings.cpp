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

    double recordSampleRateValue;
    double playbackSampleRateValue;
    bool windowStayOnTopEnabled;
    QString dbPath;

    int previewInfoDialogCount;

    static Settings *instance;
    static constexpr double DefaultRecordSampleRate = SampleRate::toValue(SampleRate::SampleRate::Auto);
    static constexpr bool DefaultWindowStayOnTopEnabled = false;
    static constexpr int DefaultPreviewInfoDialogCount = 3;
    static const QString DefaultDbPath;
    static constexpr int PreviewInfoDialogBase = 10;

    SettingsPrivate() noexcept
        : version(QCoreApplication::instance()->applicationVersion())
    {}

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

QString Settings::getDbPath() const noexcept
{
    return d->dbPath;
}

void Settings::setDbPath(const QString &dbPath) noexcept
{
    if (d->dbPath != dbPath) {
        d->dbPath = dbPath;
        emit changed();
    }
}

// PUBLIC SLOTS

void Settings::store() noexcept
{
    d->settings.setValue("Version", d->version.toString());
    d->settings.beginGroup("Sampling");
    {
        d->settings.setValue("RecordSampleRate", d->recordSampleRateValue);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Library");
    {
        d->settings.setValue("DbPath", d->dbPath);
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->settings.setValue("WindowStaysOnTopEnabled", d->windowStayOnTopEnabled);
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
    d->settings.beginGroup("Sampling");
    {
        d->recordSampleRateValue = d->settings.value("RecordSampleRate", SettingsPrivate::DefaultRecordSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The record sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultRecordSampleRate);
            d->recordSampleRateValue = SettingsPrivate::DefaultRecordSampleRate;
        }
    }
    d->settings.beginGroup("Library");
    {
        d->dbPath = d->settings.value("DbPath", SettingsPrivate::DefaultDbPath).toString();
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->windowStayOnTopEnabled = d->settings.value("WindowStaysOnTopEnabled", SettingsPrivate::DefaultWindowStayOnTopEnabled).toBool();
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
    connect(this, &Settings::recordSampleRateChanged,
            this, &Settings::changed);
    connect(this, &Settings::playbackSampleRateChanged,
            this, &Settings::changed);
}
