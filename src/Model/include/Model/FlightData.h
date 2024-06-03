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
#ifndef FLIGHTDATA_H
#define FLIGHTDATA_H

#include <cstdint>
#include <vector>

#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include "Aircraft.h"
#include "FlightCondition.h"
#include "ModelLib.h"

struct MODEL_API FlightData final
{
    /*! Defines how the flight creation time is to be reset upon clearing the flight. */
    enum struct CreationTimeMode
    {
        /*! Update the creation time to the current date & time */
        Update,
        /*! Reset the creation time to an invalid date & time */
        Reset
    };

    std::int64_t id {Const::InvalidId};
    // To be updated to "current time" at the very moment when the first recording starts
    QDateTime creationTime;
    QString title;
    QString description;
    QString flightNumber;
    FlightCondition flightCondition;
    std::vector<Aircraft> aircraft;
    int userAircraftIndex {Const::InvalidIndex};

    FlightData() = default;
    FlightData(const FlightData& rhs) = delete;
    FlightData(FlightData&& rhs) = default;
    FlightData& operator=(const FlightData& rhs) = delete;
    FlightData& operator=(FlightData&& rhs) = default;
    ~FlightData() = default;

    inline void clear(bool withOneAircraft, CreationTimeMode creationTimeMode) noexcept {
        id = Const::InvalidId;
        switch (creationTimeMode) {
        case CreationTimeMode::Update:
            creationTime = QDateTime::currentDateTime();
            break;
        case CreationTimeMode::Reset:
            creationTime = QDateTime();
            break;
        }
        title.clear();
        description.clear();
        flightCondition.clear();
        if (aircraft.size() > 0) {
            const int aircraftCount = withOneAircraft ? 1 : 0;
            aircraft.resize(aircraftCount);
            userAircraftIndex = withOneAircraft ? 0 : Const::InvalidIndex;
        }
        // A flight always has at least one aircraft; unless
        // it is newly allocated (the aircraft is only added in the constructor body)
        // or cleared just before loading a flight
        if (aircraft.size() > 0) {
            aircraft.at(0).clear();
        }
    }

    /*!
     * Returns the total duration of the flight [in milliseconds], that is it returns
     * the longest replay time of all aircraft, taking their time offsets into account.
     * Unless \c ofUserAircraft is set to \c true, in which case the replay time of the
     * \e user aircraft is returned.
     *
     * Note that the total duration is cached and not updated during recording. Use
     * the SkyConnectIntf#getCurrentTimestamp in this case, which - during recording - indicates
     * the current recorded duration (for the user aircraft).
     *
     * \param ofUserAircraft
     *        set to \c true in case to specifically query the replay duration
     *        of the user aircraft; \c false in order to query the total duration
     *        for the flight; default: \c false
     * \return the total replay duration of the flight, or the replay duration of
     *         the user aircraft: 0 if no user aircraft exists
     */
    inline std::int64_t getTotalDurationMSec(bool ofUserAircraft = false) const noexcept
    {
        std::int64_t totalDuractionMSec = 0;
        if (ofUserAircraft) {
            totalDuractionMSec = userAircraftIndex != Const::InvalidIndex ? aircraft[userAircraftIndex].getDurationMSec() : 0;
        } else {
            for (const auto &aircraft : aircraft) {
                totalDuractionMSec = std::max(aircraft.getDurationMSec(), totalDuractionMSec);
            }
        }
        return totalDuractionMSec;
    }

    /*!
     * Adds a new user Aircraft to this FlightData.
     *
     * \return the newly added Aircraft
     * \sa getUserAircraft
     */
    inline Aircraft &addUserAircraft(std::int64_t aircraftId = Const::InvalidId) noexcept
    {
        aircraft.emplace_back(aircraftId);
        // Index starts at 0
        userAircraftIndex = static_cast<int>(aircraft.size()) - 1;
        return aircraft.back();
    }

    inline Aircraft &getUserAircraft() noexcept
    {
        return aircraft.at(userAircraftIndex);
    }

    inline const Aircraft &getUserAircraftConst() const noexcept
    {
        return aircraft.at(userAircraftIndex);
    }

    inline QDateTime getAircraftCreationTime(const Aircraft &aircraft) const noexcept
    {
        return creationTime.addMSecs(-aircraft.getTimeOffset());
    }

    inline QDateTime getAircraftStartLocalTime(const Aircraft &aircraft) const noexcept
    {
        return flightCondition.startLocalDateTime.addMSecs(-aircraft.getTimeOffset());
    }

    inline QDateTime getAircraftStartZuluTime(const Aircraft &aircraft) const noexcept
    {
        return flightCondition.startZuluDateTime.addMSecs(-aircraft.getTimeOffset());
    }

    inline bool hasRecording() const noexcept
    {
        auto noRecording = [](const Aircraft &a){ return !a.hasRecording(); };
        return std::find_if(begin(), end(), noRecording) == end();
    }

    static bool hasAllRecording(const std::vector<FlightData> &flights) noexcept
    {
        auto noRecording = [](const FlightData &f){ return !f.hasRecording(); };
        return std::find_if(flights.begin(), flights.end(), noRecording) == flights.end();
    }

    using SizeType = std::vector<Aircraft>::size_type;
    inline SizeType count() const noexcept
    {
        return aircraft.size();
    }

    using Iterator = std::vector<Aircraft>::iterator;
    Iterator begin() noexcept
    {
        return aircraft.begin();
    }

    Iterator end() noexcept
    {
        return aircraft.end();
    }

    using ConstIterator = std::vector<Aircraft>::const_iterator;
    ConstIterator begin() const noexcept
    {
        return aircraft.cbegin();
    }

    ConstIterator end() const noexcept
    {
        return aircraft.cend();
    }

    // OPERATORS

    Aircraft &operator[](std::size_t index) noexcept
    {
        return aircraft[index];
    }

    const Aircraft &operator[](std::size_t index) const noexcept
    {
        return aircraft[index];
    }
};

#endif // FLIGHTDATA_H
