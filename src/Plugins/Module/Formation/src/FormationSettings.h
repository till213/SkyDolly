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
#ifndef FORMATIONSETTINGS_H
#define FORMATIONSETTINGS_H

#include <memory>

#include <QObject>
#include <QByteArray>

#include <Kernel/Settings.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "Formation.h"

struct FormationSettingsPrivate;

class FormationSettings : public ModuleBaseSettings
{
    Q_OBJECT
public:
    FormationSettings() noexcept;
    FormationSettings(const FormationSettings &rhs) = delete;
    FormationSettings(FormationSettings &&rhs) = delete;
    FormationSettings &operator=(const FormationSettings &rhs) = delete;
    FormationSettings &operator=(FormationSettings &&rhs) = delete;
    ~FormationSettings() override;

    int getBearing() const noexcept;
    void setBearing(int bearing) noexcept;

    Formation::HorizontalDistance getHorizontalDistance() const noexcept;
    void setHorizontalDistance(Formation::HorizontalDistance horizontalDistance) noexcept;

    Formation::VerticalDistance getVerticalDistance() const noexcept;
    void setVerticalDistance(Formation::VerticalDistance verticalDistance) noexcept;

    /*!
     * Returns whether the the new user aircraft should be placed at the calculated position
     * relative to the current reference aircraft (user aircraft) in the formation, e.g.
     * when starting recording, changing the reference (user) aircraft in the formation or
     * changing the bearing or distance.
     *
     * \return \c true if the aircraft should be placed at its calculated position;
     *         \c false if the aircraft should remain at its current position
     */
    bool isRelativePositionPlacementEnabled() const noexcept;

    /*!
     * Sets whether the the aircraft should be placed at the calculated relative position
     * relative to the current reference aircraft (user aircraft) in the formation.
     *
     * \param enable
     *        \c true if the aircraft should be placed at its calculated relative position;
     *        \c false if the aircraft should remain at its current position
     * \sa relativePositionPlacementChanged
     */
    void setRelativePositionPlacementEnabled(bool enable) noexcept;

    SkyConnectIntf::ReplayMode getReplayMode() const noexcept;
    void setReplayMode(SkyConnectIntf::ReplayMode replayMode) noexcept;

    /*!
     * Returns the saved formation aircraft table state.
     *
     * \return the formation aircraft table state; a \e null QByteArray if not saved before
     */
    QByteArray getFormationAircraftTableState() const;

    /*!
     * Stores the formation aircraft table state.
     *
     * \param state
     *        the formation aircraft table state encoded in the QByteAarray
     */
    void setFormationAircraftTableState(QByteArray state) noexcept;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
    const std::unique_ptr<FormationSettingsPrivate> d;
};

#endif // FORMATIONSETTINGS_H
