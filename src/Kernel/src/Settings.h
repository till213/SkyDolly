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

    /*!
     * Returns the singleton Settings instance.
     *
     * \return a reference to the singleton
     */
    static Settings &getInstance() noexcept;

    /*!
     * Destroys the singleton instance. To be called when the application is just
     * about to terminate. The settings are persisted just before destruction
     * as well, in configuration files (or the registry).
     */
    static void destroyInstance() noexcept;

    QString getLibraryPath() const noexcept;
    void setLibraryPath(const QString &libraryPath) noexcept;

    /*!
     * Returns the recording sample rate enumeration value.
     *
     * \return the SampleRate#SampleRate enumeration value.
     */
    SampleRate::SampleRate getRecordSampleRate() const noexcept;

    /*!
     * Returns the recording sample rate in Hz.
     *
     * \return the recording sample rate in Hz
     */
    double getRecordSampleRateValue() const noexcept;

    /*!
     * Sets the recording sample rate.
     *
     * \param sampleRate
     *        the SampleRate#SampleRate enumeration value
     * \sa recordSampleRateChanged()
     */
    void setRecordSampleRate(SampleRate::SampleRate sampleRate) noexcept;

    /*!
     * Returns whether the \e stay \e on \e top option is enabled.
     *
     * \return \c true if the application window is kept in the foreground;
     *         \c false else
     */
    bool isWindowStaysOnTopEnabled() const noexcept;

    /*!
     * Sets the \e stay \e on \e top option.
     *
     * \param enable
     *        \c true in order to keep the application windoww in the foreground;
     *        \c false to enable default window behaviour
     */
    void setWindowStaysOnTopEnabled(bool enable) noexcept;

    /*!
     * Returns the path of the directory which was last accessed during export or import.
     *
     * \return the path of the last export / import directory
     */
    QString getExportPath() const noexcept;

    /*!
     * Sets the path of the directory which was last accessed during export or import.
     *
     * \param exportPath
     *        the path of the last export / import directory
     * \sa exportPathChanged()
     */
    void setExportPath(QString exportPath);

    /*!
     * Returns whether the fast-forward / backward interval is an absolute value (in milliseconds).
     * \return \c true if the seek interval is an absolute value; \c false if the interval is
     *         relative (in percent of the duration)
     */
    bool isAbsoluteSeekEnabled() const noexcept;

    /*!
     * Sets whether the fast-forward / backward interval is an absolute value (in milliseconds).
     *
     * \param enable
     *        set to \c true in order to set the seek interval as an absolute value; \c false to
     *        set the seek interval as relative value (in percent of the duration)
     * \sa absoluteSeekEnabledChanged()
     */
    void setAbsoluteSeekEnabled(bool enable) noexcept;

    /*!
     * Returns the absolute seek interval.
     *
     * \return the absolute seek interval in seconds
     */
    double getSeekIntervalSeconds() const noexcept;

    /*!
     * \Sets the absolute seek interval.
     *
     * \param seconds
     *        the absolute seek interval in seconds
     * \sa seekIntervalSecondsChanged()
     */
    void setSeekIntervalSeconds(double seconds) noexcept;

    /*!
     * Returns the relative seek interval.
     *
     * \return the relative seek interval in percent of the duration
     */
    double getSeekIntervalPercent() const noexcept;

    /*!
     * Sets the relative seek interval in percent.
     *
     * \param percent
     *        the seek interval in percent of the duration
     * \sa seekIntervalPercentChanged
     */
    void setSeekIntervalPercent(double percent) noexcept;

    /*!
     * Returns whether the FLAPS HANDLE INDEX simulation variable
     * is repeatedly sent or not.
     *
     * \return \c true if the FLAPS HANDLE INDEX value is repeatedly
     *         sent when its value is greater zero; \c false if
     *         the value is only to be sent when changed
     */
    bool isRepeatFlapsHandleIndexEnabled() const noexcept;

    /*!
     * Enables the value repeat for the FLAPS HANDLE INDEX simulation
     * variable.
     *
     * \param enable
     *        set to \c true in order to enable value repeat;
     *        \c false else
     */
    void setRepeatFlapsHandleIndexEnabled(bool enable) noexcept;

    /*!
     * Returns whether the CANOPY OPEN simulation variable
     * is repeatedly sent or not.
     *
     * \return \c true if the CANOPY OPEN value is repeatedly
     *         sent when its value is greater zero; \c false if
     *         the value is only to be sent when changed
     */
    bool isRepeatCanopyOpenEnabled() const noexcept;

    /*!
     * Enables the value repeat for the CANOPY OPEN simulation
     * variable.
     *
     * \param enable
     *        set to \c true in order to enable value repeat;
     *        \c false else
     */
    void setRepeatCanopyOpenEnabled(bool enable) noexcept;

    /*!
     * Returns the count of how many times the "preview" dialog is still
     * to be shown.
     *
     * \obsolete
     */
    int getPreviewInfoDialogCount() const noexcept;

    /*!
     * Sets the count of how many times the "preview" dialog is still
     * to be shown.
     *
     * \obsolete
     */
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
     * Emitted when the library path has changed.
     *
     * \sa changed()
     */
    void libraryPathChanged(const QString &libraryPath);

    /*!
     * Emitted when the record sample rate has changed.
     *
     * \sa changed()
     */
    void recordSampleRateChanged(SampleRate::SampleRate sampleRate);

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
     * Emitted when the repeat flaps position has changed.
     *
     * \sa changed()
     */
    void repeatFlapsPositionChanged(bool enable);

    /*!
     * Emitted when the repeat canopy has changed.
     *
     * \sa changed()
     */
    void repeatCanopyChanged(bool enable);

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
