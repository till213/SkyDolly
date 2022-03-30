/**
 * Sky Dolly - The black sheep for your fposition recordings
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
#ifndef POSITION_H
#define POSITION_H

#include <memory>
#include <vector>
#include <iterator>
#include <cstdint>

#include <QObject>

#include "TimeVariableData.h"
#include "AircraftInfo.h"
#include "ModelLib.h"

struct PositionData;
class PositionPrivate;

class MODEL_API Position : public QObject
{
    Q_OBJECT
public:
    Position(const AircraftInfo &aircraftInfo, QObject *parent = nullptr) noexcept;
    virtual ~Position() noexcept;

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
    void upsertLast(const PositionData &data) noexcept;

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
    void upsert(const PositionData &data) noexcept;
    const PositionData &getFirst() const noexcept;
    const PositionData &getLast() const noexcept;
    std::size_t count() const noexcept;
    const PositionData &interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept;
    void clear() noexcept;

    typedef std::vector<PositionData>::iterator Iterator;
    typedef std::back_insert_iterator<std::vector<PositionData>> BackInsertIterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;
    BackInsertIterator backInsertIterator() noexcept;

    PositionData& operator[](std::size_t index) noexcept;
    const PositionData& operator[](std::size_t index) const noexcept;

signals:
    void dataChanged();

private:
    Q_DISABLE_COPY(Position)
    std::unique_ptr<PositionPrivate> d;
};

#endif // POSITION_H
