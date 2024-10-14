/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef SIMCONNECTVARIABLES_H
#define SIMCONNECTVARIABLES_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

/*!
 * Individual simulation variables for updating states, particularly for "toggle events".
 *
 * Implementation note: all structs needs to be packed.
 */
namespace SimConnectVariables
{
    /*! Flaps handle index.*/
    #pragma pack(push, 1)
    struct FlapsHandleIndex
    {
        std::int32_t value {0};
    };
    #pragma pack(pop)

    /*!  Navigation light. */
    #pragma pack(push, 1)
    struct NavigationLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Beacon light. */
    #pragma pack(push, 1)
    struct BeaconLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Landing light. */
    #pragma pack(push, 1)
    struct LandingLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Taxi light. */
    #pragma pack(push, 1)
    struct TaxiLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Strobe light. */
    #pragma pack(push, 1)
    struct StrobeLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Panel light.*/
    #pragma pack(push, 1)
    struct PanelLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Recognition light.*/
    #pragma pack(push, 1)
    struct RecognitionLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Wing light. */
    #pragma pack(push, 1)
    struct WingLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Logo light. */
    #pragma pack(push, 1)
    struct LogoLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Cabin light. */
    #pragma pack(push, 1)
    struct CabinLight
    {
        std::int32_t value;
    };
    #pragma pack(pop)

    /*! Simulation rate. */
    #pragma pack(push, 1)
    struct SimulationRate
    {
        float value;
    };
    #pragma pack(pop)

    void addToDataDefinition(HANDLE simConnectHandle) noexcept;
}

#endif // SIMCONNECTVARIABLES_H
