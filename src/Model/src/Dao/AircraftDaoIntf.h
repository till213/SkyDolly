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
#ifndef AIRCRAFTDAOINTF_H
#define AIRCRAFTDAOINTF_H

#include <QtGlobal>

class Aircraft;

class AircraftDaoIntf
{
public:
    virtual ~AircraftDaoIntf() = default;

    /*!
     * Persists the \c aircraft. The \c id in \c aircraft is updated.
     *
     * \param scenarioId
     *        the scenario the \c aircraft belongs to
     * \param sequenceNumber
     *        the sequence number of the aircraft
     * \param aircraft
     *        the aircraft to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(qint64 scenarioId, int sequenceNumber, Aircraft &aircraft) = 0;
    virtual bool getById(qint64 id, Aircraft &aircraft) const = 0;
    virtual bool getByScenarioId(qint64 scenarioId, int sequenceNumber, Aircraft &aircraft) const = 0;
};

#endif // AIRCRAFTDAOINTF_H
