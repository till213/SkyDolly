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
#include <memory>
#include <utility>

#include <QByteArray>

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "Formation.h"
#include "FormationSettings.h"

namespace
{
    // Keys
    constexpr const char *RelativePositionPlacementKey {"RelativePositionPlacement"};
    constexpr const char *BearingKey {"Bearing"};
    constexpr const char *HorizontalDistanceKey {"HorizontalDistance"};
    constexpr const char *VerticalDistanceKey {"VerticalDistance"};
    constexpr const char *FormationAircraftTableStateKey {"FormationAircraftTableState"};
    constexpr const char *ReplayModeKey {"ReplayMode"};

    // Defaults
    constexpr Formation::Bearing DefaultBearing {Formation::Bearing::SouthEast};
    constexpr Formation::HorizontalDistance DefaultHorizontalDistance {Formation::HorizontalDistance::Nearby};
    constexpr Formation::VerticalDistance DefaultVerticalDistance {Formation::VerticalDistance::Level};
    constexpr bool DefaultRelativePositionPlacement {true};
    constexpr SkyConnectIntf::ReplayMode DefaultReplayMode {SkyConnectIntf::ReplayMode::Normal};
}

struct FormationSettingsPrivate
{
    QByteArray formationAircraftTableState;
    Formation::Bearing bearing {::DefaultBearing};
    Formation::HorizontalDistance horizontalDistance {::DefaultHorizontalDistance};
    Formation::VerticalDistance verticalDistance {::DefaultVerticalDistance};
    bool relativePositionPlacement {::DefaultRelativePositionPlacement};
    SkyConnectIntf::ReplayMode replayMode {::DefaultReplayMode};
};

// PUBLIC

FormationSettings::FormationSettings() noexcept
    : ModuleBaseSettings(),
      d(std::make_unique<FormationSettingsPrivate>())
{}

FormationSettings::~FormationSettings() = default;

Formation::Bearing FormationSettings::getBearing() const noexcept
{
    return d->bearing;
}

void FormationSettings::setBearing(Formation::Bearing bearing) noexcept
{
    if (d->bearing != bearing) {
        d->bearing = bearing;
        emit changed();
    }
}

Formation::HorizontalDistance FormationSettings::getHorizontalDistance() const noexcept
{
    return d->horizontalDistance;
}

void FormationSettings::setHorizontalDistance(Formation::HorizontalDistance horizontalDistance) noexcept
{
    if (d->horizontalDistance != horizontalDistance) {
        d->horizontalDistance = horizontalDistance;
        emit changed();
    }
}

Formation::VerticalDistance FormationSettings::getVerticalDistance() const noexcept
{
    return d->verticalDistance;
}

void FormationSettings::setVerticalDistance(Formation::VerticalDistance verticalDistance) noexcept
{
    if (d->verticalDistance != verticalDistance) {
        d->verticalDistance = verticalDistance;
        emit changed();
    }
}

bool FormationSettings::isRelativePositionPlacementEnabled() const noexcept
{
    return d->relativePositionPlacement;
}

void FormationSettings::setRelativePositionPlacementEnabled(bool enable) noexcept
{
    if (d->relativePositionPlacement != enable) {
        d->relativePositionPlacement = enable;
        emit changed();
    }
}

SkyConnectIntf::ReplayMode FormationSettings::getReplayMode() const noexcept
{
    return d->replayMode;
}

void FormationSettings::setReplayMode(SkyConnectIntf::ReplayMode replayMode) noexcept
{
    if (d->replayMode != replayMode) {
        d->replayMode = replayMode;
        emit changed();
    }
}

QByteArray FormationSettings::getFormationAircraftTableState() const
{
    return d->formationAircraftTableState;
}

void FormationSettings::setFormationAircraftTableState(QByteArray state) noexcept
{
    d->formationAircraftTableState = std::move(state);
}

// PROTECTED

void FormationSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::BearingKey);
    keyValue.second = Enum::underly(d->bearing);
    keyValues.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::HorizontalDistanceKey);
    keyValue.second = Enum::underly(d->horizontalDistance);
    keyValues.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::VerticalDistanceKey);
    keyValue.second = Enum::underly(d->verticalDistance);
    keyValues.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::RelativePositionPlacementKey);
    keyValue.second = d->relativePositionPlacement;
    keyValues.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::ReplayModeKey);
    keyValue.second = Enum::underly(d->replayMode);
    keyValues.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::FormationAircraftTableStateKey);
    keyValue.second = d->formationAircraftTableState;
    keyValues.push_back(keyValue);
}

void FormationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::BearingKey);
    keyValue.second = Enum::underly(::DefaultBearing);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::HorizontalDistanceKey);
    keyValue.second = Enum::underly(::DefaultHorizontalDistance);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::VerticalDistanceKey);
    keyValue.second = Enum::underly(::DefaultVerticalDistance);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::RelativePositionPlacementKey);
    keyValue.second = ::DefaultRelativePositionPlacement;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::ReplayModeKey);
    keyValue.second = Enum::underly(::DefaultReplayMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = QString::fromLatin1(::FormationAircraftTableStateKey);
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void FormationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {false};
    auto enumValue = valuesByKey.at(QString::fromLatin1(::BearingKey)).toInt(&ok);
    d->bearing = ok && Enum::contains<Formation::Bearing>(enumValue) ? static_cast<Formation::Bearing>(enumValue) : ::DefaultBearing;

    enumValue = valuesByKey.at(QString::fromLatin1(::HorizontalDistanceKey)).toInt(&ok);
    d->horizontalDistance = ok && Enum::contains<Formation::HorizontalDistance>(enumValue) ? static_cast<Formation::HorizontalDistance>(enumValue) : ::DefaultHorizontalDistance;

    enumValue = valuesByKey.at(QString::fromLatin1(::VerticalDistanceKey)).toInt(&ok);
    d->verticalDistance = ok && Enum::contains<Formation::VerticalDistance>(enumValue) ? static_cast<Formation::VerticalDistance>(enumValue) : ::DefaultVerticalDistance;

    d->relativePositionPlacement = valuesByKey.at(QString::fromLatin1(::RelativePositionPlacementKey)).toBool();

    enumValue = valuesByKey.at(QString::fromLatin1(::ReplayModeKey)).toInt(&ok);
    d->replayMode = ok && Enum::contains<SkyConnectIntf::ReplayMode>(enumValue) ? static_cast<SkyConnectIntf::ReplayMode>(enumValue) : ::DefaultReplayMode;

    d->formationAircraftTableState = valuesByKey.at(QString::fromLatin1(::FormationAircraftTableStateKey)).toByteArray();
}

void FormationSettings::restoreDefaultsExtn() noexcept
{
    d->bearing = ::DefaultBearing;
    d->horizontalDistance = ::DefaultHorizontalDistance;
    d->verticalDistance = ::DefaultVerticalDistance;
    d->relativePositionPlacement = ::DefaultRelativePositionPlacement;
    d->replayMode = ::DefaultReplayMode;
    d->formationAircraftTableState = {};
}
