/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef ABSTRACTKMLTRACKPARSER_H
#define ABSTRACTKMLTRACKPARSER_H

#include <memory>

#include <QDateTime>

class QXmlStreamReader;

#include "AbstractKmlParser.h"

struct FlightData;
struct AbstractKmlTrackParserPrivate;

class AbstractKmlTrackParser : public AbstractKmlParser
{
public:
    AbstractKmlTrackParser() noexcept;
    AbstractKmlTrackParser(const AbstractKmlTrackParser &rhs) = delete;
    AbstractKmlTrackParser(AbstractKmlTrackParser &&rhs) = delete;
    AbstractKmlTrackParser &operator=(const AbstractKmlTrackParser &rhs) = delete;
    AbstractKmlTrackParser &operator=(AbstractKmlTrackParser &&rhs) = delete;
    ~AbstractKmlTrackParser() override;

protected:
    void parseTrack(FlightData &flightData) noexcept override;
    QDateTime getFirstDateTimeUtc() const;

private:
    const std::unique_ptr<AbstractKmlTrackParserPrivate> d;
};

#endif // ABSTRACTKMLTRACKPARSER_H
