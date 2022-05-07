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
#ifndef ABSTRACTCOMPONENT_H
#define ABSTRACTCOMPONENT_H

#include <memory>
#include <vector>
#include <iterator>
#include <cstdint>
#include <type_traits>

#include "TimeVariableData.h"
#include "AircraftInfo.h"
#include "ModelLib.h"

template <typename T>
class MODEL_API AbstractComponent
{
    static_assert(std::is_base_of<TimeVariableData, T>::value, "T must inherit from TimeVariableData");
public:
    AbstractComponent(const AircraftInfo &aircraftInfo) noexcept;

    /*!
     * Inserts \c data at the end, or updates the \em last element (only) if
     * the data items have the same timestamp.
     *
     * Use case: recorded data items are inserted chronologically, but some recorded items
     * may have the same timestamp: the last recorded data item "wins".
     *
     * \param data
     *        the data to be upserted
     * \sa upsert
     */
    void upsertLast(const T data) noexcept
    {
        if (m_data.size() > 0 && m_data.back() == data)  {
            // Same timestamp -> replace
            m_data[m_data.size() - 1] = data;
        } else {
            m_data.push_back(data);
        }
    }

    /*!
     * Inserts \c data at the end, or updates the element having the same
     * timestamp. That is, the entire collection is being searched first.
     *
     * Use case: data items are inserted in random order ("flight augmentation");
     * use \c upsertLast in case items are to be inserted sequentially in order
     *
     * \param data
     *        the data to be upserted
     * \sa upsertLast
     */
    void upsert(const T data) noexcept
    {
        auto result = std::find_if(m_data.begin(), m_data.end(),
                                  [&data] (const TimeVariableData &d) { return d.timestamp == data.timestamp; });
        if (result != m_data.end()) {
            // Same timestamp -> update
            *result = data;
        } else {
            m_data.push_back(data);
        }
    }

    const T getFirst() const noexcept;
    const T getLast() const noexcept;
    [[nodiscard]]
    std::size_t count() const noexcept;
    void clear() noexcept;

    using Data = typename std::vector<T>;
    using Iterator = typename Data::iterator;
    using BackInsertIterator = typename std::back_insert_iterator<Data>;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;
    BackInsertIterator backInsertIterator() noexcept;

    T operator[](std::size_t index) noexcept;
    const T operator[](std::size_t index) const noexcept;

    virtual const T interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept = 0;


protected:
    AircraftInfo m_aircraftInfo;
    Data m_data;
};

#endif // ABSTRACTCOMPONENT_H
