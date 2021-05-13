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
#include "../../../Model/src/FlightDate.h"
#include "../../../Model/src/FlightSummary.h"
#include "../PersistenceLib.h"

class SkyConnectIntf;
class FlightServicePrivate;

class PERSISTENCE_API FlightService : public QObject
{
    Q_OBJECT
public:
    FlightService(QObject *parent = nullptr) noexcept;
    virtual ~FlightService() noexcept;

    bool store(Flight &flight) noexcept;
    bool restore(qint64 id, Flight &flight) noexcept;
    bool deleteById(qint64 id) noexcept;
    bool updateTitle(qint64 id, const QString &title) noexcept;
    bool updateTitleAndDescription(qint64 id, const QString &title, const QString &description) noexcept;
    bool updateUserAircraftIndex(Flight &flight, int index) noexcept;

    QVector<FlightDate> getFlightDates() const noexcept;
    QVector<FlightSummary> getFlightSummaries() const noexcept;

signals:
    void flightStored(qint64 id);
    void flightRestored(qint64 id);
    void flightUpdated(qint64 id);

private:
    std::unique_ptr<FlightServicePrivate> d;
};

#endif // FLIGHTSERVICE_H
