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

#include <QVector>

#include "AircraftData.h"

namespace SkySearch {

    constexpr int InvalidIndex = -1;
    constexpr int LinearIntervalSearch = -2;
    constexpr int BinaryIntervalSearch = -3;

    /*!
     * Returns the lower index i of the interval [i, j] where i.timestamp <= timestamp < j.timestamp.
     * A special case is the last index l in case l.timestamp =< timestamp, in which case the last index l
     * is returned.
     *
     * The interval is assumed to be within [lowIndex, highIndex] and searched with an adapted
     * binary search (O(log(N)) time).
     *
     * \param aircraftData
     *        the array of data, with ascending timestamps
     * \param timestamp
     *        the timestamp within the interval to be searched
     * \param lowIndex
     *        the lower starting index >= 0 and <= highIndex
     * \param highIndex
     *        the higher starting index <= lastIndex and >= lowIndex
     * \return the lower index i of the interval [i, j], or \c InvalidIndex if not found
     */
    inline int binaryIntervalSearch(const QVector<AircraftData> &aircraftData, qint64 timestamp, int lowIndex, int highIndex)
    {
        int index;
        if (aircraftData.size() == 0) {
            index = InvalidIndex;
        } else if (aircraftData.at(0).timestamp >= timestamp) {
            index = 0;
        } else if (aircraftData.constLast().timestamp <= timestamp) {
            index = aircraftData.size() - 1;
        } else {

            int low = lowIndex, high = highIndex;
            while (low <= high)
            {
                int mid = (low + high) / 2;
                if (aircraftData.at(mid).timestamp <= timestamp && (mid == high || timestamp < aircraftData.at(mid + 1).timestamp)) {
                    // Found the interval [mid].timestamp <= timestamp < [mid+1].timestamp
                    index = mid;
                    break;
                }
                else if (timestamp < aircraftData.at(mid).timestamp) {
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

    inline int linearIntervalSearch(const QVector<AircraftData> &aircraftData, qint64 timestamp, int startIndex)
    {
        int index = startIndex;
        int size = aircraftData.size();
        // Linear search: increment the current index, until we find a position having a
        // timestamp > the given timestamp
        bool found = false;
        while (!found && index < size) {
            if (index < (size - 1)) {
                if (aircraftData.at(index + 1).timestamp > timestamp) {
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

        return index;
    }

} // namespace

#endif // SKYSEARCH_H
