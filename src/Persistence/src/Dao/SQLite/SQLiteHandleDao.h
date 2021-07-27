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
#ifndef SQLITEHANDLEDAO_H
#define SQLITEHANDLEDAO_H

#include <memory>
#include <vector>
#include <iterator>

#include <QtGlobal>

#include "../../../../Model/src/AircraftHandleData.h"
#include "../HandleDaoIntf.h"

class SQLiteHandleDao : public HandleDaoIntf
{
public:
    explicit SQLiteHandleDao() noexcept;
    virtual ~SQLiteHandleDao() noexcept;

    virtual bool add(qint64 aircraftId, const AircraftHandleData &data) noexcept override;
    virtual bool getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<AircraftHandleData>> insertIterator) const noexcept override;
    virtual bool deleteByFlightId(qint64 flightId) noexcept override;
    virtual bool deleteByAircraftId(qint64 aircraftId) noexcept override;
};


#endif // SQLITEHANDLEDAO_H
