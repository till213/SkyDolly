/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SKYSEARCH_H
#define SKYSEARCH_H

#include <type_traits>
#include <vector>
#include <limits>
#include <cstdint>

#include "TimeVariableData.h"

namespace SkySearch {

    constexpr int InvalidIndex = -1;
    constexpr int LinearIntervalSearch = -2;
    constexpr int BinaryIntervalSearch = -3;

    /*!
     * In case we seek 3 seconds "into the future" we use binary search
     * to find the next position (otherwise linear search, assuming that
     * the next position is "nearby" (within the 3 seconds threshold).
     */
    constexpr std::int64_t BinaryIntervalSearchThreshold = 3000;

    /*!
     * The size of the default interpolation window [-DefaultInterpolationWindow, DefaultInterpolationWindow] [milliseconds].
     * Only sampled data within that time window is considered for interpolation. Any other sampled data outside
     * (before or after) this window is considered "inactive".
     */
    constexpr std::int64_t DefaultInterpolationWindow = 2000;

    /*!
     * An interpolation window that considers all sample points.
     */
    constexpr std::int64_t InfinitetInterpolationWindow = std::numeric_limits<std::int64_t>::max();

    /*!
     * Returns the lower index i of the interval [i, j] where i.timestamp <= timestamp < j.timestamp.
     * A special case is the last index l in case l.timestamp =< timestamp, in which case the last index l
     * is returned.
     *
     * The interval is assumed to be within [lowIndex, highIndex] and searched with an adapted
     * binary search (O(log(N)) time).
     *
     * \param data
     *        the array of data, with ascending timestamps
     * \param timestamp
     *        the timestamp within the interval to be searched
     * \param lowIndex
     *        the lower starting index >= 0 and <= highIndex
     * \param highIndex
     *        the higher starting index <= lastIndex and >= lowIndex
     * \return the lower index i of the interval [i, j], or \p InvalidIndex if not found
     */
    template <typename T>
    int binaryIntervalSearch(const std::vector<T> &data, std::int64_t timestamp, int lowIndex, int highIndex) noexcept
    {
        int index {InvalidIndex};
        if (data.size() == 0 || data.at(lowIndex).timestamp > timestamp || data.at(highIndex).timestamp < timestamp) {
            index = InvalidIndex;
        } else if (data.at(0).timestamp == timestamp) {
            index = 0;
        } else if (data.back().timestamp == timestamp) {
            index = data.size() - 1;
        } else {
            int low = lowIndex;
            int high = highIndex;
            while (low <= high)
            {
                int mid = (low + high) / 2;
                if (data.at(mid).timestamp <= timestamp && (mid == high || timestamp < data.at(mid + 1).timestamp)) {
                    // Found the interval [mid].timestamp <= timestamp < [mid+1].timestamp
                    index = mid;
                    break;
                }
                else if (timestamp < data.at(mid).timestamp) {
                    // Modified binary search: we are actually looking for two indices [mid, mid + 1]
                    // which encompass the timestamp: that solution could still include index mid,
                    // so the high value becomes mid (and not high = mid -1, as in the original
                    // binary search
                    high = mid;
                }
                else {
                    // See comment above: mid could still part of the encompassing solution
                    // [mid, mid + 1], so low becomes mid (and not low = mid + 1)
                    low = mid;
                }
            }
        }

        return index;
    }

    template <typename T>
    int linearIntervalSearch(const std::vector<T> &data, std::int64_t timestamp, int startIndex) noexcept
    {
        int index {startIndex};
        if (data.size() == 0 || data.at(startIndex).timestamp > timestamp || data.back().timestamp < timestamp) {
            index = InvalidIndex;
        } else {
            int size = data.size();
            // Linear search: increment the current index, until we find a position having a
            // timestamp > the given timestamp
            bool found = false;
            while (!found && index < size) {
                if (index < (size - 1)) {
                    if (data.at(index + 1).timestamp > timestamp) {
                        // The next index has a larger timestamp, so this index is the one we are looking for
                        found = true;
                    } else {
                        ++index;
                    }
                } else {
                    // Reached the last index
                    found = true;
                }
            }
        }

        return index;
    }

    /*!
     *  Updates the \p startIndex with the last index having a timestamp <= the given \p timestamp.
     */
    template <typename T>
    int updateStartIndex(const std::vector<T> &data, int startIndex, std::int64_t timestamp) noexcept
    {
        int index {startIndex};
        int size = data.size();
        if (size > 0) {
            if (timestamp < data.back().timestamp) {
                if (index != InvalidIndex) {
                    if (timestamp < data.at(index).timestamp || timestamp - BinaryIntervalSearchThreshold > data.at(index).timestamp) {
                        // The timestamp was moved to front ("rewind"), so search the
                        // array until and including the current index
                        index = BinaryIntervalSearch;
                    }
                } else {
                    // Current index not yet initialised, so search the entire array
                    index = BinaryIntervalSearch;
                }
            } else {
                // The given timestamp lies past the last sample data
                // -> return the last sample data
                index = data.size() - 1;
            }
        } else {
            // No data yet, or timestamp not between given range
            index = InvalidIndex;
        }

        if (index != InvalidIndex && index != (data.size() - 1)) {

            // If the given timestamp lies "in the future" (as seen from the timetamp of the current index
            // the we assume that time has progressed "only a little" (normal replay) and we simply do
            // a linear search from the current index onwards
            if (index != BinaryIntervalSearch) {
                // Linear search: increment the current index, until we find a position having a
                // timestamp > the given timestamp
                index = linearIntervalSearch(data, timestamp, index);
            } else {
                // The given timestamp lies "in the past" and could really be anywwhere
                // -> binary search in the past
                auto low {0};
                auto high {data.size() - 1};
                if (startIndex != InvalidIndex) {
                    if (timestamp < data.at(startIndex).timestamp) {
                        // Search in "the past"
                        low = 0;
                        high = startIndex;
                    } else {
                        // Search in "the future"
                        low = startIndex;
                        high = data.size() - 1;
                    }
                }
                index = binaryIntervalSearch(data, timestamp, low, high);
            }

        }
        return index;
    }

    template <typename T>
    bool getCubicInterpolationSupportData(const std::vector<T> &data, std::int64_t timestamp, std::int64_t interpolationWindow, int &startIndex, const T **p0, const T **p1, const T **p2, const T **p3) noexcept
    {
        static_assert(std::is_base_of<TimeVariableData, T>::value, "T not derived from TimeVariableData");

        startIndex = updateStartIndex(data, startIndex, timestamp);
        if (startIndex != InvalidIndex) {

            *p1 = &data.at(startIndex);
            // Is p1 within the interpolation window?
            if ((timestamp - (*p1)->timestamp) <= interpolationWindow) {

                if (startIndex > 0) {
                   *p0 = &data.at(startIndex - 1);
                } else {
                   *p0 = *p1;
                }
                if (startIndex < data.size() - 1) {
                   if (startIndex < data.size() - 2) {
                       *p2 = &data.at(startIndex + 1);
                       *p3 = &data.at(startIndex + 2);
                   } else {
                       // p1 is the second to last data
                       *p2 = &data.at(startIndex + 1);
                       *p3 = *p2;
                   }
                } else {
                    // p1 is the last data
                    *p2 = *p3 = *p1;
                }

                // Is p2 within the interpolation window?
                if (((*p2)->timestamp - timestamp) > interpolationWindow) {
                    *p2 = *p3 = *p1;
                }

            } else {
                *p0 = *p1 = *p2 = *p3 = nullptr;
            }

        } else {
            // We are outside the sampled data time interval: either before the first (even with timestamp = 0;
            // sampled data does not necessarily start with timestamp = 0) or after the last sample point
            if (data.size() > 0) {
                if (timestamp < data.front().timestamp) {
                    // We always start with the first sample point (regardless of the interpolation window)
                    *p0 = *p1 = *p2 = *p3 = &data.front();
                } else if (timestamp <= data.back().timestamp + interpolationWindow) {
                    *p0 = *p1 = *p2 = *p3 = &data.back();
                } else {
                    *p0 = *p1 = *p2 = *p3 = nullptr;
                }
            } else {
                *p0 = *p1 = *p2 = *p3 = nullptr;
            }
        }

        return *p0 != nullptr;
    }

    template <typename T>
    bool getLinearInterpolationSupportData(const std::vector<T> &data, std::int64_t timestamp, std::int64_t interpolationWindow, int &startIndex, const T **p1, const T **p2) noexcept
    {
        static_assert(std::is_base_of<TimeVariableData, T>::value, "T not derived from TimeVariableData");

        startIndex = updateStartIndex(data, startIndex, timestamp);
        if (startIndex != InvalidIndex) {

            *p1 = &data.at(startIndex);
            // Is p1 within the interpolation window?
            if ((timestamp - (*p1)->timestamp) <= interpolationWindow) {

                if (startIndex < data.size() - 1) {
                    *p2 = &data.at(startIndex + 1);
                } else {
                    // p1 is the last data
                    *p2 = *p1;
                }

                // Is p2 within the interpolation window?
                if (((*p2)->timestamp - timestamp) > interpolationWindow) {
                    *p2 = *p1;
                }

            } else {
                *p1 = *p2 = nullptr;
            }

        } else {
            // We are past the last sample point
            if (data.size() > 0 && timestamp - data.back().timestamp <= interpolationWindow) {
                *p1 = *p2 = &data.back();
            } else {
                *p1 = *p2 = nullptr;
            }
        }

        return *p1 != nullptr;
    }

    double normaliseTimestamp(const TimeVariableData &p1, const TimeVariableData &p2, std::int64_t timestamp) noexcept;

} // namespace

#endif // SKYSEARCH_H
