#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

#include "SampleRate.h"
#include "KernelLib.h"

class SettingsPrivate;
class Version;

/*!
 * The application settings. These settings are persisted to user configuration
 * files.
 *
 * Implementation note:
 *
 * - \b Windows: INI format
 * - \b Mac: Native format (CFPreferences API)
 * - \b Linux: INI format
 */
class KERNEL_API Settings : public QObject
{
    Q_OBJECT
public:

    static Settings &getInstance();
    static void destroyInstance();

    SampleRate::SampleRate getRecordSampleRate() const;
    double getRecordSampleRateValue() const;
    void setRecordSampleRate(SampleRate::SampleRate sampleRate);

    SampleRate::SampleRate getPlaybackSampleRate() const;
    double getPlaybackSampleRateValue() const;
    void setPlaybackSampleRate(SampleRate::SampleRate sampleRate);

public slots:
    /*!
     * Stores these Settings to a user configuration file.
     */
    void store();

    /*!
     * Restores these Settings from a user configuration file. If no user
     * configuration is present the settings are set to default values.
     *
     * \sa #changed()
     */
    void restore();

signals:
    /*!
     * Emitted when the record sample rate has changed.
     *
     * \sa changed()
     */
    void recordSampleRateChanged(double sampleRateValue);

    /*!
     * Emitted when the playback sample rate has changed.
     *
     * \sa changed()
     */
    void playbackSampleRateChanged(double sampleRateValue);

    /*!
     * Emitted when any setting has changed.
     */
    void changed();

protected:
    virtual ~Settings();
    const Version &getVersion() const;

private:
    Q_DISABLE_COPY(Settings)
    SettingsPrivate *d;

    Settings();

    void frenchConnection();
};

#endif // SETTINGS_H
