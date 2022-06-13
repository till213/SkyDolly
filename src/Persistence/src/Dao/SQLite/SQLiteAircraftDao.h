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
#ifndef SQLITEAIRCRAFTDAO_H
#define SQLITEAIRCRAFTDAO_H

#include <memory>
#include <vector>
#include <cstdint>
#include <iterator>
#include <cstddef>

#include <QtGlobal>

#include <Model/Aircraft.h>
#include "../AircraftDaoIntf.h"

class SQLiteAircraftDaoPrivate;

class SQLiteAircraftDao : public AircraftDaoIntf
{
public:
    explicit SQLiteAircraftDao() noexcept;
    virtual ~SQLiteAircraftDao() noexcept;

    virtual bool add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) noexcept override;
    virtual bool getByFlightId(std::int64_t flightId, std::back_insert_iterator<std::vector<std::unique_ptr<Aircraft>>> backInsertIterator) const noexcept override;
    virtual bool adjustAircraftSequenceNumbersByFlightId(std::int64_t flightId, std::size_t sequenceNumber) noexcept override;
    virtual bool deleteAllByFlightId(std::int64_t flightId) noexcept override;
    virtual bool deleteById(std::int64_t id) noexcept override;
    virtual bool getAircraftInfosByFlightId(std::int64_t flightId, std::vector<AircraftInfo> &aircraftInfos) const noexcept override;
    virtual bool updateTimeOffset(std::int64_t id, std::int64_t timeOffset) noexcept override;
    virtual bool updateTailNumber(std::int64_t id, const QString &tailNumber) noexcept override;

private:
    std::unique_ptr<SQLiteAircraftDaoPrivate> d;
};

#endif // SQLITEAIRCRAFTDAO_H
