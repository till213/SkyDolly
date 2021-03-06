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
#include <memory>

#include <QObject>
#include <QVector>

#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "Position.h"
#include "PositionData.h"
#include "Engine.h"
#include "EngineData.h"
#include "PrimaryFlightControl.h"
#include "PrimaryFlightControlData.h"
#include "SecondaryFlightControl.h"
#include "SecondaryFlightControlData.h"
#include "AircraftHandle.h"
#include "AircraftHandleData.h"
#include "Light.h"
#include "LightData.h"
#include "FlightPlan.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate() noexcept
        : id(Aircraft::InvalidId),
          simulationObjectId(Aircraft::InvalidSimulationId),
          aircraftInfo(id),
          duration(TimeVariableData::InvalidTime)
    {}

    qint64 id;
    qint64 simulationObjectId;
    AircraftInfo aircraftInfo;
    Position position;
    Engine engine;
    PrimaryFlightControl primaryFlightControl;
    SecondaryFlightControl secondaryFlightControl;
    AircraftHandle aircraftHandle;
    Light light;
    FlightPlan flightPlan;

    mutable qint64 duration;
};

// PUBLIC

Aircraft::Aircraft(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AircraftPrivate>())
{
    frenchConnection();
}

Aircraft::~Aircraft() noexcept
{
#ifdef DEBUG
    qDebug("Aircraft::~Aircraft: DELETED, ID: %lld", d->id);
#endif
}

qint64 Aircraft::getId() const noexcept
{
    return d->id;
}

void Aircraft::setId(qint64 id) noexcept
{
    d->id = id;
    d->aircraftInfo.aircraftId = id;
}

qint64 Aircraft::getSimulationObjectId() const noexcept
{
    return d->simulationObjectId;
}

void Aircraft::setSimulationObjectId(qint64 id) noexcept
{
    d->simulationObjectId = id;
}

const Position &Aircraft::getPositionConst() const noexcept
{
    return d->position;
}

Position &Aircraft::getPosition() const noexcept
{
    return d->position;
}

const Engine &Aircraft::getEngineConst() const noexcept
{
    return d->engine;
}

Engine &Aircraft::getEngine() const noexcept
{
    return d->engine;
}

const PrimaryFlightControl &Aircraft::getPrimaryFlightControlConst() const noexcept
{
    return d->primaryFlightControl;
}

PrimaryFlightControl &Aircraft::getPrimaryFlightControl() const noexcept
{
    return d->primaryFlightControl;
}

const SecondaryFlightControl &Aircraft::getSecondaryFlightControlConst() const noexcept
{
    return d->secondaryFlightControl;
}

SecondaryFlightControl &Aircraft::getSecondaryFlightControl() const noexcept
{
    return d->secondaryFlightControl;
}

const AircraftHandle &Aircraft::getAircraftHandleConst() const noexcept
{
    return d->aircraftHandle;
}

AircraftHandle &Aircraft::getAircraftHandle() const noexcept
{
    return d->aircraftHandle;
}

const Light &Aircraft::getLightConst() const noexcept
{
    return d->light;
}

Light &Aircraft::getLight() const noexcept
{
    return d->light;
}

const AircraftInfo &Aircraft::getAircraftInfoConst() const noexcept
{
    return d->aircraftInfo;
}

void Aircraft::setAircraftInfo(const AircraftInfo &aircraftInfo) noexcept
{
    d->aircraftInfo = aircraftInfo;
    emit infoChanged();
}

const FlightPlan &Aircraft::getFlightPlanConst() const noexcept
{
    return d->flightPlan;
}

FlightPlan &Aircraft::getFlightPlan() const noexcept
{
    return d->flightPlan;
}

qint64 Aircraft::getDurationMSec() const noexcept
{
    if (d->duration == TimeVariableData::InvalidTime) {
        d->duration = 0;
        if (d->position.count() > 0) {
            d->duration = d->position.getLast().timestamp;
        }
        if (d->engine.count() > 0) {
            d->duration = qMax(d->engine.getLast().timestamp, d->duration);
        }
        if (d->primaryFlightControl.count() > 0) {
            d->duration = qMax(d->primaryFlightControl.getLast().timestamp, d->duration);
        }
        if (d->secondaryFlightControl.count() > 0) {
            d->duration = qMax(d->secondaryFlightControl.getLast().timestamp, d->duration);
        }
        if (d->aircraftHandle.count() > 0) {
            d->duration = qMax(d->aircraftHandle.getLast().timestamp, d->duration);
        }
        if (d->light.count() > 0) {
            d->duration = qMax(d->light.getLast().timestamp, d->duration);
        }
        // Update end time
        d->aircraftInfo.endDate = d->aircraftInfo.startDate.addMSecs(d->duration);
    }
    return d->duration;
}

bool Aircraft::hasRecording() const noexcept
{
    return d->position.count() > 0;
}

void Aircraft::clear() noexcept
{
    d->position.clear();
    d->engine.clear();
    d->primaryFlightControl.clear();
    d->secondaryFlightControl.clear();
    d->aircraftHandle.clear();
    d->light.clear();
    d->flightPlan.clear();
    d->aircraftInfo.clear();
    emit dataChanged();
}

bool Aircraft::operator == (const Aircraft &rhs) const noexcept
{
    return this->d->id == rhs.d->id;
}

bool Aircraft::operator != (const Aircraft &rhs) const noexcept
{
    return this->d->id != rhs.d->id;
}

// PRIVATE

void Aircraft::frenchConnection()
{
    connect(&d->position, &Position::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(&d->engine, &Engine::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(&d->primaryFlightControl, &PrimaryFlightControl::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(&d->secondaryFlightControl, &SecondaryFlightControl::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(&d->aircraftHandle, &AircraftHandle::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(&d->light, &Light::dataChanged,
            this, &Aircraft::handleDataChanged);
    connect(this, &Aircraft::dataChanged,
            this, &Aircraft::invalidateDuration);
}

// PRIVATE SLOTS

void Aircraft::handleDataChanged()
{
    emit dataChanged();
}

void Aircraft::invalidateDuration()
{
    d->duration = TimeVariableData::InvalidTime;
    d->aircraftInfo.endDate = QDateTime();
}
