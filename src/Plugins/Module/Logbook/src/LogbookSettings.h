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
#ifndef LOGBOOKSETTINGS_H
#define LOGBOOKSETTINGS_H

#include <memory>

#include <QObject>

#include <Kernel/Settings.h>
#include <PluginManager/Module/ModuleBaseSettings.h>

struct LogbookSettingsPrivate;

class LogbookSettings : public ModuleBaseSettings
{
    Q_OBJECT
public:
    LogbookSettings() noexcept;
    LogbookSettings(const LogbookSettings &rhs) = delete;
    LogbookSettings(LogbookSettings &&rhs) = delete;
    LogbookSettings &operator=(const LogbookSettings &rhs) = delete;
    LogbookSettings &operator=(LogbookSettings &&rhs) = delete;
    ~LogbookSettings() override;

    /*!
     * Returns the saved logbook table state.
     *
     * \return the logbook table state; a \e null QByteArray if not saved before
     */
    QByteArray getLogbookTableState() const;

    /*!
     * Stores the logbook table state.
     *
     * \param state
     *        the logbook table state encoded in the QByteAarray
     */
    void setLogbookTableState(QByteArray state) noexcept;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
    const std::unique_ptr<LogbookSettingsPrivate> d;
};

#endif // LOGBOOKSETTINGS_H
