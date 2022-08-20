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
#include <QString>

#include "AircraftType.h"

// PUBLIC

AircraftType::AircraftType() noexcept
    : wingSpan(0),
      engineType(SimType::EngineType::Unknown),
      numberOfEngines(0)
{}

AircraftType::AircraftType(QString type, QString category, int wingSpan, SimType::EngineType engineType, int numberOfEngines) noexcept
    : type(type), category(category), wingSpan(wingSpan), engineType(engineType), numberOfEngines(numberOfEngines)
{};

AircraftType::AircraftType(AircraftType &&other) noexcept
    : type(std::move(other.type)),
      category(std::move(other.category)),
      wingSpan(other.wingSpan),
      engineType(other.engineType),
      numberOfEngines(other.numberOfEngines)
{}

AircraftType &AircraftType::operator=(AircraftType &&rhs) noexcept
{
    if (this != &rhs) {
        type = std::move(rhs.type);
        category = std::move(rhs.category);
        wingSpan = rhs.wingSpan;
        engineType = rhs.engineType;
        numberOfEngines = rhs.numberOfEngines;
    }
    return *this;
}

void AircraftType::clear() noexcept
{
    *this = AircraftType();
}
