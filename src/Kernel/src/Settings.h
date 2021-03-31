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

    static Settings &getInstance() noexcept;
    static void destroyInstance() noexcept;

    SampleRate::SampleRate getRecordSampleRate() const noexcept;
    double getRecordSampleRateValue() const noexcept;
    void setRecordSampleRate(SampleRate::SampleRate sampleRate) noexcept;

    bool isWindowStaysOnTopEnabled() const noexcept;
    void setWindowStaysOnTopEnabled(bool enable) noexcept;

    QString getExportPath() const noexcept;
    void setExportPath(QString exportPath);

    bool isAbsoluteSeekEnabled() const noexcept;
    void setAbsoluteSeekEnabled(bool enable) noexcept;

    double getSeekIntervalSeconds() const noexcept;
    void setSeekIntervalSeconds(double seconds) noexcept;

    double getSeekIntervalPercent() const noexcept;
    void setSeekIntervalPercent(double percent) noexcept;

    QString getDbPath() const noexcept;
    void setDbPath(const QString &dbPath) noexcept;

    int getPreviewInfoDialogCount() const noexcept;
    void setPreviewInfoDialogCount(int count) noexcept;

public slots:
    /*!
     * Stores these Settings to a user configuration file.
     */
    void store() noexcept;

    /*!
     * Restores these Settings from a user configuration file. If no user
     * configuration is present the settings are set to default values.
     *
     * \sa #changed()
     */
    void restore() noexcept;

signals:
    /*!
     * Emitted when the record sample rate has changed.
     *
     * \sa changed()
     */
    void recordSampleRateChanged(SampleRate::SampleRate sampleRate);

    /*!
     * Emitted when the replay sample rate has changed.
     *
     * \sa changed()
     */
    void replaySampleRateChanged(SampleRate::SampleRate sampleRate);

    /*!
     * Emitted when the export path has changed.
     *
     * \sa changed()
     */
    void exportPathChanged(const QString &exportPath);

    /*!
     * Emitted when the absolute/relative seek setting has changed.
     *
     * \sa changed()
     */
    void absoluteSeekEnabledChanged(bool enabled);

    /*!
     * Emitted when the seek interval in secondshas changed.
     *
     * \sa changed()
     */
    void seekIntervalSecondsChanged(double seconds);

    /*!
     * Emitted when the seek interval in percent has changed.
     *
     * \sa changed()
     */
    void seekIntervalPercentChanged(double percent);

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
