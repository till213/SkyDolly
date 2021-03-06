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
#ifndef PRIMARYFLIGHTCONTROL_H
#define PRIMARYFLIGHTCONTROL_H

#include <memory>
#include <vector>
#include <iterator>

#include <QObject>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct PrimaryFlightControlData;
class PrimaryFlightControlPrivate;

class MODEL_API PrimaryFlightControl : public QObject
{
    Q_OBJECT
public:
    PrimaryFlightControl(QObject *parent = nullptr) noexcept;
    virtual ~PrimaryFlightControl() noexcept;

    void upsert(const PrimaryFlightControlData &primaryFlightControlData) noexcept;
    const PrimaryFlightControlData &getFirst() const noexcept;
    const PrimaryFlightControlData &getLast() const noexcept;
    std::size_t count() const noexcept;
    const PrimaryFlightControlData &interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept;
    void clear() noexcept;

    typedef std::vector<PrimaryFlightControlData>::iterator Iterator;
    typedef std::insert_iterator<std::vector<PrimaryFlightControlData>> InsertIterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;
    InsertIterator insertIterator() noexcept;

    PrimaryFlightControlData& operator[](std::size_t index) noexcept;
    const PrimaryFlightControlData& operator[](std::size_t index) const noexcept;

signals:
    void dataChanged();

private:
    Q_DISABLE_COPY(PrimaryFlightControl)
    std::unique_ptr<PrimaryFlightControlPrivate> d;
};

#endif // PRIMARYFLIGHTCONTROL_H
