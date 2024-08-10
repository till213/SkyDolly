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
#include <memory>
#include <vector>
#include <mutex>

#include <QObject>

#include "Logbook.h"

struct LogbookPrivate
{
    std::vector<std::unique_ptr<Flight>> flights;

    static inline std::once_flag onceFlag;
    static inline std::unique_ptr<Logbook> instance;
};

// PUBLIC

Logbook &Logbook::getInstance() noexcept
{
    std::call_once(LogbookPrivate::onceFlag, []() {
        LogbookPrivate::instance = std::unique_ptr<Logbook>(new Logbook());
    });
    return *LogbookPrivate::instance;
}

void Logbook::destroyInstance() noexcept
{
    if (LogbookPrivate::instance != nullptr) {
        LogbookPrivate::instance.reset();
    }
}

Flight &Logbook::getCurrentFlight() const noexcept
{
    return *(*d->flights.cbegin());
}

// PRIVATE

Logbook::Logbook() noexcept
    : QObject(),
      d {std::make_unique<LogbookPrivate>()}
{
    // Logbook may support several flights, but for now there will be always
    // exactly one
    d->flights.push_back(std::make_unique<Flight>());
}

Logbook::~Logbook() = default;
