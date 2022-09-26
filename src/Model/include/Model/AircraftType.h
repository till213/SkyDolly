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
#ifndef AIRCRAFTTYPE_H
#define AIRCRAFTTYPE_H

#include <QString>
#include <QDateTime>

#include "SimType.h"
#include "ModelLib.h"

struct MODEL_API AircraftType
{
    /*!
     * The aircraft type, e.g. "Pitts Special". This is really the SimConnect "container title"
     * which is also used to spawn AI aircraft.
     */
    QString type;

    /*!
     * The aircraft category:
     * - Piston
     * - Jet
     * - Rocket
     * - Glider
     * - ...
     */
    QString category;
    // Feet
    int wingSpan;
    SimType::EngineType engineType;
    int numberOfEngines;

    AircraftType() noexcept;
    AircraftType(QString type, QString category, int wingSpan, SimType::EngineType engineType, int numberOfEngines) noexcept;
    ~AircraftType() = default;
    AircraftType(const AircraftType &rhs) = default;
    AircraftType(AircraftType &&rhs) noexcept;
    AircraftType &operator=(const AircraftType &rhs) = default;
    AircraftType &operator=(AircraftType &&rhs) noexcept;

    void clear() noexcept;
};

inline bool operator==(const AircraftType &lhs, const AircraftType &rhs) {
    return lhs.type == rhs.type &&
           lhs.category == rhs.category &&
           lhs.wingSpan == rhs.wingSpan &&
           lhs.engineType == rhs.engineType &&
           lhs.numberOfEngines == rhs.numberOfEngines;
}

inline bool operator!=(const AircraftType&lhs, const AircraftType &rhs) {
    return !(lhs == rhs);
}

#endif // AIRCRAFTTYPE_H
