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
#ifndef ABSTRACTCOMPONENT_H
#define ABSTRACTCOMPONENT_H

#include <memory>
#include <vector>
#include <iterator>
#include <cstdint>
#include <type_traits>

#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "ModelLib.h"

template <typename T>
class MODEL_API AbstractComponent
{
    static_assert(std::is_base_of<TimeVariableData, T>::value, "T must inherit from TimeVariableData");
public:
    using Data = typename std::vector<T>;
    using Iterator = typename Data::iterator;
    using ConstIterator = typename Data::const_iterator;

    explicit AbstractComponent(const AircraftInfo &aircraftInfo) noexcept
        : m_aircraftInfo(aircraftInfo)
    {}

    AbstractComponent() noexcept = default;
    AbstractComponent(const AbstractComponent &rhs) = default;
    AbstractComponent(AbstractComponent &&rhs) noexcept = default;
    AbstractComponent &operator=(const AbstractComponent &rhs) = default;
    AbstractComponent &operator=(AbstractComponent &&rhs) noexcept = default;
    virtual ~AbstractComponent() = default;

    void setData(const Data &data) noexcept
    {
        m_data = data;
    }

    void setData(Data &&data) noexcept
    {
        m_data = std::move(data);
    }

    /*!
     * Inserts \c data at the end, or updates the \e last element (only) if
     * the data items have the same timestamp.
     *
     * Use case: recorded data items are inserted chronologically, but some recorded items
     * may have the same timestamp: the last recorded data item "wins".
     *
     * \param data
     *        the data to be upserted
     * \sa upsert
     */
    void upsertLast(const T &data) noexcept
    {
        if (m_data.size() > 0 && m_data.back() == data)  {
            // Same timestamp -> replace
            m_data.back() = data;
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
    void upsert(const T &data) noexcept
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

    /*!
     * Returns the first data element. The collection must have at least one element,
     * otherwise the behaviour is undefined.
     *
     * \return the first element T of the collection
     */
    const T &getFirst() const noexcept
    {
        return m_data.front();
    }

    /*!
     * Returns the last data element. The collection must have at least one element,
     * otherwise the behaviour is undefined.
     *
     * \return the last element T of the collection
     */
    const T &getLast() const noexcept
    {
        return m_data.back();
    }

    std::size_t count() const noexcept
    {
        return m_data.size();
    }

    void reserve(typename Data::size_type size)
    {
        m_data.reserve(size);
    }

    void insert(typename Data::size_type count, const T &value)
    {
        m_data.insert(m_data.end(), count, value);
    }

    typename Data::size_type capacity() const noexcept
    {
        return m_data.capacity();
    }

    void clear() noexcept
    {
        m_data.clear();
        m_currentTimestamp = TimeVariableData::InvalidTime;
        m_currentIndex = SkySearch::InvalidIndex;
    }

    Iterator begin() noexcept
    {
        return m_data.begin();
    }

    ConstIterator begin() const noexcept
    {
        return m_data.begin();
    }

    Iterator end() noexcept
    {
        return m_data.end();
    }

    ConstIterator end() const noexcept
    {
        return m_data.end();
    }

    ConstIterator cbegin() const noexcept
    {
        return m_data.cbegin();
    }

    ConstIterator cend() const noexcept
    {
        return m_data.cend();
    }

    T &operator[](std::size_t index) noexcept
    {
        return m_data[index];
    }

    const T &operator[](std::size_t index) const noexcept
    {
        return m_data[index];
    }

    virtual const T &interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept = 0;

protected:
    inline const Data &getData() const noexcept
    {
        return m_data;
    }

    inline const AircraftInfo &getAircraftInfo() const noexcept
    {
        return m_aircraftInfo;
    }

    inline std::int64_t getCurrentTimestamp() const noexcept
    {
        return m_currentTimestamp;
    }
    inline void setCurrentTimestamp(std::int64_t timestamp) const noexcept
    {
        m_currentTimestamp = timestamp;
    }

    inline int getCurrentIndex() const noexcept
    {
        return m_currentIndex;
    }
    inline void setCurrentIndex(int index) const noexcept
    {
        m_currentIndex = index;
    }

    inline TimeVariableData::Access getCurrentAccess() const noexcept
    {
        return m_currentAccess;
    }
    inline void setCurrentAccess(TimeVariableData::Access access) const noexcept
    {
        m_currentAccess = access;
    }

private:
    Data m_data;
    const AircraftInfo &m_aircraftInfo;
    mutable std::int64_t m_currentTimestamp {TimeVariableData::InvalidTime};
    mutable int m_currentIndex {SkySearch::InvalidIndex};
    mutable TimeVariableData::Access m_currentAccess {TimeVariableData::Access::Linear};
};

#endif // ABSTRACTCOMPONENT_H
