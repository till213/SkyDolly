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
#ifndef FLIGHTSERVICE_H
#define FLIGHTSERVICE_H

#include <memory>

#include <QObject>
#include <QVector>

#include "../../../Model/src/Flight.h"
#include "../../../Model/src/FlightSummary.h"
#include "../PersistenceLib.h"

class FlightServicePrivate;

class PERSISTENCE_API FlightService : public QObject
{
    Q_OBJECT
public:
    FlightService() noexcept;
    ~FlightService() noexcept;

    bool store(Flight &flight) noexcept;
    bool restore(qint64 id, Flight &flight) noexcept;
    bool deleteById(qint64 id) noexcept;
    bool updateDescription(qint64 id, const QString &description) noexcept;

    QVector<FlightSummary> getFlightDescriptions() const noexcept;

signals:
    void flightStored(qint64 flightId);
    void flightRestored(qint64 flightId);

private:
    std::unique_ptr<FlightServicePrivate> d;
};

#endif // FLIGHTSERVICE_H
