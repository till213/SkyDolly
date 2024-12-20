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
#ifndef SIMCONNECTEVENT_H
#define SIMCONNECTEVENT_H

#include <windows.h>
#include <SimConnect.h>

namespace SimConnectEvent
{
    enum struct Event: ::SIMCONNECT_CLIENT_EVENT_ID {
        // System events (subscription)
        SimStart,
        Pause,
        Crashed,
        Frame,
        // System events (requests)
        PauseSet,
        SimRateIncr,
        SimRateDecr,
        // Simulation time (requests)
        ZuluYearSet,
        ZuluDaySet,
        ZuluHoursSet,
        ZuluMinutesSet,
        // Aircraft freeze
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude,
        // Engine
        EngineAutoStart,
        EngineAutoShutdown,
        // Primary flight controls
        AxisAileronsSet,
        AxisElevatorSet,
        AxisRudderSet,
        // Secondary flight controls
        FlapsDecrease,
        FlapsIncrease,
        SpoilersSet,
        AxisSpoilerSet,
        SpoilersOff,
        SpoilersOn,
        SpoilersArmSet,
        // Aircraft handles
        GearDown,
        GearUp,
        SetTailHookHandle,
        SetWingFold,
        AxisSteeringSet,
        // Light
        ToggleNavLights,
        ToggleBeaconLights,
        LandingLightsToggle,
        ToggleTaxiLights,
        StrobesToggle,
        PanelLightsToggle,
        ToggleRecognitionLights,
        ToggleWingLights,
        ToggleLogoLights,
        ToggleCabinLights,
        // Custom input events
        CustomRecord,
        CustomReplay,
        CustomPause,
        CustomStop,
        CustomForwardDown,
        CustomForwardUp,
        CustomBackwardDown,
        CustomBackwardUp,
        CustomBegin,
        CustomEnd
    };
}

#endif // SIMCONNECTEVENT_H
