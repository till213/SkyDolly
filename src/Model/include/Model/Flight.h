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
#include "FlightSummary.h"
#include "ModelLib.h"

class FlightCondition;
class FlightPrivate;

class MODEL_API Flight : public QObject
{
    Q_OBJECT
public:
    explicit Flight(QObject *parent = nullptr) noexcept;
    ~Flight() noexcept override;

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
    Aircraft &getUserAircraft() const noexcept;
    int getUserAircraftIndex() const noexcept;
    void setUserAircraftIndex(int index) noexcept;
    std::int64_t deleteAircraftByIndex(int index) noexcept;
    std::size_t count() const noexcept;

    const FlightCondition &getFlightCondition() const noexcept;
    void setFlightCondition(FlightCondition flightCondition) noexcept;

    FlightSummary getFlightSummary() const noexcept;

    /*!
     * Returns the total duration of the flight [in milliseconds], that is it returns
     * the longest replay time of all aircraft, taking their time offsets into account.
     * Unless \c ofUserAircraft is set to \c true, in which case the replay time of the
     * \em user aircraft is returned.
     *
     * Note that the total duration is cached and not updated during recording. Use
     * the SkyConnectIntf#getCurrentTimestamp in this case, which - during recording - indicates
     * the current recorded duration (for the user aircraft).
     *
     * \param ofUserAircraft
     *        set to \c true in case to specifically query the replay duration
     *        of the user aircraft; \c false in order to query the total duration
     *        for the flight; default: \c false
     * \return the total replay duration of the flight, or the replay duration of
     *         the user aircraft
     */
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

    /*!
     * The initial ID for every newly created flight. An invalid ID indicates that this
     * flight has not yet been (successfully) persisted.
     */
    static constexpr int InvalidId = -1;

signals:
    void flightStored(std::int64_t id);
    void flightRestored(std::int64_t id);

    void cleared();
    void descriptionOrTitleChanged();
    void flightConditionChanged();

    void aircraftAdded(Aircraft &newAircraft);
    void aircraftRemoved(std::int64_t removedAircraftId);
    void userAircraftChanged(Aircraft &newUserAircraft);

    void aircraftInfoChanged(Aircraft &aircraft);
    void tailNumberChanged(Aircraft &aircraft);
    void timeOffsetChanged(Aircraft &aircraft);

    void aircraftStored(Aircraft &aircraft);

private:
    Q_DISABLE_COPY(Flight)
    std::unique_ptr<FlightPrivate> d;

    inline void connectWithAircraftSignals(Aircraft &aircraft);
};

#endif // FLIGHT_H
