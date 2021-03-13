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

    int previewInfoDialogCount;

    static Settings *instance;
    static constexpr double DefaultRecordSampleRate = SampleRate::toValue(SampleRate::SampleRate::Hz10);
    static constexpr double DefaultPlaybackSampleRate = SampleRate::toValue(SampleRate::SampleRate::Hz60);
    static constexpr bool DefaultWindowStayOnTopEnabled = false;
    static constexpr int DefaultPreviewInfoDialogCount = 3;

    SettingsPrivate()
        : version(QCoreApplication::instance()->applicationVersion())
    {}

    ~SettingsPrivate()
    {}
};

Settings *SettingsPrivate::instance = nullptr;

// PUBLIC

Settings &Settings::getInstance()
{
    if (SettingsPrivate::instance == nullptr) {
        SettingsPrivate::instance = new Settings();
    }
    return *SettingsPrivate::instance;
}

void Settings::destroyInstance()
{
    if (SettingsPrivate::instance != nullptr) {
        delete SettingsPrivate::instance;
        SettingsPrivate::instance = nullptr;
    }
}

SampleRate::SampleRate Settings::getRecordSampleRate() const
{
    return SampleRate::fromValue(d->recordSampleRateValue);
}

double Settings::getRecordSampleRateValue() const
{
    return d->recordSampleRateValue;
}

void Settings::setRecordSampleRate(SampleRate::SampleRate sampleRate)
{
    double sampleRateValue = SampleRate::toValue(sampleRate);
    if (d->recordSampleRateValue != sampleRateValue) {
        d->recordSampleRateValue = sampleRateValue;
        emit recordSampleRateChanged(sampleRate);
    }
}

SampleRate::SampleRate Settings::getPlaybackSampleRate() const
{
    return SampleRate::fromValue(d->playbackSampleRateValue);
}

double Settings::getPlaybackSampleRateValue() const
{
    return d->playbackSampleRateValue;
}

void Settings::setPlaybackSampleRate(SampleRate::SampleRate sampleRate)
{
    double sampleRateValue = SampleRate::toValue(sampleRate);
    if (d->playbackSampleRateValue != sampleRateValue) {
        d->playbackSampleRateValue = SampleRate::toValue(sampleRate);
        emit playbackSampleRateChanged(sampleRate);
    }
}

bool Settings::isWindowStaysOnTopEnabled() const
{
    return d->windowStayOnTopEnabled;
}

void Settings::setWindowStaysOnTopEnabled(bool enable)
{
    if (d->windowStayOnTopEnabled != enable) {
        d->windowStayOnTopEnabled = enable;
        emit changed();
    }
}

int Settings::getPreviewInfoDialogCount() const
{
    return d->previewInfoDialogCount;
}

void Settings::setPreviewInfoDialogCount(int count)
{
    if (d->previewInfoDialogCount != count) {
        d->previewInfoDialogCount = count;
        emit changed();
    }
}

// PUBLIC SLOTS

void Settings::store()
{
    d->settings.setValue("Version", d->version.toString());
    d->settings.beginGroup("Sampling");
    {
        d->settings.setValue("RecordSampleRate", d->recordSampleRateValue);
        d->settings.setValue("PlaybackSampleRate", d->playbackSampleRateValue);
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

void Settings::restore()
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
        d->playbackSampleRateValue = d->settings.value("PlaybackSampleRate", SettingsPrivate::DefaultPlaybackSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The playback sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultPlaybackSampleRate);
            d->playbackSampleRateValue = SettingsPrivate::DefaultPlaybackSampleRate;
        }
    }
    d->settings.endGroup();
    d->settings.beginGroup("Window");
    {
        d->windowStayOnTopEnabled = d->settings.value("WindowStaysOnTopEnabled", SettingsPrivate::DefaultWindowStayOnTopEnabled).toBool();
    }
    d->settings.endGroup();
    d->settings.beginGroup("_Preview");
    {
        d->previewInfoDialogCount = d->settings.value("PreviewInfoDialogCount", SettingsPrivate::DefaultPreviewInfoDialogCount).toInt(&ok);
        if (!ok) {
            d->previewInfoDialogCount = SettingsPrivate::DefaultPreviewInfoDialogCount;
        }
    }
    d->settings.endGroup();
}

// PROTECTED

Settings::~Settings()
{
    store();
    delete d;
}

const Version &Settings::getVersion() const
{
    return d->version;
}

// PRIVATE

Settings::Settings()
    : d(new SettingsPrivate())
{
    restore();
    frenchConnection();
}

void Settings::frenchConnection()
{
    connect(this, &Settings::recordSampleRateChanged,
            this, &Settings::changed);
    connect(this, &Settings::playbackSampleRateChanged,
            this, &Settings::changed);
}
