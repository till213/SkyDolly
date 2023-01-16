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
#ifndef SIMCONNECTEVENT_H
#define SIMCONNECTEVENT_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/SimType.h>
#include "Engine/SimConnectEngineEvent.h"
#include "Engine/SimConnectEngineAll.h"
#include "PrimaryFlightControl/SimConnectPrimaryFlightControlEvent.h"
#include "SecondaryFlightControl/SimConnectSecondaryFlightControlEvent.h"
#include "AircraftHandle/SimConnectAircraftHandleAll.h"
#include "Light/SimConnectLightEvent.h"
#include "SimConnectType.h"

class SimConnectEvent
{
public:
    static constexpr std::int32_t InvalidFlapsIndex {-1};

    enum struct Event: ::SIMCONNECT_CLIENT_EVENT_ID {
        // System events (subscription)
        SimStart,
        Pause,
        Crashed,
        Frame,
        // System events (requests)
        PauseSet,
        // Aircraft
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude,
        // Engine
        AxisThrottle1Set,
        AxisThrottle2Set,
        AxisThrottle3Set,
        AxisThrottle4Set,
        AxisPropeller1Set,
        AxisPropeller2Set,
        AxisPropeller3Set,
        AxisPropeller4Set,
        CowlFlap1Set,
        CowlFlap2Set,
        CowlFlap3Set,
        CowlFlap4Set,
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
        SmokeSet,
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
        ToggleCabinLights
    };

    enum struct EngineState: int {
        Unknown,
        Starting,
        Started,
        Stopped
    };

    template <typename T>
    struct SwitchState
    {
        T current {};
        T requested {};
        bool valid {false};
        bool pending {false};

        SwitchState()
        {
            reset();
        }

        /*!
         * Returns whether an update needs to be sent, as the \c requested value differs from the \c current
         * value.
         *
         * \return \c true if the \c current value is different from the \c requested value; \c false else
         */
        bool needsUpdate() const noexcept
        {
            return current != requested;
        }

        void reset() noexcept
        {
            if constexpr(std::is_integral_v<T>) {
                current = 0;
                requested = 0;
            }
            else if constexpr(std::is_floating_point_v<T>) {
                current = 0.0;
                requested = 0.0;
            } else if constexpr(std::is_same_v<T, bool>) {
                current = false;
                requested = false;
            } else {
                current = {};
                requested = {};
            }
            valid = false;
            pending = false;
        }
    };

    struct ToggleSwitch : public SwitchState<bool>
    {
        Event toggleEvent;

        ToggleSwitch(Event toggleEvent)
            : toggleEvent(toggleEvent)
        {
            SwitchState<bool>::reset();
        }
    };

    SimConnectEvent(::HANDLE simConnectHandle)
        : m_simConnectHandle(simConnectHandle)
    {}

    static constexpr std::int16_t Max16KPosition {16383};

    /*!
     * Converts the normalised \c value to an \em event (position) value.
     *
     * @param value
     *        the normalised value to be converted [-1.0, 1.0]
     * @return the converted \em event (position) value [-16384, 16384]
     */
    constexpr std::int16_t positionTo16K(double value) noexcept
    {
        return static_cast<std::int16_t>(std::round(value * Max16KPosition));
    }

    /*!
     * Converts the \c percent to an \em event (position) value.
     *
     * @param percent
     *        the percent value to be converted [0, 100]
     * @return the converted \em event (position) value [-16384, 16384]
     */
    constexpr std::int16_t percentTo16K(double percent) noexcept
    {
        return static_cast<std::int16_t>(std::round(percent * (double)Max16KPosition / 100.0));
    }

    void setupEvents()
    {
        // System event subscription
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::SimStart), "SimStart");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::Pause), "Pause");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::Crashed), "Crashed");

        // Client events
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::PauseSet), "PAUSE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeLatituteLongitude), "FREEZE_LATITUDE_LONGITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeAltitude), "FREEZE_ALTITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeAttitude), "FREEZE_ATTITUDE_SET");
        // Engine
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::EngineAutoStart), "ENGINE_AUTO_START");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::EngineAutoShutdown), "ENGINE_AUTO_SHUTDOWN");
        // Primary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::AxisAileronsSet), "AXIS_AILERONS_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::AxisElevatorSet), "AXIS_ELEVATOR_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::AxisRudderSet), "AXIS_RUDDER_SET");
        // Secondary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FlapsDecrease), "FLAPS_DECR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FlapsIncrease), "FLAPS_INCR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SpoilersSet), "SPOILERS_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::AxisSpoilerSet), "AXIS_SPOILER_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SpoilersOff), "SPOILERS_OFF");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SpoilersOn), "SPOILERS_ON");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SpoilersArmSet), "SPOILERS_ARM_SET");
        // Handles
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::GearUp), "GEAR_UP");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::GearDown), "GEAR_DOWN");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SetTailHookHandle), "SET_TAIL_HOOK_HANDLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SetWingFold), "SET_WING_FOLD");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::SmokeSet), "SMOKE_SET");
        // Lights
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleNavLights), "TOGGLE_NAV_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleBeaconLights), "TOGGLE_BEACON_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::LandingLightsToggle), "LANDING_LIGHTS_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleTaxiLights), "TOGGLE_TAXI_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::StrobesToggle), "STROBES_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::PanelLightsToggle), "PANEL_LIGHTS_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleRecognitionLights), "TOGGLE_RECOGNITION_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleWingLights), "TOGGLE_WING_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleLogoLights), "TOGGLE_LOGO_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ToggleCabinLights), "TOGGLE_CABIN_LIGHTS");
    }

    inline void pauseSimulation(bool enable) noexcept
    {
        const DWORD data = enable ? 1 : 0;
        m_paused = enable;
        ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::PauseSet), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    }

    /*!
     * Resumes ("unpauses") the simulation if it had prevsiouly been paused by the application.
     */
    inline void resumePausedSimulation() noexcept
    {
        if (m_paused) {
            ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::PauseSet), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            m_paused = false;
        }
    }

    inline bool freezeAircraft(::SIMCONNECT_OBJECT_ID objectId, bool enable) const noexcept
    {
        const DWORD data = enable ? 1 : 0;
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, objectId, Enum::underly(SimConnectEvent::Event::FreezeLatituteLongitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, objectId, Enum::underly(SimConnectEvent::Event::FreezeAltitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, objectId, Enum::underly(SimConnectEvent::Event::FreezeAttitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

        return result == S_OK;
    }

    inline bool sendEngine(const SimConnectEngineAll &engine) noexcept
    {
        const bool ok = sendEngineState(engine);
        return ok;
    }

    inline bool sendPrimaryFlightControl(const SimConnectPrimaryFlightControlEvent &event)
    {
        // The recorded control surface values have opposite sign than the event values to be sent
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AxisRudderSet), -positionTo16K(event.rudderPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AxisAileronsSet), -positionTo16K(event.aileronPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AxisElevatorSet), -positionTo16K(event.elevatorPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

        return result == S_OK;
    }

    inline bool sendSecondaryFlightControl(const SimConnectSecondaryFlightControlEvent &event)
    {
        bool ok = sendSpoilersArmed(event.spoilersArmed);
        if (ok && event.spoilersArmed == 0) {
            ok = sendSpoilerPosition(percentTo16K(event.spoilersHandlePosition));
        }
        if (ok) {
            m_flapsIndex.requested = event.flapsHandleIndex;
            ok = sendFlapsHandleIndex();
        }
        return ok;
    }

    inline bool sendAircraftHandle(const SimConnectAircraftHandleAll &aircraftHandle)
    {
        const SimConnectAircraftHandleEvent &event = aircraftHandle.event;
        bool ok = sendGearHandlePosition(event.gearHandlePosition);
        if (ok) {
            sendTailhookHandlePosition(aircraftHandle.info.tailhookHandle);
        }
        if (ok) {
            sendSmokeEnabled(aircraftHandle.event.smokeEnable);
        }
        if (ok) {
            sendWingFold(aircraftHandle.info.foldingWingHandlePosition);
        }
        return ok;
    }

    inline bool sendLight(const SimConnectLightEvent &event)
    {
        m_navigationLightSwitch.requested = event.navigation;
        bool ok = sendLightState(m_navigationLightSwitch);
        m_beaconLightSwitch.requested = event.beacon;
        ok = ok && sendLightState(m_beaconLightSwitch);
        m_landingLightSwitch.requested = event.landing;
        ok = ok && sendLightState(m_landingLightSwitch);
        m_taxiLightSwitch.requested = event.taxi;
        ok = ok && sendLightState(m_taxiLightSwitch);
        m_strobeLightSwitch.requested = event.taxi;
        ok = ok && sendLightState(m_strobeLightSwitch);
        m_panelLightSwitch.requested = event.panel;
        ok = ok && sendLightState(m_panelLightSwitch);
        m_recognitionLightSwitch.requested = event.recognition;
        ok = ok && sendLightState(m_recognitionLightSwitch);
        m_wingLightSwitch.requested = event.wing;
        ok = ok && sendLightState(m_wingLightSwitch);
        m_logoLightSwitch.requested = event.logo;
        ok = ok && sendLightState(m_logoLightSwitch);
        m_cabinLightSwitch.requested = event.cabin;
        ok = ok && sendLightState(m_cabinLightSwitch);

        return ok;
    }

    inline bool setCurrentFlapsHandleIndex(std::int32_t index)
    {
        m_flapsIndex.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentFlapsHandleIndex: current index received:" << index
                 << "Previous index:" << m_flapsIndex.current;
#endif
        m_flapsIndex.current = index;
        m_flapsIndex.valid = true;
        const bool ok = sendFlapsHandleIndex();
        return ok;
    }

    inline bool setCurrentNavigationLight(std::int32_t enabled)
    {
        m_navigationLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentNavigationLight: current navigation light received:" << enabled
                 << "Previous state:" << m_navigationLightSwitch.current;
#endif
        m_navigationLightSwitch.current = (enabled != 0);
        m_navigationLightSwitch.valid = true;
        const bool ok = sendLightState(m_navigationLightSwitch);
        return ok;
    }

    inline bool setCurrentBeaconLight(std::int32_t enabled)
    {
        m_navigationLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentBeaconLight: current beacon light received:" << enabled
                 << "Previous state:" << m_navigationLightSwitch.current;
#endif
        m_beaconLightSwitch.current = (enabled != 0);
        m_beaconLightSwitch.valid = true;
        const bool ok = sendLightState(m_beaconLightSwitch);
        return ok;
    }

    inline bool setCurrentLandingLight(std::int32_t enabled)
    {
        m_landingLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentLandingLight: current landing light received:" << enabled
                 << "Previous state:" << m_landingLightSwitch.current;
#endif
        m_landingLightSwitch.current = (enabled != 0);
        m_landingLightSwitch.valid = true;
        const bool ok = sendLightState(m_landingLightSwitch);
        return ok;
    }

    inline bool setCurrentTaxiLight(std::int32_t enabled)
    {
        m_taxiLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentTaxiLight: current taxi light received:" << enabled
                 << "Previous state:" << m_taxiLightSwitch.current;
#endif
        m_taxiLightSwitch.current = (enabled != 0);
        m_taxiLightSwitch.valid = true;
        const bool ok = sendLightState(m_taxiLightSwitch);
        return ok;
    }

    inline bool setCurrentStrobeLight(std::int32_t enabled)
    {
        m_strobeLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentStrobeLight: current strobe light received:" << enabled
                 << "Previous state:" << m_strobeLightSwitch.current;
#endif
        m_strobeLightSwitch.current = (enabled != 0);
        m_strobeLightSwitch.valid = true;
        const bool ok = sendLightState(m_strobeLightSwitch);
        return ok;
    }

    inline bool setCurrentPanelLight(std::int32_t enabled)
    {
        m_panelLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentPanelLight: current panel light received:" << enabled
                 << "Previous state:" << m_panelLightSwitch.current;
#endif
        m_panelLightSwitch.current = (enabled != 0);
        m_panelLightSwitch.valid = true;
        const bool ok = sendLightState(m_panelLightSwitch);
        return ok;
    }

    inline bool setCurrentRecognitionLight(std::int32_t enabled)
    {
        m_recognitionLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentRecognitionLight: current recognition light received:" << enabled
                 << "Previous state:" << m_recognitionLightSwitch.current;
#endif
        m_recognitionLightSwitch.current = (enabled != 0);
        m_recognitionLightSwitch.valid = true;
        const bool ok = sendLightState(m_recognitionLightSwitch);
        return ok;
    }

    inline bool setCurrentWingLight(std::int32_t enabled)
    {
        m_wingLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentWingLight: current wing light received:" << enabled
                 << "Previous state:" << m_wingLightSwitch.current;
#endif
        m_wingLightSwitch.current = (enabled != 0);
        m_wingLightSwitch.valid = true;
        const bool ok = sendLightState(m_wingLightSwitch);
        return ok;
    }

    inline bool setCurrentLogoLight(std::int32_t enabled)
    {
        m_logoLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentLogoLight: current logo light received:" << enabled
                 << "Previous state:" << m_logoLightSwitch.current;
#endif
        m_logoLightSwitch.current = (enabled != 0);
        m_logoLightSwitch.valid = true;
        const bool ok = sendLightState(m_logoLightSwitch);
        return ok;
    }

    inline bool setCurrentCabinLight(std::int32_t enabled)
    {
        m_cabinLightSwitch.pending = false;
#ifdef DEBUG
        qDebug() << "SimConnectEvent::setCurrentCabinLight: current cabin light received:" << enabled
                 << "Previous state:" << m_cabinLightSwitch.current;
#endif
        m_cabinLightSwitch.current = (enabled != 0);
        m_cabinLightSwitch.valid = true;
        const bool ok = sendLightState(m_cabinLightSwitch);
        return ok;
    }

    inline void reset() {
        // Engine
        m_engineState = EngineState::Unknown;
        // Flaps
        m_flapsIndex.reset();
        // Lights
        m_navigationLightSwitch.reset();
        m_beaconLightSwitch.reset();
        m_landingLightSwitch.reset();
        m_taxiLightSwitch.reset();
        m_strobeLightSwitch.reset();
        m_panelLightSwitch.reset();
        m_recognitionLightSwitch.reset();
        m_wingLightSwitch.reset();
        m_logoLightSwitch.reset();
        m_cabinLightSwitch.reset();
    }

private:
    ::HANDLE m_simConnectHandle;
    EngineState m_engineState {EngineState::Unknown};

    SwitchState<std::int32_t> m_flapsIndex;

    ToggleSwitch m_navigationLightSwitch {Event::ToggleNavLights};
    ToggleSwitch m_beaconLightSwitch {Event::ToggleBeaconLights};
    ToggleSwitch m_landingLightSwitch {Event::LandingLightsToggle};
    ToggleSwitch m_taxiLightSwitch {Event::ToggleTaxiLights};
    ToggleSwitch m_strobeLightSwitch {Event::StrobesToggle};
    ToggleSwitch m_panelLightSwitch {Event::PanelLightsToggle};
    ToggleSwitch m_recognitionLightSwitch {Event::ToggleRecognitionLights};
    ToggleSwitch m_wingLightSwitch {Event::ToggleWingLights};
    ToggleSwitch m_logoLightSwitch {Event::ToggleLogoLights};
    ToggleSwitch m_cabinLightSwitch {Event::ToggleCabinLights};

    bool m_paused {false};

    inline bool sendEngineState(const SimConnectEngineAll &engine) noexcept
    {
        HRESULT result {S_OK};
        const SimConnectEngineCore &engineCore = engine.core;
        const SimConnectEngineEvent &engineEvent = engine.event;
        switch (m_engineState) {
        case EngineState::Starting:
            if (engineEvent.hasCombustion()) {
                m_engineState = EngineState::Started;
            } else if (!engineCore.hasEngineStarterEnabled()) {
                 // STARTING: Engine started disabled, no combustion -> STOPPED
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EngineState::Stopped;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendEngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: STARTING"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        case EngineState::Started:
            if (!engineEvent.hasCombustion()) {
                // STARTED: No combustion -> STOPPED
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EngineState::Stopped;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendEngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: STARTED"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        case EngineState::Stopped:
            // Either general engine starter has been enabled or combustion has started -> engine start
            // Note: apparently the engine starter can be disabled (false) and yet with an active combustion (= running engine)
            //       specifically in the case when the aircraft has been "auto-started" (CTRL + E)
            if (engineCore.hasEngineStarterEnabled() || engineEvent.hasCombustion()) {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EngineState::Starting;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendEngineState: starting engines"
                         << "Current engine state: STARTING"
                         << "Previous engine state: STOPPED"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        default:
            // Unknown
            if (engineCore.hasEngineStarterEnabled() || engineEvent.hasCombustion()) {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = engineEvent.hasCombustion() ? EngineState::Started : EngineState::Starting;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendEngineState: starting engines"
                         << "Current engine state:" << (engineEvent.hasCombustion() ? "STARTED" : "STARTING")
                         << "Previous engine state: UNKNOWN"
                         << "Success:" << (result == S_OK);
#endif
            } else {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EngineState::Stopped;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendEngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: UNKNOWN"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        }

        return result == S_OK;
    }

    inline bool sendFlapsHandleIndex()
    {
        HRESULT result {S_OK};
        if (m_flapsIndex.needsUpdate()) {
            if (m_flapsIndex.valid) {
                Event event = m_flapsIndex.requested > m_flapsIndex.current ? Event::FlapsIncrease : Event::FlapsDecrease;
                const int steps = std::abs(m_flapsIndex.current - m_flapsIndex.requested);
                for (int step = 0; step < steps; ++step) {
                    result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(event), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                }
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendFlapsHandleIndex: incrementally setting flaps handle index to:" << m_flapsIndex.requested
                         << "Previous index:" << m_flapsIndex.current
                         << "Steps:" << steps
                         << "Event ID:" << Enum::underly(event)
                         << "Success:" << (result == S_OK);
#endif
                if (result == S_OK) {
                    m_flapsIndex.current = m_flapsIndex.requested;
                }
            } else if (!m_flapsIndex.pending) {
                // Request current flaps index
                result = ::SimConnect_RequestDataOnSimObject(m_simConnectHandle, Enum::underly(SimConnectType::DataRequest::FlapsHandleIndex),
                                                             Enum::underly(SimConnectType::DataDefinition::FlapsHandleIndex),
                                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
                if (result == S_OK) {
                    m_flapsIndex.pending = true;
                }
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendFlapsHandleIndex: requesting current flaps index"
                         << "Current index:" << m_flapsIndex.current
                         << "Success:" << (result == S_OK);
#endif
            }
        } // m_requestedFlapsIndex != m_currentFlapsIndex
        return result == S_OK;
    }

    inline bool sendSpoilerPosition(std::int32_t spoilersHandlePosition)
    {
        // Implementation note:
        // Apparently not every aircraft reacts to every simulation event, so we combine a mixture of events here:
        // - Spoilers set: this SHOULD set the spoiler handle exactly to the value that we provide, but some 3rd party aircraft seemingly ignore this event altogether
        // - Axis spoiler set: while this seems to move the spoiler handle for most aircraft there is some "response curve" ("sensitivity") applied -> not what we generally want
        // - Spoilers on/off: some aircraft ignore this as well (but we send it anway, who knows what good it does for other aircraft)
        // - Oh well...
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SpoilersSet), spoilersHandlePosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        if (spoilersHandlePosition == 0) {
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SpoilersOff), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AxisSpoilerSet), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        } else if (spoilersHandlePosition == Max16KPosition) {
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SpoilersOn), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AxisSpoilerSet), Max16KPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }
        return result == S_OK;
    }

    inline bool sendSpoilersArmed(std::int32_t armed)
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SpoilersArmSet), armed, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendGearHandlePosition(bool gearDown)
    {
        const SIMCONNECT_CLIENT_EVENT_ID eventId = gearDown ?Enum::underly(Event::GearDown) : Enum::underly(Event::GearUp);
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, eventId, 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendTailhookHandlePosition(std::int32_t enable)
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SetTailHookHandle), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendSmokeEnabled(std::int32_t enable)
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SmokeSet), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendWingFold(std::int32_t enable)
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::SetWingFold), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendLightState(ToggleSwitch lightSwitch)
    {
        HRESULT result {S_OK};
        if (lightSwitch.needsUpdate()) {
            if (lightSwitch.valid) {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(lightSwitch.toggleEvent), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendLightState: requested state:" << lightSwitch.requested
                         << "Previous state:" << lightSwitch.current
                         << "Event:" << Enum::underly(lightSwitch.toggleEvent)
                         << "Success:" << (result == S_OK);
#endif
                if (result == S_OK) {
                    lightSwitch.current = lightSwitch.requested;
                }
            } else if (!lightSwitch.pending) {
                // Request current light state
                result = ::SimConnect_RequestDataOnSimObject(m_simConnectHandle, Enum::underly(SimConnectType::DataRequest::LightEvent),
                                                             Enum::underly(SimConnectType::DataDefinition::LightEvent),
                                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
                if (result == S_OK) {
                    lightSwitch.pending = true;
                }
#ifdef DEBUG
                qDebug() << "SimConnectEvent::sendLightState: requesting current light state"
                         << "Current index:" << lightSwitch.current
                         << "Event:" << Enum::underly(lightSwitch.toggleEvent)
                         << "Success:" << (result == S_OK);
#endif
            }
        } // needsUpdate
        return result == S_OK;
    }
};

#endif // SIMCONNECTEVENT_H
