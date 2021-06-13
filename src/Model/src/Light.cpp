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
#include "LightData.h"
#include "Light.h"

class LightPrivate
{
public:
    LightPrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    QVector<LightData> lightData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    LightData currentLightData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

Light::Light(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<LightPrivate>())
{}

Light::~Light() noexcept
{
#ifdef DEBUG
    qDebug("Light::~Light: DELETED, data count: %d", d->lightData.size());
#endif
}

void Light::upsert(const LightData &lightData) noexcept
{
    if (d->lightData.count() > 0 && d->lightData.last().timestamp == lightData.timestamp)  {
        // Same timestamp -> replace
        d->lightData[d->lightData.count() - 1] = lightData;
    } else {
        d->lightData.append(lightData);
    }
    emit dataChanged();
}

const LightData &Light::getLast() const noexcept
{
    if (!d->lightData.isEmpty()) {
        return d->lightData.last();
    } else {
        return LightData::NullData;
    }
}

QVector<LightData> &Light::getAll() const noexcept
{
    return d->lightData;
}

const QVector<LightData> &Light::getAllConst() const noexcept
{
    return d->lightData;
}

void Light::clear() noexcept
{
    d->lightData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const LightData &Light::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const LightData *p1, *p2;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        switch (access) {
        case TimeVariableData::Access::Linear:
            SkySearch::getLinearInterpolationSupportData(d->lightData, timestamp, d->currentIndex, &p1, &p2);
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->lightData, d->currentIndex, timestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->lightData.at(d->currentIndex);
                p2 = p1;
            } else {
                p1 = p2 = nullptr;
            }
            break;
        default:
            p1 = p2 = nullptr;
            break;
        }

        if (p1 != nullptr) {
            // No interpolation for light states
            d->currentLightData.lightStates = p1->lightStates;
            d->currentLightData.timestamp = timestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentLightData = LightData::NullData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
#ifdef DEBUG
    } else {
        qDebug("Light::interpolateLightData: cached result for timestamp: %lld", timestamp);
#endif
    }
    return d->currentLightData;
}
