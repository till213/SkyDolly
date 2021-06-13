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
#ifndef FLIGHT_H
#define FLIGHT_H

#include <memory>

#include <QObject>

class QDateTime;
class QString;

#include "ModelLib.h"

class FlightCondition;
class Aircraft;
class FlightPrivate;

class MODEL_API Flight : public QObject
{
    Q_OBJECT
public:
    Flight(QObject *parent = nullptr) noexcept;
    virtual ~Flight() noexcept;

    qint64 getId() const noexcept;
    void setId(qint64 id) noexcept;

    const QDateTime &getCreationDate() const noexcept;
    void setCreationDate(const QDateTime &creationDate) noexcept;

    const QString &getTitle() const noexcept;
    void setTitle(const QString &title) noexcept;

    const QString &getDescription() const noexcept;
    void setDescription(const QString &description) noexcept;

    Aircraft &addUserAircraft() noexcept;
    const Aircraft &getUserAircraftConst() const noexcept;
    Aircraft &getUserAircraft() const noexcept;
    int getUserAircraftIndex() const noexcept;
    void setUserAircraftIndex(int index) noexcept;
    qint64 deleteAircraftByIndex(int index) noexcept;

    std::vector<std::unique_ptr<Aircraft>> &getAircrafts() const noexcept;
    int getAircraftCount() const noexcept;

    const FlightCondition &getFlightConditionConst() const noexcept;
    void setFlightCondition(FlightCondition flightCondition) noexcept;

    qint64 getTotalDurationMSec(bool ofUserAircraft = false) const noexcept;

    void clear() noexcept;

    template <typename T>
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = value_type*;
        using reference         = value_type&;

        Iterator(typename std::vector<value_type>::iterator it) noexcept
            : m_it(it)
        {}

        reference operator*() const noexcept { return m_it.operator*(); }
        pointer operator->() noexcept { return m_it.operator->(); }

        // Prefix increment
        Iterator& operator++() noexcept { m_it++; return *this; }

        // Postfix increment
        Iterator operator++(int) noexcept { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) noexcept { return a.m_it == b.m_it; };
        friend bool operator!= (const Iterator& a, const Iterator& b) noexcept { return a.m_it != b.m_it; };

    private:
        typename std::vector<value_type>::iterator m_it;
    };

    typedef Iterator<std::unique_ptr<Aircraft>> it;

    it begin() noexcept;
    it end() noexcept;
    const it begin() const noexcept;
    const it end() const noexcept;

    static constexpr int InvalidId = -1;

signals:
    void flightChanged();
    void flightConditionChanged();
    void aircraftAdded(Aircraft &aircraft);
    void aircraftRemoved();
    void userAircraftChanged(Aircraft &aircraft);

private:
    Q_DISABLE_COPY(Flight)
    std::unique_ptr<FlightPrivate> d;
};

#endif // FLIGHT_H
