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
#ifndef AIRCRAFTHANDLE_H
#define AIRCRAFTHANDLE_H

#include <memory>
#include <vector>
#include <iterator>

#include <QObject>

#include "TimeVariableData.h"
#include "ModelLib.h"

class AircraftHandleData;
class AircraftHandlePrivate;

class MODEL_API AircraftHandle : public QObject
{
    Q_OBJECT
public:
    AircraftHandle(QObject *parent = nullptr) noexcept;
    virtual ~AircraftHandle() noexcept;

    void upsert(const AircraftHandleData &aircraftHandleData) noexcept;
    const AircraftHandleData &getFirst() const noexcept;
    const AircraftHandleData &getLast() const noexcept;
    std::size_t count() const noexcept;
    const AircraftHandleData &interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept;
    void clear() noexcept;

    typedef std::vector<AircraftHandleData>::iterator Iterator;
    typedef std::insert_iterator<std::vector<AircraftHandleData>> InsertIterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;
    InsertIterator insertIterator() noexcept;

    AircraftHandleData& operator[](std::size_t index) noexcept;
    const AircraftHandleData& operator[](std::size_t index) const noexcept;

signals:
    void dataChanged();

private:
    Q_DISABLE_COPY(AircraftHandle)
    std::unique_ptr<AircraftHandlePrivate> d;
};

#endif // AIRCRAFTHANDLE_H
