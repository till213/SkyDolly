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
#ifndef FLIGHTSERVICE_H
#define FLIGHTSERVICE_H

#include <memory>
#include <forward_list>
#include <cstdint>

#include <Model/Flight.h>
#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include "../PersistenceLib.h"

class SkyConnectIntf;
class FlightServicePrivate;

class PERSISTENCE_API FlightService
{
public:
    FlightService() noexcept;
    virtual ~FlightService() noexcept;

    bool store(Flight &flight) noexcept;
    bool restore(std::int64_t id, Flight &flight) noexcept;
    bool deleteById(std::int64_t id) noexcept;
    bool updateTitle(Flight &flight, const QString &title) noexcept;
    bool updateTitle(std::int64_t id, const QString &title) noexcept;
    bool updateTitleAndDescription(Flight &flight, const QString &title, const QString &description) noexcept;
    bool updateTitleAndDescription(std::int64_t id, const QString &title, const QString &description) noexcept;
    bool updateUserAircraftIndex(Flight &flight, int index) noexcept;

private:
    std::unique_ptr<FlightServicePrivate> d;
};

#endif // FLIGHTSERVICE_H
