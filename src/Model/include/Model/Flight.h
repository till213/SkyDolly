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
#include "FlightCondition.h"
#include "ModelLib.h"

struct FlightPrivate;

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

    /*!
     * Returns the index of the \c aircraft.
     *
     * \return the index of the \c aircraft; indexing starts at 0
     */
    int getAircraftIndex(const Aircraft &aircraft) const noexcept;

    /*!
     * Returns the index of the user aircraft.
     *
     * \return the index of the user aircraft; indexing starts at 0
     */
    int getUserAircraftIndex() const noexcept;

    /*!
     * Sets the user aircraft index to \c index and emits the signal #userAircraftChanged, but the
     * second signal parameter (\c previousUserAircraftIndex) is set to \c InvalidIndex.
     *
     * This implies that while the previous AI object for the new user aircraft (identified by the
     * new \c index) is deleted no new AI object for the previous user aircraft is created. This
     * is useful when deleting the current user aircraft itself.
     *
     * In effect, one AI object is deleted and none created.
     *
     * \param index
     *        the index of the new user aircraft
     * \sa switchUserAircraftIndex
     * \sa reassignUserAircraftIndex
     */
    void setUserAircraftIndex(int index) noexcept;

    /*!
     * Sets the user aircraft index to \c index and emits the signal #userAircraftChanged. In contrast
     * to #setUserAircraftIndex however the second signal parameter (\c previousUserAircraftIndex) is set
     * to the previous user aircraft index.
     *
     * This implies that the previous AI object for the new user aircraft (identified by the
     * new \c index) is deleted and a new AI object for the previous user aircraft is created. Or in
     * other words, the user aircraft is switched to the position of another AI object.
     *
     * * In effect, one AI object is deleted and another one is created.
     *
     * \param index
     * \sa setUserAircraftIndex
     * \sa reassignUserAircraftIndex
     */
    void switchUserAircraftIndex(int index) noexcept;

    std::int64_t deleteAircraftByIndex(int index) noexcept;
    std::size_t count() const noexcept;

    const FlightCondition &getFlightCondition() const noexcept;
    void setFlightCondition(FlightCondition flightCondition) noexcept;

    FlightSummary getFlightSummary() const noexcept;

    /*!
     * Returns the total duration of the flight [in milliseconds], that is it returns
     * the longest replay time of all aircraft, taking their time offsets into account.
     * Unless \c ofUserAircraft is set to \c true, in which case the replay time of the
     * \e user aircraft is returned.
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

    using Iterator = std::vector<std::unique_ptr<Aircraft>>::iterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;

    Aircraft &operator[](std::size_t index) noexcept;
    const Aircraft &operator[](std::size_t index) const noexcept;

    /*!
     * The initial ID for every newly created flight. An invalid ID indicates that this
     * flight has not yet been (successfully) persisted.
     */
    static constexpr std::int64_t InvalidId {-1};
    static constexpr int InvalidAircraftIndex {-1};

signals:
    void flightStored(std::int64_t id);
    void flightRestored(std::int64_t id);

    void cleared();
    void descriptionOrTitleChanged();
    void flightConditionChanged();

    void aircraftAdded(const Aircraft &newAircraft);
    void aircraftRemoved(std::int64_t removedAircraftId);

    /*!
     * Emitted whenever the user aircraft index is changed to \c newUserAircraftIndex. In case a previous user aircraft
     * existed the \c previousUserAircraftIndex is set accordingly, otherwise it is set to InvalidAircraftIndex.
     *
     * \param newUserAircraftIndex
     *        the index of the new user aircraft
     * \param previousUserAircraftIndex
     *        the index of the previous user aircraft; InvalidAircraftIndex if no previous user aircraft was set
     */
    void userAircraftChanged(int newUserAircraftIndex, int previousUserAircraftIndex);

    /*!
     * Emited whenever any of the aircraft info data has changed, including the tail number and
     * time offset.
     *
     * \param aircraft
     *        the Aircraft of which the info has changed
     * \sa tailNumberChanged
     * \sa timeOffsettChanged
     */
    void aircraftInfoChanged(const Aircraft &aircraft);
    void tailNumberChanged(const Aircraft &aircraft);

    /*!
     * Emitted whenever the time offset of the \c aircraft has changed.
     *
     * \param aircraft
     *        the aircraft whose time offset has changed
     */
    void timeOffsetChanged(const Aircraft &aircraft);

    void aircraftStored(const Aircraft &aircraft);

private:
    std::unique_ptr<FlightPrivate> d;

    inline void connectWithAircraftSignals(Aircraft &aircraft);

    /*
     * Re-assigns the user aircraft \c index, but without emitting the \c userAircraftChanged signal.
     * This is useful in case an aircraft with an index lower (<) than the current user aircraft
     * is deleted and hence the user aircraft index must be re-assigned, but without actually
     * switching the user aircraft to a previous AI object.
     */
    void reassignUserAircraftIndex(std::int64_t index) noexcept;
};

#endif // FLIGHT_H
