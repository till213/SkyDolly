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

#include <Kernel/SkyMath.h>
#include "Aircraft.h"
#include "FlightSummary.h"
#include "FlightCondition.h"
#include "FlightData.h"
#include "ModelLib.h"

struct Waypoint;
struct FlightPrivate;

class MODEL_API Flight final : public QObject
{
    Q_OBJECT
public:
    explicit Flight(FlightData flightData, QObject *parent = nullptr) noexcept;
    explicit Flight(QObject *parent = nullptr) noexcept;    
    Flight(const Flight &rhs) = delete;
    Flight(Flight &&rhs) = delete;
    Flight &operator=(const Flight &rhs) = delete;
    Flight &operator=(Flight &&rhs) = delete;
    ~Flight() override;

    /*!
     * Restores this Flight from the given \c flightData.
     *
     * \param flightData
     *        the FlightData to set
     * \sa flightRestored
     */
    void fromFlightData(FlightData &&flightData) noexcept;
    FlightData &getFlightData() const noexcept;

    std::int64_t getId() const noexcept;
    void setId(std::int64_t id) noexcept;

    /*!
     * In order to reset the creation time clear this Flight.
     *
     * \return the creation date & time when the first aircraft was recorded; an invalid
     *         QDateTime when no recording has been done yet
     * \sa clear
     */
    const QDateTime &getCreationTime() const noexcept;

    const QString &getTitle() const noexcept;
    void setTitle(QString title) noexcept;

    const QString &getDescription() const noexcept;
    void setDescription(QString description) noexcept;

    /*!
     * Adds all \c aircraft to the existing Aircraft of this Flight.
     * For each aircraft the signal aircraftAdded is emitted.
     *
     * \param aircraft
     *        the aircraft collection to be added to this Flight
     * \sa aircraftAdded
     */
    void addAircraft(std::vector<Aircraft> &&aircraft) noexcept;

    /*!
     * Creates and adds a new user Aircraft to this Flight.
     *
     * \return the newly created Aircraft
     * \sa aircraftAdded
     */
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
     * second signal parameter (\c previousUserAircraftIndex) is set to \c Const#InvalidIndex.
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

    /*!
     * Removes the aircraft identified by its \c index.
     *
     * \param index
     *        the index of the aircraft to be removed
     * \return the ID of the removed Aircraft
     * \sa aircraftRemoved
     */
    std::int64_t removeAircraftByIndex(int index) noexcept;

    /*!
     * Removes the last (highest sequence number) aircraft from this flight.
     *
     * \return the ID of the removed Aircraft
     * \sa count
     * \sa aircraftRemoved
     */
    std::int64_t removeLastAircraft() noexcept;

    /*!
     * Returns the aircraft count of this Flight. Note that the count is usually
     * at least one (except in some intermediate, internal states, e.g. just before
     * restoring a new Flight from the Logbook), as a Flight always has at least
     * one \e user aircraft. However that does not mean that any sampled (recorded)
     * data exists.
     *
     * \return the number of Aircraft in this Flight; usually at least one
     * \sa getUserAircraft
     * \sa hasRecording
     */
    std::size_t count() const noexcept;

    /*!
     * Adds the \c waypoint to the flight plan of the user aircraft.
     *
     * \param waypoint
     *        the waypoint to be added to the flight plan of the user aircraft
     * \sa waypointAdded
     */
    void addWaypoint(const Waypoint &waypoint) noexcept;

    /*!
     * Updates the waypoint at \c index with the given \c waypoint.
     *
     * \param index
     *        the index of the waypoint to be updated
     * \param waypoint
     *        the waypoint data to update with
     * \sa waypointUpdated
     */
    void updateWaypoint(int index, const Waypoint &waypoint) noexcept;

    /*!
     * Clears all waypoints of the flight plan of the user aircraft.
     *
     * \sa waypointsCleared
     */
    void clearWaypoints() noexcept;

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

    void clear(bool withOneAircraft, FlightData::CreationTimeMode creationTimeMode) noexcept;

    /*!
     * Returns whether at least one Aircraft with sampled position data exists.
     *
     * \return \c true if at least one Aircraft has sampled position data; \c false else
     * \sa Aircraft#hasRecording
     */
    bool hasRecording() const noexcept;

    /*!
     * Synchronises the time offsets of each aircraft in the \c flightsToBeSynchronised according
     * to the creation time of this \e current Flight and the creation time of each \c flightsToBeSynchronised.
     *
     * \param timeOffsetSync
     *        defines how to synchronise the time offsets
     * \param flightsToBeSynchronised
     *        the flights to be synchronised, according to their creation time
     * \sa getCreationTime
     */
    void syncAircraftTimeOffset(SkyMath::TimeOffsetSync timeOffsetSync, std::vector<FlightData> &flightsToBeSynchronised) const noexcept;

    using Iterator = std::vector<Aircraft>::iterator;
    Iterator begin() noexcept;
    Iterator end() noexcept;

    using ConstIterator = std::vector<Aircraft>::const_iterator;
    ConstIterator begin() const noexcept;
    ConstIterator end() const noexcept;

    Aircraft &operator[](std::size_t index) noexcept;
    const Aircraft &operator[](std::size_t index) const noexcept;

signals:
    /*!
     * Emitted whenever one or several Flight have been stored to the logbook.
     */
    void flightStored();

    /*!
     * Emitted whenever the Flight given by its \c id has been restored from the logbook.
     *
     * \param id
     *        the id of the restored Flight
     */
    void flightRestored(std::int64_t id);

    /*!
     * Emitted whenever one or several Aircraft have been stored to the logbook. Typically
     * emitted whenever a new formation Aircraft has been stored, or imported and added
     * to the current Flight.
     */
    void aircraftStored();

    void cleared();
    void titleChanged(std::int64_t flightId, const QString &title);
    void descriptionChanged(std::int64_t flightId, const QString &description);
    void flightConditionChanged();

    /*!
     * Emitted whenever an \c aircraft has been added to this Flight. This is
     * typically called before creating a new formation flight and allows to
     * update existing "AI aircraft" models.
     *
     * \param aircraft
     *        the Aircraft that has been added to this Flight
     */
    void aircraftAdded(const Aircraft &aircraft);

    /*!
     * Emitted whenever an \c aircraft has been removed from this Flight. This is
     * typically called from the formation module and allows to update existing
     * "AI aircraft" models.
     *
     * \param aircraftId
     *        the id of the removed Aircraft
     */
    void aircraftRemoved(std::int64_t aircraftId);

    /*!
     * Emitted whenever a new \c waypoint has been added to the user aircraft.
     *
     * \param waypoint
     *        the newly added waypoint
     */
    void waypointAdded(const Waypoint &waypoint);

    /*!
     * Emitted whenever the \c waypoint of the user aircraft at \c index has been udpated.
     *
     * \param index
     *        the index of the updated waypoint
     * \param waypoint
     *        the updated waypoint
     */
    void waypointUpdated(int index, const Waypoint &waypoint);

    /*!
     * Emitted whenever all waypoints of the user aircraft have been cleared.
     */
    void waypointsCleared();

    /*!
     * Emitted whenever the user aircraft index is changed to \c newUserAircraftIndex. In case a previous user aircraft
     * existed the \c previousUserAircraftIndex is set accordingly, otherwise it is set to Const::InvalidIndex.
     *
     * \param newUserAircraftIndex
     *        the index of the new user aircraft
     * \param previousUserAircraftIndex
     *        the index of the previous user aircraft; Const::InvalidIndex if no previous user aircraft was set
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

    /*!
     * Emitted whenever the tail number of the \c aircraft has changed.
     *
     * \param aircraft
     *        the aircraft whose tail number has changed
     */
    void tailNumberChanged(const Aircraft &aircraft);

    /*!
     * Emitted whenever the time offset of the \c aircraft has changed.
     *
     * \param aircraft
     *        the aircraft whose time offset has changed
     */
    void timeOffsetChanged(const Aircraft &aircraft);

private:
    std::unique_ptr<FlightPrivate> d;

    void frenchConnection();

    /*
     * Re-assigns the user aircraft \c index, but without emitting the \c userAircraftChanged signal.
     * This is useful in case an aircraft with an index lower (<) than the current user aircraft
     * is deleted and hence the user aircraft index must be re-assigned, but without actually
     * switching the user aircraft to a previous AI object.
     */
    void reassignUserAircraftIndex(int index) noexcept;
};

#endif // FLIGHT_H
