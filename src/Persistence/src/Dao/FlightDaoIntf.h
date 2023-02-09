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
#ifndef FLIGHTDAOINTF_H
#define FLIGHTDAOINTF_H

#include <cstdint>

class QString;

struct FlightData;
class FlightSummary;

class FlightDaoIntf
{
public:
    FlightDaoIntf() = default;
    FlightDaoIntf(const FlightDaoIntf &rhs) = delete;
    FlightDaoIntf(FlightDaoIntf &&rhs) = default;
    FlightDaoIntf &operator=(const FlightDaoIntf &rhs) = delete;
    FlightDaoIntf &operator=(FlightDaoIntf &&rhs) = default;
    virtual ~FlightDaoIntf() = default;

    /*!
     * Persists the \c flight. The \c id in \c flight is updated.
     *
     * \param flightData
     *        the FlightData to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(FlightData &flightData) noexcept = 0;

    /*!
     * Exports the \c flight. While a new \c id for the exported Flight
     * is generated the \c id of the given \c flight (to be exported) is not updated.
     *
     * \param flightData
     *        the FlightData to be exported (persisted)
     * \return \c true on success; \c false else
     */
    virtual bool exportFlightData(const FlightData &flightData) noexcept = 0;
    virtual bool get(std::int64_t id, FlightData &flightData) const noexcept = 0;
    virtual bool deleteById(std::int64_t id) noexcept = 0;
    virtual bool updateTitle(std::int64_t id, const QString &title) noexcept = 0;
    virtual bool updateTitleAndDescription(std::int64_t id, const QString &title, const QString &description) noexcept = 0;
    virtual bool updateUserAircraftIndex(std::int64_t id, int index) noexcept = 0;
};

#endif // FLIGHTDAOINTF_H
