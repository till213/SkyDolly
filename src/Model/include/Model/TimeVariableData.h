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
#ifndef TIMEVARIABLEDATA_H
#define TIMEVARIABLEDATA_H

#include <limits>
#include <cstdint>

#include "SimType.h"
#include "ModelLib.h"

struct MODEL_API TimeVariableData
{
    static constexpr std::int64_t InvalidTime = std::numeric_limits<std::int64_t>::min();

    /*!
     * Defines the way (use case) the sampled data is accessed.
     */
    enum struct Access {
        /*! The sampled data is accessed in a linear way, taking the time-offset of the Aircraft into account, typically for replay. */
        Linear,
        /*! The sampled data is accessed in a random fashion, taking the time-offset of the Aircraft into account, typically for seeking in the timeline. */
        Seek,
        /*! The sampled data is accessed for export (in a linear way), but always starting from the first sample point (not taking the time-offset of the Aircraft into account). */
        Export
    };

    // In milliseconds since the start of recording
    std::int64_t timestamp;

    TimeVariableData() noexcept;
    virtual ~TimeVariableData() noexcept;

    inline bool isNull() const noexcept {
        return (timestamp == InvalidTime);
    }

    TimeVariableData(TimeVariableData &&) = default;
    TimeVariableData(const TimeVariableData &) = default;
    TimeVariableData &operator= (const TimeVariableData &rhs) = default;

    inline bool operator == (const TimeVariableData &rhs) noexcept
    {
        return timestamp == rhs.timestamp;
    }

    inline bool operator>=(const TimeVariableData &rhs) noexcept
    {
        return timestamp >= rhs.timestamp;
    }

    inline bool operator<(const TimeVariableData &rhs) noexcept
    {
        return !(*this >= rhs);
    }

    friend inline bool operator>=(const TimeVariableData &lhs, const TimeVariableData &rhs) noexcept
    {
        return lhs.timestamp >= rhs.timestamp;
    }

    friend inline bool operator<(const TimeVariableData &lhs, const TimeVariableData &rhs) noexcept
    {
        return !(lhs >= rhs);
    }
};



#endif // TIMEVARIABLEDATA_H
