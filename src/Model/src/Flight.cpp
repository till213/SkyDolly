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
#include <memory>
#include <vector>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include <QDateTime>
#include <QString>

#include "FlightCondition.h"
#include "Aircraft.h"
#include "Flight.h"

namespace
{
    constexpr int InvalidAircraftIndex = -1;
}

class FlightPrivate
{
public:

    FlightPrivate() noexcept
        : userAircraftIndex(InvalidAircraftIndex)
    {
        clear();
    }

    qint64 id;
    QDateTime creationDate;
    QString title;
    QString description;
    FlightCondition flightCondition;
    std::vector<std::unique_ptr<Aircraft>> aircrafts;
    int userAircraftIndex;

    inline void clear() noexcept {
        id = Flight::InvalidId;
        title.clear();
        description.clear();
    }
};

// PUBLIC

struct Flight::Iterator
{
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Aircraft;
    using pointer           = value_type*;
    using reference         = value_type&;

    Iterator(std::vector<std::unique_ptr<Aircraft>>::iterator it)
        : m_it(it)
    {}

    reference operator*() const { return *(m_it.operator*()); }
    pointer operator->() { return m_it.operator->()->get(); }

private:
    std::vector<std::unique_ptr<Aircraft>>::iterator m_it;
};

Flight::Flight(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightPrivate>())
{
    // The flight may support several aircrafts, but for now there will be always
    // exactly one user aircraft
    std::unique_ptr<Aircraft> userAircraft = std::make_unique<Aircraft>();
    d->aircrafts.push_back(std::move(userAircraft));

    frenchConnection();
}

Flight::~Flight() noexcept
{}

void Flight::setId(qint64 id) noexcept
{
    d->id = id;
}

qint64 Flight::getId() const noexcept
{
    return d->id;
}

const QDateTime &Flight::getCreationDate() const noexcept
{
    return d->creationDate;
}

void Flight::setCreationDate(const QDateTime &creationDate) noexcept
{
    d->creationDate = creationDate;
}

const QString &Flight::getTitle() const noexcept
{
    return d->title;
}

void Flight::setTitle(const QString &title) noexcept
{
    if (d->title != title) {
        d->title = title;
        emit flightChanged();
    }
}

const QString &Flight::getDescription() const noexcept
{
    return d->description;
}

void Flight::setDescription(const QString &description) noexcept
{
    if (d->description != description) {
        d->description = description;
        emit flightChanged();
    }
}

const Aircraft &Flight::getUserAircraftConst() const noexcept
{
    return *(*d->aircrafts.cbegin());
}

Aircraft &Flight::getUserAircraft() const noexcept
{
    return *(*d->aircrafts.cbegin());
}

const FlightCondition &Flight::getFlightConditionConst() const noexcept
{
    return d->flightCondition;
}

void Flight::setFlightCondition(FlightCondition flightCondition) noexcept
{
    d->flightCondition = flightCondition;
    emit flightConditionChanged();
}

qint64 Flight::getTotalDurationMSec() const noexcept
{
    // For now the total duration is the duration of the
    // (one and only) user aircraft
    return d->aircrafts.at(0)->getDurationMSec();
}

void Flight::clear() noexcept
{
    d->clear();
    getUserAircraft().clear();
    d->flightCondition.clear();
}

// PRIVATE

void Flight::frenchConnection() noexcept
{
    Aircraft &userAircraft = getUserAircraft();
    connect(&userAircraft, &Aircraft::infoChanged,
            this, &Flight::aircraftInfoChanged);
    connect(&userAircraft, &Aircraft::dataChanged,
            this, &Flight::positionDataChanged);
}
