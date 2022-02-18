/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef KMLIMPORTSETTINGS_H
#define KMLIMPORTSETTINGS_H

#include <QObject>
#include <QString>

#include "../../../../../Kernel/src/Settings.h"

class KMLImportSettings : public QObject
{
    Q_OBJECT
public:
    /*!
     * KML format (flavour).
     */
    enum struct Format {
        FlightAware = 0,
        FlightRadar24 = 1,
    };

    KMLImportSettings() noexcept;

    Format m_format;

    Settings::PluginSettings getSettings() const noexcept;
    Settings::KeysWithDefaults getKeysWithDefault() const noexcept;
    void setSettings(Settings::ValuesByKey) noexcept;
    void restoreDefaults() noexcept;

signals:
    void defaultsRestored();

private:
    void initSettings() noexcept;

    static constexpr Format DefaultFormat = Format::FlightAware;
};

#endif // KMLIMPORTSETTINGS_H
