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
#ifndef FLIGHT_H
#define FLIGHT_H

#include <memory>

#include <QObject>

class QDateTime;
class QString;

#include "ModelLib.h"

class FlightCondition;
class Aircraft;
class FlightPrivate;

class MODEL_API Flight : public QObject
{
    Q_OBJECT
public:
    Flight(QObject *parent = nullptr) noexcept;
    ~Flight() noexcept;

    qint64 getId() const noexcept;
    void setId(qint64 id) noexcept;

    const QDateTime &getCreationDate() const noexcept;
    void setCreationDate(const QDateTime &creationDate) noexcept;

    const QString &getTitle() const noexcept;
    void setTitle(const QString &title) noexcept;

    const QString &getDescription() const noexcept;
    void setDescription(const QString &description) noexcept;

    const Aircraft &getUserAircraftConst() const noexcept;
    Aircraft &getUserAircraft() const noexcept;

    const FlightCondition &getFlightConditionConst() const noexcept;
    void setFlightCondition(FlightCondition flightCondition) noexcept;

    qint64 getTotalDurationMSec() const noexcept;

    void clear() noexcept;

     static constexpr int InvalidId = -1;

signals:
    void aircraftInfoChanged();
    void aircraftDataChanged();
    void flightConditionChanged();

private:
    Q_DISABLE_COPY(Flight)
    std::unique_ptr<FlightPrivate> d;

    void frenchConnection() noexcept;
};

#endif // FLIGHT_H
