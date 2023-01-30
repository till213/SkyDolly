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
#ifndef SQLITEENGINEDAO_H
#define SQLITEENGINEDAO_H

#include <memory>
#include <vector>
#include <cstdint>

class QSqlDatabase;

#include "../EngineDaoIntf.h"

struct EngineData;
struct SQLiteEngineDaoPrivate;

class SQLiteEngineDao : public EngineDaoIntf
{
public:
    SQLiteEngineDao(const QSqlDatabase &db) noexcept;
    SQLiteEngineDao(const SQLiteEngineDao &rhs) = delete;
    SQLiteEngineDao(SQLiteEngineDao &&rhs) noexcept;
    SQLiteEngineDao &operator=(const SQLiteEngineDao &rhs) = delete;
    SQLiteEngineDao &operator=(SQLiteEngineDao &&rhs) noexcept;
    ~SQLiteEngineDao() override;

    bool add(std::int64_t aircraftId, const EngineData &data) noexcept override;
    std::vector<EngineData> getByAircraftId(std::int64_t aircraftId, bool *ok = nullptr) const noexcept override;
    bool deleteByFlightId(std::int64_t flightId) noexcept override;
    bool deleteByAircraftId(std::int64_t aircraftId) noexcept override;

private:
    std::unique_ptr<SQLiteEngineDaoPrivate> d;
};

#endif // SQLITEENGINEDAO_H
