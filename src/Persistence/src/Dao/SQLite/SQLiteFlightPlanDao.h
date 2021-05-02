/**
 * Sky Dolly - The black sheep for your fFlightPlan recordings
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
#ifndef SQLITEFFLIGHTPLANDAO_H
#define SQLITEFFLIGHTPLANDAO_H

#include <memory>

#include <QObject>
#include <QVector>

#include "../../../../Model/src/FlightPlanData.h"
#include "../FlightPlanDaoIntf.h"

class SQLiteFlightPlanDaoPrivate;

class SQLiteFlightPlanDao : public QObject, public FlightPlanDaoIntf
{
public:
    explicit SQLiteFlightPlanDao(QObject *parent = nullptr) noexcept;
    virtual ~SQLiteFlightPlanDao() noexcept;

    virtual bool add(qint64 aircraftId, const QVector<FlightPlanData> &data) noexcept override;
    virtual bool getByAircraftId(qint64 aircraftId, QVector<FlightPlanData> &data) const noexcept override;
    virtual bool deleteByFlightId(qint64 flightId) noexcept override;

private:
    std::unique_ptr<SQLiteFlightPlanDaoPrivate> d;

    void frenchConnection() noexcept;

private slots:
    void handleConnectionChanged() noexcept;
};

#endif // SQLITEFFLIGHTPLANDAO_H
