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
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <memory>

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "ModelLib.h"
#include "AircraftInfo.h"
#include "Engine.h"

class AircraftData;
class AircraftPrivate;

class MODEL_API Aircraft : public QObject
{
    Q_OBJECT
public:    
    Aircraft(QObject *parent = nullptr) noexcept;
    virtual ~Aircraft() noexcept;

    const Engine &getEngineConst() const;
    Engine &getEngine() const;

    void setAircraftInfo(AircraftInfo aircraftInfo) noexcept;
    const AircraftInfo &getAircraftInfo() const noexcept;

    void upsertAircraftData(AircraftData aircraftData) noexcept;
    const AircraftData &getLastAircraftData() const noexcept;
    const QVector<AircraftData> getAllAircraftData() const noexcept;
    const AircraftData &interpolateAircraftData(qint64 timestamp) const noexcept;

    void clear();

signals:
    void infoChanged();
    void dataChanged();

private:
    Q_DISABLE_COPY(Aircraft)
    std::unique_ptr<AircraftPrivate> d;

    bool updateCurrentIndex(qint64 timestamp) const noexcept;
    bool getSupportData(qint64 timestamp, const AircraftData **p0, const AircraftData **p1, const AircraftData **p2, const AircraftData **p3) const noexcept;
    static double normaliseTimestamp(const AircraftData &p1, const AircraftData &p2, quint64 timestamp) noexcept;
};

#endif // AIRCRAFT_H
