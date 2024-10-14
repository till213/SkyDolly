/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef LOCATION_H
#define LOCATION_H

#include <cstdint>

#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include "InitialPosition.h"
#include "Data.h"
#include "ModelLib.h"

struct MODEL_API Location final : public Data
{
    QString title;
    QString description;
    QDate localSimulationDate;
    QTime localSimulationTime;
    std::int64_t typeId {Const::InvalidId};
    std::int64_t categoryId {Const::InvalidId};
    std::int64_t countryId {Const::InvalidId};
    std::int64_t engineEventId {Const::InvalidId};
    QString identifier;
    double latitude {0.0};
    double longitude {0.0};
    double altitude {0.0};
    double pitch {0.0};
    double bank {0.0};
    double trueHeading {0.0};
    int indicatedAirspeed {0};
    bool onGround {false};

    explicit Location(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0) noexcept;
    explicit Location(const InitialPosition &initialPosition) noexcept;

    inline InitialPosition toInitialPosition() const noexcept {
        InitialPosition initialPosition {latitude, longitude, altitude};
        initialPosition.pitch = pitch;
        initialPosition.bank = bank;
        initialPosition.trueHeading = trueHeading;
        initialPosition.indicatedAirspeed = indicatedAirspeed;
        initialPosition.onGround = onGround;

        return initialPosition;
    }
};

#endif // LOCATION_H

