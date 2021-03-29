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
#ifndef SKYSEARCH_H
#define SKYSEARCH_H

#include <type_traits>

#include <QVector>

#include "TimeVariableData.h"
#include "AircraftData.h"

namespace SkySearch {

    inline constexpr int InvalidIndex = -1;
    inline constexpr int LinearIntervalSearch = -2;
    inline constexpr int BinaryIntervalSearch = -3;

    /*!
     * In case we seek 3 seconds "into the future" we use binary search
     * to find the next position (otherwise linear search, assuming that
     * the next position is "nearby" (within the 3 seconds threshold).
     */
    inline constexpr qint64 BinaryIntervalSearchThreshold = 3000;

    /*!
     * The size of the interpolation window [-TimestampWindows, TimestampWindow] [milliseconds].
     */
    inline constexpr qint64 TimestampWindow = 1000;

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
     * \return the lower index i of the interval [i, j], or \c InvalidIndex if not found
     */
    template <typename T>
    int binaryIntervalSearch(const QVector<T> &data, qint64 timestamp, int lowIndex, int highIndex) noexcept
    {
        int index;
        if (data.size() == 0) {
            index = InvalidIndex;
        } else if (data.at(lowIndex).timestamp > timestamp) {
            index = InvalidIndex;
        } else if (data.at(highIndex).timestamp < timestamp) {
            index = InvalidIndex;
        } else if (data.at(0).timestamp == timestamp) {
            index = 0;
        } else if (data.constLast().timestamp == timestamp) {
            index = data.size() - 1;
        } else {

            index = InvalidIndex;
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
    int linearIntervalSearch(const QVector<T> &data, qint64 timestamp, int startIndex) noexcept
    {
        int index;
        if (data.size() == 0) {
            index = InvalidIndex;
        } else if (data.at(startIndex).timestamp > timestamp) {
            index = InvalidIndex;
        } else if (data.constLast().timestamp < timestamp) {
            index = InvalidIndex;
        } else {
            index = startIndex;
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
     *  Updates the \c startIndex with the last index having a timestamp <= the given \c timestamp.
     */
    template <typename T>
    int updateStartIndex(QVector<T> data, int startIndex, qint64 timestamp) noexcept
    {
        int index = startIndex;
        int size = data.size();
        if (size > 0) {
            if (timestamp < data.last().timestamp) {
                if (index != InvalidIndex) {
                    if (timestamp < data.at(index).timestamp) {
                        // The timestamp was moved to front ("rewind"), so search the
                        // array until and including the current index
                        index = BinaryIntervalSearch;
                    } else if (timestamp - BinaryIntervalSearchThreshold > data.at(index).timestamp) {
                        index = BinaryIntervalSearch;
                    }
                } else {
                    // Current index not yet initialised, so search the entire array
                    index = BinaryIntervalSearch;
                }
            } else {
                // The given timestamp lies past the last sample data
                // -> return the last sample data
                index = data.count() - 1;
            }
        } else {
            // No data yet, or timestamp not between given range
            index = InvalidIndex;
        }

        if (index != InvalidIndex && index != (data.count() - 1)) {

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
                int low;
                int high;
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
                } else {
                    // index not yet initialised -> search entire timeline
                    low = 0;
                    high = data.size() - 1;
                }
                index = binaryIntervalSearch(data, timestamp, low, high);
            }

        }
        return index;
    }

    template <typename T>
    bool getSupportData(const QVector<T> data, qint64 timestamp, int &startIndex, const T **p0, const T **p1, const T **p2, const T **p3) noexcept
    {
        static_assert(std::is_base_of<TimeVariableData, T>::value, "T not derived from TimeVariableData");

        startIndex = updateStartIndex(data, startIndex, timestamp);
        if (startIndex != InvalidIndex) {

            *p1 = &data.at(startIndex);
            // Is p1 within the timestamp window?
            if ((timestamp - (*p1)->timestamp) <= TimestampWindow) {

                if (startIndex > 0) {
                   *p0 = &data.at(startIndex - 1);
                } else {
                   *p0 = *p1;
                }
                if (startIndex < data.count() - 1) {
                   if (startIndex < data.count() - 2) {
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

                // Is p2 within the timestamp window?
                if (((*p2)->timestamp - timestamp) > TimestampWindow) {
                    *p2 = *p3 = *p1;
                }

            } else {
                *p0 = *p1 = *p2 = *p3 = nullptr;
            }

        } else {
            // We are past the last sample point
            if (data.count() > 0 && timestamp - data.last().timestamp <= TimestampWindow) {
                *p0 = *p1 = *p2 = *p3 = &data.last();
            } else {
                *p0 = *p1 = *p2 = *p3 = nullptr;
            }
        }

        return *p0 != nullptr;
    }

    double normaliseTimestamp(const TimeVariableData &p1, const TimeVariableData &p2, quint64 timestamp) noexcept;

} // namespace

#endif // SKYSEARCH_H
