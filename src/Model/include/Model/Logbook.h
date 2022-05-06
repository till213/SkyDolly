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
#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <memory>

#include <QObject>

#include "Flight.h"
#include "ModelLib.h"

class LogbookPrivate;

class MODEL_API Logbook : public QObject
{
    Q_OBJECT
public:
    static Logbook &getInstance() noexcept;
    static void destroyInstance() noexcept;
    Flight &getCurrentFlight() const;
    const Flight &getCurrentFlightConst() const;

signals:
    void flightTitleOrDescriptionChanged(std::int64_t id);

protected:
    ~Logbook();

private:
    Q_DISABLE_COPY(Logbook)
    std::unique_ptr<LogbookPrivate> d;

    Logbook() noexcept;
};

#endif // LOGBOOK_H
