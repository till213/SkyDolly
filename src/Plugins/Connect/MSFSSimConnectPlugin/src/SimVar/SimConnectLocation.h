/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef SIMCONNECTLOCATION_H
#define SIMCONNECTLOCATION_H

#include <windows.h>

#include <Model/Location.h>
#include <Model/LightData.h>

/*!
 * The location response structure. It essentially contains the same information like the
 * predefined data structure SIMCONNECT_DATA_INITPOSITION, however the later cannot be
 * used as part of data requests.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectLocation
{
    double latitude {0.0};
    double longitude {0.0};
    double altitude {0.0};
    double pitch {0.0};
    double bank {0.0};
    double trueHeading {0.0};
    int indicatedAirspeed {0};
    int onGround {false};

    inline Location toLocation() const noexcept
    {
        Location location;
        location.latitude = latitude;
        location.longitude = longitude;
        location.altitude = altitude;
        location.pitch = pitch;
        location.bank = bank;
        location.trueHeading = trueHeading;
        location.indicatedAirspeed = indicatedAirspeed;
        location.onGround = (onGround != 0);

        return location;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTLOCATION_H
