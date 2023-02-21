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
#ifndef LOCATIONSETTINGS_H
#define LOCATIONSETTINGS_H

#include <memory>

#include <QObject>

#include <Kernel/Settings.h>
#include <Persistence/LocationSelector.h>
#include <PluginManager/Module/ModuleBaseSettings.h>

struct LocationSettingsPrivate;

class LocationSettings : public ModuleBaseSettings
{
    Q_OBJECT
public:
    LocationSettings() noexcept;
    LocationSettings(const LocationSettings &rhs) = delete;
    LocationSettings(LocationSettings &&rhs) = delete;
    LocationSettings &operator=(const LocationSettings &rhs) = delete;
    LocationSettings &operator=(LocationSettings &&rhs) = delete;
    ~LocationSettings() override;

    LocationSelector::TypeSelection getTypeSelection() const noexcept;
    void setTypeSelection(LocationSelector::TypeSelection typeSelection) noexcept;

    std::int64_t getCategoryId() const noexcept;
    void setCategoryId(std::int64_t id) noexcept;

    std::int64_t getCountryId() const noexcept;
    void setCountryId(std::int64_t id) noexcept;

    const QString &getSearchText() const noexcept;
    void setSearchText(QString searchtext) noexcept;

    bool showUserLocations() const noexcept;
    bool hasSelectors() const noexcept;
    const LocationSelector &getLocationSelector() const noexcept;

    double getAltitude() const noexcept;
    void setAltitdue(double altitude) noexcept;

    int getIndicatedAirspeed() const noexcept;
    void setIndicatedAirspeed(int airspeed);

    bool isOnGround() const noexcept;
    void setOnGround(bool enable) noexcept;

    void clearLocationSelector() noexcept;

    /*!
     * Returns the saved location table state.
     *
     * \return the location table state; a \e null QByteArray if not saved before
     */
    QByteArray getLocationTableState() const;

    /*!
     * Stores the location table state.
     *
     * \param state
     *        the location table state encoded in the QByteAarray
     */
    void setLocationTableState(QByteArray state) noexcept;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
    const std::unique_ptr<LocationSettingsPrivate> d;
};

#endif // LOCATIONSETTINGS_H
