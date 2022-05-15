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
#include <algorithm>
#include <cstdint>

#include <QDebug>

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "AircraftHandleData.h"
#include "AircraftHandle.h"

// PUBLIC

AircraftHandle::AircraftHandle(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{
#ifdef DEBUG
    qDebug() << "AircraftHandle::AircraftHandle: CREATED";
#endif
}

AircraftHandle::~AircraftHandle() noexcept
{
#ifdef DEBUG
    qDebug() << "AircraftHandle::AircraftHandle: DELETED";
#endif
}

const AircraftHandleData &AircraftHandle::interpolate(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const AircraftHandleData *p1 {nullptr}, *p2 {nullptr};
    const std::int64_t timeOffset = access != TimeVariableData::Access::Export ? getAircraftInfo().timeOffset : 0;
    const std::int64_t adjustedTimestamp = std::max(timestamp + timeOffset, std::int64_t(0));

    if (getCurrentTimestamp() != adjustedTimestamp || getCurrentAccess() != access) {

        int currentIndex = getCurrentIndex();
        double tn {0.0};
        switch (access) {
        case TimeVariableData::Access::Linear:
            [[fallthrough]];
        case TimeVariableData::Access::Export:
            if (SkySearch::getLinearInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::DefaultInterpolationWindow, currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            currentIndex = SkySearch::updateStartIndex(getData(), currentIndex, adjustedTimestamp);
            if (currentIndex != SkySearch::InvalidIndex) {
                p1 = &getData().at(currentIndex);
                p2 = p1;
                tn = 0.0;
            } else {
                p1 = p2 = nullptr;
            }
            break;
        }

        if (p1 != nullptr) {
            m_currentAircraftHandleData.brakeLeftPosition = SkyMath::interpolateLinear(p1->brakeLeftPosition, p2->brakeLeftPosition, tn);
            m_currentAircraftHandleData.brakeRightPosition = SkyMath::interpolateLinear(p1->brakeRightPosition, p2->brakeRightPosition, tn);
            m_currentAircraftHandleData.waterRudderHandlePosition = SkyMath::interpolateLinear(p1->waterRudderHandlePosition, p2->waterRudderHandlePosition, tn);
            m_currentAircraftHandleData.tailhookPosition = SkyMath::interpolateLinear(p1->tailhookPosition, p2->tailhookPosition, tn);
            m_currentAircraftHandleData.canopyOpen = SkyMath::interpolateLinear(p1->canopyOpen, p2->canopyOpen, tn);
            m_currentAircraftHandleData.leftWingFolding = SkyMath::interpolateLinear(p1->leftWingFolding, p2->leftWingFolding, tn);
            m_currentAircraftHandleData.rightWingFolding = SkyMath::interpolateLinear(p1->rightWingFolding, p2->rightWingFolding, tn);
            m_currentAircraftHandleData.gearHandlePosition = p1->gearHandlePosition;
            m_currentAircraftHandleData.smokeEnabled = p1->smokeEnabled;
            m_currentAircraftHandleData.timestamp = adjustedTimestamp;

            // Certain aircraft override the CANOPY OPEN, so values need to be repeatedly set
            if (Settings::getInstance().isRepeatCanopyOpenEnabled()) {
                // We do that my storing the previous values (when the canopy is "open")...
                m_previousAircraftHandleData = m_currentAircraftHandleData;
            } else {
                // "Repeat values" setting disabled
                m_previousAircraftHandleData = AircraftHandleData::NullData;
            }
        } else if (!m_previousAircraftHandleData.isNull()) {
            // ... and send the previous values again
            m_currentAircraftHandleData = m_previousAircraftHandleData;
            m_currentAircraftHandleData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            m_currentAircraftHandleData = AircraftHandleData::NullData;
        }

        setCurrentIndex(currentIndex);
        setCurrentTimestamp(adjustedTimestamp);
        setCurrentAccess(access);
    }
    return m_currentAircraftHandleData;
}
