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
#ifndef ENGINE_H
#define ENGINE_H

#include <memory>
#include <vector>
#include <iterator>

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "TimeVariableData.h"
#include "ModelLib.h"

class EngineData;
class EnginePrivate;

class MODEL_API Engine : public QObject
{
    Q_OBJECT
public:
    Engine(QObject *parent = nullptr) noexcept;
    virtual ~Engine() noexcept;

    void upsert(const EngineData &engineData) noexcept;
    const EngineData &getFirst() const noexcept;
    const EngineData &getLast() const noexcept;
    std::size_t count() const noexcept;
    const EngineData &interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept;
    void clear() noexcept;

    typedef std::vector<EngineData>::iterator Iterator;
    typedef std::insert_iterator<std::vector<EngineData>> InsertIterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;
    InsertIterator insertIterator() noexcept;

    EngineData& operator[](std::size_t index) noexcept;
    const EngineData& operator[](std::size_t index) const noexcept;

signals:
    void dataChanged();

private:
    Q_DISABLE_COPY(Engine)
    std::unique_ptr<EnginePrivate> d;
};

#endif // ENGINE_H
