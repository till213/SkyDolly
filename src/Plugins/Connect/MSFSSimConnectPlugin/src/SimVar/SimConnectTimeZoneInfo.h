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
#ifndef SIMCONNECTTIMEZONEINFO_H
#define SIMCONNECTTIMEZONEINFO_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QTimeZone>

#include <Model/TimeZoneInfo.h>

/*!
 * Simulation date and time (local and zulu).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectTimeZoneInfo
{
    std::int32_t timeZoneOffset {0};
    std::int32_t zuluSunriseTime {0};
    std::int32_t zuluSunsetTime {0};

    inline TimeZoneInfo toTimeZoneInfo() const noexcept
    {
        TimeZoneInfo info {};
        info.timeZoneOffsetSeconds = timeZoneOffset;
        info.zuluSunriseTimeSeconds = zuluSunriseTime;
        info.zuluSunsetTimeSeconds = zuluSunsetTime;
        return info;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTTIMEZONEINFO_H
