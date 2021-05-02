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
#ifndef SCENARIODAOINTF_H
#define SCENARIODAOINTF_H

#include <QVector>

class Flight;
class FlightDescription;

class FlightDaoIntf
{
public:
    virtual ~FlightDaoIntf() = default;

    /*!
     * Persists the \c flight. The \c id in \c flight is updated.
     * \param flight
     *        the Flight to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool addFlight(Flight &flight) = 0;
    virtual bool getFlightById(qint64 id, Flight &flight) const = 0;
    virtual bool deleteById(qint64 id) = 0;
    virtual QVector<FlightDescription> getFlightDescriptions() const = 0;
};

#endif // SCENARIODAOINTF_H
