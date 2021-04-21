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
#ifndef SCENARIO_H
#define SCENARIO_H

#include <memory>

#include <QObject>

#include "ModelLib.h"

class FlightCondition;
class Aircraft;
class ScenarioPrivate;

class MODEL_API Scenario : public QObject
{
    Q_OBJECT
public:
    Scenario(QObject *parent = nullptr) noexcept;
    ~Scenario() noexcept;

    void setId(qint64 id) noexcept;
    qint64 getId() const noexcept;

    void setDescription(const QString &description) noexcept;
    const QString &getDescription() const noexcept;

    const Aircraft &getUserAircraftConst() const noexcept;
    Aircraft &getUserAircraft() const noexcept;

    void setFlightCondition(FlightCondition flightCondition) noexcept;
    const FlightCondition &getFlightConditionConst() const noexcept;

    qint64 getTotalDurationMSec() const noexcept;

    void clear() noexcept;

signals:
    void aircraftInfoChanged();
    void aircraftDataChanged();
    void flightConditionChanged();

private:
    Q_DISABLE_COPY(Scenario)
    std::unique_ptr<ScenarioPrivate> d;

    void frenchConnection() noexcept;
};

#endif // SCENARIO_H
