/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef TIMEVARIABLEDATA_H
#define TIMEVARIABLEDATA_H

#include <limits>
#include <cstdint>

#include "ModelLib.h"

struct MODEL_API TimeVariableData
{
    TimeVariableData() = default;
    TimeVariableData(const TimeVariableData &rhs) = default;
    TimeVariableData(TimeVariableData &&rhs) = default;
    TimeVariableData &operator=(const TimeVariableData &rhs) = default;
    TimeVariableData &operator=(TimeVariableData &&rhs) = default;
    virtual ~TimeVariableData() = default;

    /*!
     * Defines the way (use case) the sampled data is accessed. A distinction is made for the seek access:
     *
     * - discrete: a single seek operation is made; certain event data (such as flaps handle index) is
     *   only sent in this case
     * - continuous: a continuous seek operation is in progress ("dragging the time position slider");
     *   certain event data is dererred until the end of the seek operation
     */
    enum struct Access: std::uint8_t
    {
        /*!
         * The sampled data is accessed in a linear way, taking the time offset of the Aircraft into account,
         * typically for replay.
         */
        Linear,
        /*!
         * The sampled data is accessed in a random fashion, taking the time offset of the Aircraft
         * into account, typically for a single seek operation ("skip to begin/end, end of a continuous
         * time position slider drag operation")
         */
        DiscreteSeek,
        /*!
         * The sampled data is accessed in a random fashion, taking the time offset of the Aircraft
         * into account, typically for continuous seeking in the timeline ("dragging the time position slider").
         */
        ContinuousSeek,
        /*!
         * The sampled data is accessed for import or export (in a linear way), but always starting from the first
         * sample point (not taking the time offset of the Aircraft into account).
         */
        NoTimeOffset
    };

    static inline bool isSeek(Access access) noexcept
    {
        return access == Access::DiscreteSeek || access == Access::ContinuousSeek;
    }

    // In milliseconds since the start of recording
    std::int64_t timestamp {InvalidTime};

    /*!
     * Returns whether this data is considered \e null data.
     *
     * \return \c true if this data is \e null (invalid) data; \c false else
     * \sa reset
     */
    inline bool isNull() const noexcept
    {
        return (timestamp == InvalidTime);
    }

    /*!
     * Resets this data such that it is considered \e null data afterwards.
     *
     * \sa isNull
     */
    inline void reset() noexcept
    {
        timestamp = InvalidTime;
    }

    friend inline bool operator==(const TimeVariableData &lhs, const TimeVariableData &rhs) noexcept
    {
        return lhs.timestamp == rhs.timestamp;
    }

    friend inline bool operator>=(const TimeVariableData &lhs, const TimeVariableData &rhs) noexcept
    {
        return lhs.timestamp >= rhs.timestamp;
    }

    friend inline bool operator<(const TimeVariableData &lhs, const TimeVariableData &rhs) noexcept
    {
        return !(lhs >= rhs);
    }

    static constexpr std::int64_t InvalidTime = std::numeric_limits<std::int64_t>::min();

};

#endif // TIMEVARIABLEDATA_H
