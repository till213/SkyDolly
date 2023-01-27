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
#ifndef LIGHTDAOINTF_H
#define LIGHTDAOINTF_H

#include <vector>
#include <cstdint>

class QSqlDatabase;

struct LightData;

class LightDaoIntf
{
public:
    LightDaoIntf() = default;
    LightDaoIntf(const LightDaoIntf &rhs) = delete;
    LightDaoIntf(LightDaoIntf &&rhs) = default;
    LightDaoIntf &operator=(const LightDaoIntf &rhs) = delete;
    LightDaoIntf &operator=(LightDaoIntf &&rhs) = default;
    virtual ~LightDaoIntf() = default;

    /*!
     * Persists the \c lightData.
     *
     * \param aircraftId
     *        the aircraft the \c lightData belongs to
     * \param lightData
     *        the LightData to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(QSqlDatabase &db, std::int64_t aircraftId, const LightData &lightData) noexcept = 0;
    virtual std::vector<LightData> getByAircraftId(QSqlDatabase &db, std::int64_t aircraftId, bool *ok = nullptr) const noexcept = 0;
    virtual bool deleteByFlightId(QSqlDatabase &db, std::int64_t flightId) noexcept = 0;
    virtual bool deleteByAircraftId(QSqlDatabase &db, std::int64_t aircraftId) noexcept = 0;
};

#endif // LIGHTDAOINTF_H
