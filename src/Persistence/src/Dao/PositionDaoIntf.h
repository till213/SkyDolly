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
#ifndef POSITIONDAOINTF_H
#define POSITIONDAOINTF_H

#include <QVector>

class AircraftData;

class PositionDaoIntf
{
public:
    virtual ~PositionDaoIntf() = default;

    /*!
     * Persists the \c data.
     *
     * \param aircraftId
     *        the aircraft the \c data belongs to
     * \param data
     *        the AircraftData to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(qint64 aircraftId, const AircraftData &data) = 0;
    virtual bool getByAircraftId(qint64 aircraftId, QVector<AircraftData> &data) const = 0;
};

#endif // POSITIONDAOINTF_H
