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
#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H


#include <memory>

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "ModelLib.h"

class FlightPlanData;
class FlightPlanPrivate;

class MODEL_API FlightPlan : public QObject
{
    Q_OBJECT
public:
    FlightPlan(QObject *parent = nullptr) noexcept;
    virtual ~FlightPlan() noexcept;

    void add(const FlightPlanData &flightPlanData) noexcept;
    QVector<FlightPlanData> &getAll() const noexcept;
    const QVector<FlightPlanData> &getAllConst() const noexcept;

    void clear() noexcept;

signals:
    void dataChanged();

private:
    Q_DISABLE_COPY(FlightPlan)
    std::unique_ptr<FlightPlanPrivate> d;
};

#endif // FLIGHTPLAN_H
