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
#ifndef FLIGHT_H
#define FLIGHT_H

#include <memory>
#include <vector>
#include <iterator>
#include <cstdint>

#include <QObject>

class QDateTime;
class QString;

#include "Aircraft.h"
#include "ModelLib.h"

class FlightCondition;
class FlightPrivate;

class MODEL_API Flight : public QObject
{
    Q_OBJECT
public:
    Flight(QObject *parent = nullptr) noexcept;
    virtual ~Flight() noexcept;

    std::int64_t getId() const noexcept;
    void setId(std::int64_t id) noexcept;

    const QDateTime &getCreationTime() const noexcept;
    void setCreationTime(const QDateTime &creationTime) noexcept;

    const QString &getTitle() const noexcept;
    void setTitle(const QString &title) noexcept;

    const QString &getDescription() const noexcept;
    void setDescription(const QString &description) noexcept;

    void setAircraft(std::vector<std::unique_ptr<Aircraft>> aircraft) noexcept;
    Aircraft &addUserAircraft() noexcept;
    const Aircraft &getUserAircraftConst() const noexcept;
    Aircraft &getUserAircraft() const noexcept;
    int getUserAircraftIndex() const noexcept;
    void setUserAircraftIndex(int index) noexcept;
    std::int64_t deleteAircraftByIndex(int index) noexcept;
    std::size_t count() const noexcept;

    const FlightCondition &getFlightConditionConst() const noexcept;
    void setFlightCondition(FlightCondition flightCondition) noexcept;

    std::int64_t getTotalDurationMSec(bool ofUserAircraft = false) const noexcept;
    QDateTime getAircraftCreationTime(const Aircraft &aircraft) const noexcept;
    QDateTime getAircraftStartLocalTime(const Aircraft &aircraft) const noexcept;
    QDateTime getAircraftStartZuluTime(const Aircraft &aircraft) const noexcept;

    void clear(bool withOneAircraft) noexcept;

    typedef std::vector<std::unique_ptr<Aircraft>>::iterator Iterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;

    Aircraft& operator[](std::size_t index) noexcept;
    const Aircraft& operator[](std::size_t index) const noexcept;

    static constexpr int InvalidId = -1;

signals:
    void flightChanged();
    void flightConditionChanged();
    void aircraftAdded(Aircraft &aircraft);
    void aircraftDeleted(std::int64_t removedSimulationObjectId);
    void aircraftInfoChanged(Aircraft &aircraft);
    void tailNumberChanged(Aircraft &aircraft);
    void timeOffsetChanged(Aircraft &aircraft);
    void userAircraftChanged(Aircraft &aircraft);
    void flightStored(std::int64_t id);
    void aircraftStored(Aircraft &aircraft);

private:
    Q_DISABLE_COPY(Flight)
    std::unique_ptr<FlightPrivate> d;

    inline void connectWithAircraftSignals(Aircraft &aircraft);
};

#endif // FLIGHT_H
