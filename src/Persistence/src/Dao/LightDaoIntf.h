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
#ifndef LIGHTDAOINTF_H
#define LIGHTDAOINTF_H

#include <vector>
#include <iterator>

#include <QtGlobal>

class LightData;

class LightDaoIntf
{
public:
    virtual ~LightDaoIntf() = default;

    /*!
     * Persists the \c data.
     *
     * \param aircraftId
     *        the aircraft the \c data belongs to
     * \param data
     *        the LightData to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(qint64 aircraftId, const LightData &lightData) noexcept = 0;
    virtual bool getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<LightData>> insertIterator) const noexcept = 0;
    virtual bool deleteByFlightId(qint64 flightId) noexcept = 0;
    virtual bool deleteByAircraftId(qint64 aircraftId) noexcept = 0;
};

#endif // LIGHTDAOINTF_H
