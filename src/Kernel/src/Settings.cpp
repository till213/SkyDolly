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

    static Settings *instance;
    static constexpr double DefaultRecordSampleRate = SampleRate::toValue(SampleRate::SampleRate::Hz10);
    static constexpr double DefaultPlaybackSampleRate = SampleRate::toValue(SampleRate::SampleRate::Hz60);

    SettingsPrivate()
        : version(QCoreApplication::instance()->applicationVersion())
    {}

    ~SettingsPrivate()
    {}

    const Version &getVersion() const {
        return version;
    }
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
        emit recordSampleRateChanged(d->recordSampleRateValue);
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
        emit playbackSampleRateChanged(d->playbackSampleRateValue);
    }
}

// PUBLIC SLOTS

void Settings::store()
{
    d->settings.setValue("version", d->version.toString());
    d->settings.beginGroup("sampling");
    {
        d->settings.setValue("recordSampleRate", d->recordSampleRateValue);
        d->settings.setValue("playbackSampleRate", d->playbackSampleRateValue);
    }
    d->settings.endGroup();
}

void Settings::restore()
{
    QString version;
    version = d->settings.value("version", getVersion().toString()).toString();
    Version settingsVersion(version);
    if (settingsVersion < getVersion()) {
#ifdef DEBUG
        qDebug("Settings::restore: app version: %s, settings version: %s, conversion might be necessary!",
               qPrintable(getVersion().toString()), qPrintable(settingsVersion.toString()));
        /*!\todo Settings conversion as necessary */
#endif
    }

    bool ok;
    d->settings.beginGroup("sampling");
    {
        d->recordSampleRateValue = d->settings.value("recordSampleRate", SettingsPrivate::DefaultRecordSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The record sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultRecordSampleRate);
            d->recordSampleRateValue = SettingsPrivate::DefaultRecordSampleRate;
        }
        d->playbackSampleRateValue = d->settings.value("playbackSampleRate", SettingsPrivate::DefaultPlaybackSampleRate).toDouble(&ok);
        if (!ok) {
            qWarning("The playback sample rate in the settings could not be parsed, so setting value to default value %f", SettingsPrivate::DefaultPlaybackSampleRate);
            d->playbackSampleRateValue = SettingsPrivate::DefaultPlaybackSampleRate;
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
