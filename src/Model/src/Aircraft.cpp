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
#include <QByteArray>
#include <QVector>

#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "Engine.h"
#include "AircraftInfo.h"
#include "AircraftData.h"
#include "PrimaryFlightControl.h"
#include "SecondaryFlightControl.h"
#include "AircraftHandle.h"
#include "Light.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTimestamp),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    Engine engine;
    PrimaryFlightControl primaryFlightControl;
    SecondaryFlightControl secondaryFlightControl;
    AircraftHandle aircraftHandle;
    Light light;
    AircraftInfo aircraftInfo;

    QVector<AircraftData> aircraftData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    AircraftData currentAircraftData;
    mutable int currentIndex;
};

// PUBLIC

Aircraft::Aircraft(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AircraftPrivate>())
{
}

Aircraft::~Aircraft() noexcept
{
}

const Engine &Aircraft::getEngineConst() const
{
    return d->engine;
}

Engine &Aircraft::getEngine() const
{
    return d->engine;
}

const PrimaryFlightControl &Aircraft::getPrimaryFlightControlConst() const
{
    return d->primaryFlightControl;
}

PrimaryFlightControl &Aircraft::getPrimaryFlightControl() const
{
    return d->primaryFlightControl;
}

const SecondaryFlightControl &Aircraft::getSecondaryFlightControlConst() const
{
    return d->secondaryFlightControl;
}

SecondaryFlightControl &Aircraft::getSecondaryFlightControl() const
{
    return d->secondaryFlightControl;
}

const AircraftHandle &Aircraft::getAircraftHandleConst() const
{
    return d->aircraftHandle;
}

AircraftHandle &Aircraft::getAircraftHandle() const
{
    return d->aircraftHandle;
}

const Light &Aircraft::getLightConst() const
{
    return d->light;
}

Light &Aircraft::getLight() const
{
    return d->light;
}

void Aircraft::setAircraftInfo(AircraftInfo aircraftInfo) noexcept
{
    d->aircraftInfo = aircraftInfo;
    emit infoChanged();
}

const AircraftInfo &Aircraft::getAircraftInfo() const noexcept
{
    return d->aircraftInfo;
}

void Aircraft::upsert(AircraftData aircraftData) noexcept
{
    if (d->aircraftData.count() > 0 && d->aircraftData.last().timestamp == aircraftData.timestamp)  {
        // Same timestamp -> replace
        d->aircraftData[d->aircraftData.count() - 1] = aircraftData;
#ifdef DEBUG
        qDebug("Aircraft::upsertAircraftData: UPDATE sample, timestamp: %llu count: %d", aircraftData.timestamp, d->aircraftData.count());
#endif
    } else {
        d->aircraftData.append(aircraftData);
#ifdef DEBUG
        qDebug("Aircraft::upsertAircraftData: INSERT sample, timestamp: %llu count: %d", aircraftData.timestamp, d->aircraftData.count());
#endif
    }
    emit dataChanged();
}

const AircraftData &Aircraft::getLast() const noexcept
{
    if (!d->aircraftData.isEmpty()) {
        return d->aircraftData.last();
    } else {
        return AircraftData::NullAircraftData;
    }
}

const QVector<AircraftData> Aircraft::getAll() const noexcept
{
    return d->aircraftData;
}

void Aircraft::clear()
{
    d->aircraftData.clear();
    d->engine.clear();
    d->primaryFlightControl.clear();
    d->secondaryFlightControl.clear();
    d->aircraftHandle.clear();
    d->light.clear();
    d->aircraftInfo.clear();
    d->currentTimestamp = TimeVariableData::InvalidTimestamp;
    d->currentIndex = SkySearch::InvalidIndex;

    emit dataChanged();
}

const AircraftData &Aircraft::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const AircraftData *p0, *p1, *p2, *p3;
    const double Tension = 0.0;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getSupportData(d->aircraftData, timestamp, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, timestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->aircraftData, d->currentIndex, timestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->aircraftData.at(d->currentIndex);
                p2 = p1;
                tn = 0.0;
            } else {
                p1 = p2 = nullptr;
            }
            break;
        default:
            p1 = p2 = nullptr;
            break;
        }

        if (p1 != nullptr) {
            // Aircraft position & attitude

            // Latitude: [-90, 90] - no discontinuity at +/- 90
            d->currentAircraftData.latitude  = SkyMath::interpolateHermite(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn, Tension);
            // Longitude: [-180, 180] - discontinuity at the +/- 180 meridian
            d->currentAircraftData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn, Tension);
            // Altitude [open range]
            d->currentAircraftData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn, Tension);
            // Pitch: [-90, 90] - no discontinuity at +/- 90
            d->currentAircraftData.pitch = SkyMath::interpolateHermite(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn, Tension);
            // Bank: [-180, 180] - discontinuity at +/- 180
            d->currentAircraftData.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn, Tension);
            // Heading: [0, 360] - discontinuity at 0/360
            d->currentAircraftData.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn, Tension);

            // Velocity
            d->currentAircraftData.velocityBodyX = SkyMath::interpolateLinear(p1->velocityBodyX, p2->velocityBodyX, tn);
            d->currentAircraftData.velocityBodyY = SkyMath::interpolateLinear(p1->velocityBodyY, p2->velocityBodyY, tn);
            d->currentAircraftData.velocityBodyZ = SkyMath::interpolateLinear(p1->velocityBodyZ, p2->velocityBodyZ, tn);
            d->currentAircraftData.rotationVelocityBodyX = SkyMath::interpolateLinear(p1->rotationVelocityBodyX, p2->rotationVelocityBodyX, tn);
            d->currentAircraftData.rotationVelocityBodyY = SkyMath::interpolateLinear(p1->rotationVelocityBodyY, p2->rotationVelocityBodyY, tn);
            d->currentAircraftData.rotationVelocityBodyZ = SkyMath::interpolateLinear(p1->rotationVelocityBodyZ, p2->rotationVelocityBodyZ, tn);

            d->currentAircraftData.timestamp = timestamp;

        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentAircraftData = AircraftData::NullAircraftData;
        }
        d->currentTimestamp = timestamp;
#ifdef DEBUG
    } else {
        qDebug("Aircraft::interpolateAircraftData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentAircraftData;
}
