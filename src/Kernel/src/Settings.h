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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <memory>

#include <QObject>
#include <QSettings>

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

    static Settings &getInstance();
    static void destroyInstance();

    SampleRate::SampleRate getRecordSampleRate() const;
    double getRecordSampleRateValue() const;
    void setRecordSampleRate(SampleRate::SampleRate sampleRate);

    SampleRate::SampleRate getPlaybackSampleRate() const;
    double getPlaybackSampleRateValue() const;
    void setPlaybackSampleRate(SampleRate::SampleRate sampleRate);

    bool isWindowStaysOnTopEnabled() const;
    void setWindowStaysOnTopEnabled(bool enable);

    int getPreviewInfoDialogCount() const;
    void setPreviewInfoDialogCount(int count);

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
    void recordSampleRateChanged(SampleRate::SampleRate sampleRate);

    /*!
     * Emitted when the playback sample rate has changed.
     *
     * \sa changed()
     */
    void playbackSampleRateChanged(SampleRate::SampleRate sampleRate);

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

    Settings();

    void frenchConnection();
};

#endif // SETTINGS_H
