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
#ifndef GPXIMPORTSETTINGS_H
#define GPXIMPORTSETTINGS_H

#include <memory>

#include <QObject>
#include <QString>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../ImportPluginBaseSettings.h"

class GpxImportSettingsPrivate;

class GpxImportSettings : public ImportPluginBaseSettings
{
    Q_OBJECT
public:
    enum struct GPXElement {
        Waypoint = 0,
        Route = 1,
        Track = 2,
    };

    GpxImportSettings() noexcept;
    virtual ~GpxImportSettings() noexcept;

    GPXElement getWaypointSelection() const noexcept;
    void setWaypointSelection(GPXElement selection) noexcept;

    GPXElement getPositionSelection() const noexcept;
    void setPositionSelection(GPXElement selection) noexcept;

    int getDefaultAltitude() const noexcept;
    void setDefaultAltitude(int altitude) noexcept;

    int getDefaultVelocity() const noexcept;
    void setDefaultVelocity(int velocity) noexcept;

    bool isConvertAltitudeEnabled() const noexcept;
    void setConvertAltitudeEnabled(bool enable) noexcept;

signals:
    /*!
     * Emitted whenever the extended settings have changed.
     */
    void extendedSettingsChanged();

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    virtual void restoreDefaultsExtn() noexcept override;

private:
    std::unique_ptr<GpxImportSettingsPrivate> d;
};

#endif // GPXIMPORTSETTINGS_H
