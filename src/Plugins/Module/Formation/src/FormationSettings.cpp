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

    // Defaults
    constexpr int DefaultBearing {135};
    constexpr Formation::HorizontalDistance DefaultHorizontalDistance {Formation::HorizontalDistance::Nearby};
    constexpr Formation::VerticalDistance DefaultVerticalDistance {Formation::VerticalDistance::Level};
    constexpr bool DefaultRelativePositionPlacement {true};
}

struct FormationSettingsPrivate
{
    QByteArray formationAircraftTableState;
    int bearing {::DefaultBearing};
    Formation::HorizontalDistance horizontalDistance {::DefaultHorizontalDistance};
    Formation::VerticalDistance verticalDistance {::DefaultVerticalDistance};
    bool relativePositionPlacement {::DefaultRelativePositionPlacement};
};

// PUBLIC

FormationSettings::FormationSettings() noexcept
    : ModuleBaseSettings(),
      d(std::make_unique<FormationSettingsPrivate>())
{}

FormationSettings::~FormationSettings() = default;

int FormationSettings::getBearing() const noexcept
{
    return d->bearing;
}

void FormationSettings::setBearing(int bearing) noexcept
{
    if (d->bearing != bearing) {
        d->bearing = bearing;
        emit changed();
    }
}

Formation::HorizontalDistance FormationSettings::getHorizontalDistance() const noexcept
{
    return d->horizontalDistance;;
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

QByteArray FormationSettings::getFormationAircraftTableState() const
{
    return d->formationAircraftTableState;
}

void FormationSettings::setFormationAircraftTableState(QByteArray state) noexcept
{
    if (d->formationAircraftTableState != state) {
        d->formationAircraftTableState = std::move(state);
        emit changed();
    }
}

// PROTECTED

void FormationSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::BearingKey;
    keyValue.second = d->bearing;
    keyValues.push_back(keyValue);

    keyValue.first = ::HorizontalDistanceKey;
    keyValue.second = Enum::underly(d->horizontalDistance);
    keyValues.push_back(keyValue);

    keyValue.first = ::VerticalDistanceKey;
    keyValue.second = Enum::underly(d->verticalDistance);
    keyValues.push_back(keyValue);

    keyValue.first = ::RelativePositionPlacementKey;
    keyValue.second = d->relativePositionPlacement;
    keyValues.push_back(keyValue);

    keyValue.first = ::FormationAircraftTableStateKey;
    keyValue.second = d->formationAircraftTableState;
    keyValues.push_back(keyValue);
}

void FormationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::BearingKey;
    keyValue.second = ::DefaultBearing;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::HorizontalDistanceKey;
    keyValue.second = Enum::underly(::DefaultHorizontalDistance);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::VerticalDistanceKey;
    keyValue.second = Enum::underly(::DefaultVerticalDistance);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::RelativePositionPlacementKey;
    keyValue.second = ::DefaultRelativePositionPlacement;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::FormationAircraftTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void FormationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {false};
    d->bearing = valuesByKey.at(::BearingKey).toInt(&ok);
    if (!ok) {
        d->bearing = ::DefaultBearing;
    }
    d->horizontalDistance = static_cast<Formation::HorizontalDistance>(valuesByKey.at(::HorizontalDistanceKey).toInt(&ok));
    if (!ok) {
        d->horizontalDistance = ::DefaultHorizontalDistance;
    }
    d->verticalDistance = static_cast<Formation::VerticalDistance>(valuesByKey.at(::VerticalDistanceKey).toInt(&ok));
    if (!ok) {
        d->verticalDistance = ::DefaultVerticalDistance;
    }

    d->relativePositionPlacement = valuesByKey.at(::RelativePositionPlacementKey).toBool();
    d->formationAircraftTableState = valuesByKey.at(::FormationAircraftTableStateKey).toByteArray();
}

void FormationSettings::restoreDefaultsExtn() noexcept
{
    d->bearing = ::DefaultBearing;
    d->horizontalDistance = ::DefaultHorizontalDistance;
    d->verticalDistance = ::DefaultVerticalDistance;
    d->relativePositionPlacement = ::DefaultRelativePositionPlacement;
    d->formationAircraftTableState = {};
}
