/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef FLIGHTSERVICE_H
#define FLIGHTSERVICE_H

#include <memory>
#include <cstdint>

#include <QSqlDatabase>

#include <Kernel/Const.h>
#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include "../PersistenceLib.h"

class Flight;
struct FlightData;
class SkyConnectIntf;
struct FlightServicePrivate;

class PERSISTENCE_API FlightService final
{
public:
    FlightService(QString connectionName = Const::DefaultConnectionName) noexcept;
    FlightService(const FlightService &rhs) = delete;
    FlightService(FlightService &&rhs) noexcept;
    FlightService &operator=(const FlightService &rhs) = delete;
    FlightService &operator=(FlightService &&rhs) noexcept;
    ~FlightService();

    /*!
     * Stores the FlightData of the given \p flight and emits the Flight#flightStored signal
     * upon success. The \p id of the Flight is updated accordingly.
     *
     * \param flight
     *        the flight that will be stored; typically the \e current flight of the Logbook
     * \return \c true upon success; \c false else
     * \sa storeFlightData
     * \sa Flight#flightStored
     */
    bool storeFlight(Flight &flight) noexcept;

    /*!
     * Stores the \p flightData, but does not emit any signal. The \p id of the FlightData is
     * updated accordingly.
     *
     * \param flightData
     *        the flight data that will be stored; typically data that has just been imported
     *        from some other data source (logbook)
     * \return \c true upon success; \c false else
     * \sa storeFlight
     * \sa exportFlightData
     */
    bool storeFlightData(FlightData &flightData) noexcept;

    /*!
     * Exports the \p flightData, but does not emit any signal. The \p id of the FlightData is
     * left unchanged (possibly still Const#InvalidId).
     *
     * \param flightData
     *        the flight data that will be exported, typically into another logbook
     * \return \c true upon success; \c false else
     * \sa storeFlightData
     */
    bool exportFlightData(const FlightData &flightData) noexcept;

    /*!
     * Restores the Flight identified by \p id into \p flight and emits the Flight#flightRestored
     * signal upon success.
     *
     * \param id
     *        the id of the Flight to be restored
     * \param flight
     *        the Flight that will contain the restored data; typically the \e current
     *        flight of the Logbook
     * \return \c true upon success; \c false else
     * \sa importFlightData
     * \sa Flight#flightRestored
     */
    bool restoreFlight(std::int64_t id, Flight &flight) noexcept;

    /*!
     * Imports the FlightData identified by \p id into \p flightData, but does not emit any signal.
     *
     * \param flightData
     *        the flight data that will be imported, typically from another logbook
     * \return \c true upon success; \c false else
     * \sa restoreFlight
     */
    bool importFlightData(std::int64_t id, FlightData &flightData) const noexcept;
    bool deleteById(std::int64_t id) const noexcept;
    bool updateTitle(Flight &flight, const QString &title) const noexcept;
    bool updateTitle(std::int64_t id, const QString &title) const noexcept;
    bool updateFlightNumber(Flight &flight, const QString &flightNumber) const noexcept;
    bool updateFlightNumber(std::int64_t id, const QString &flightNumber) const noexcept;
    bool updateDescription(Flight &flight, const QString &description) const noexcept;
    bool updateDescription(std::int64_t id, const QString &description) const noexcept;
    bool updateUserAircraftIndex(Flight &flight, int index) const noexcept;

private:
    std::unique_ptr<FlightServicePrivate> d;
};

#endif // FLIGHTSERVICE_H
