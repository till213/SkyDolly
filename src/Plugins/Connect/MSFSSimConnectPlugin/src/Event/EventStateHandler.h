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
#ifndef EVENTSTATEHANDLER_H
#define EVENTSTATEHANDLER_H

#include <cstdint>
#include <algorithm>
#include <array>

#include <windows.h>
#include <SimConnect.h>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/SimType.h>
#include <Model/TimeVariableData.h>
#include "Engine/SimConnectEngineEvent.h"
#include "Engine/SimConnectEngineAll.h"
#include "PrimaryFlightControl/SimConnectPrimaryFlightControlEvent.h"
#include "SecondaryFlightControl/SimConnectSecondaryFlightControlEvent.h"
#include "AircraftHandle/SimConnectAircraftHandleAll.h"
#include "Light/SimConnectLightEvent.h"
#include "EventState.h"
#include "SimConnectEvent.h"
#include "SimConnectType.h"

class EventStateHandler final
{
public:
    EventStateHandler(::HANDLE simConnectHandle = nullptr) noexcept
        : m_simConnectHandle(simConnectHandle)
    {}

    ::HANDLE getSimConnectHandle() const noexcept
    {
        return m_simConnectHandle;
    }

    void setSimConnectHandle(::HANDLE simConnectHandle) noexcept
    {
        m_simConnectHandle = simConnectHandle;
    }

    static constexpr std::int16_t Min16KPosition {-16383};
    static constexpr std::int16_t Max16KPosition {16384};

    /*!
     * Converts the normalised \c value to an \em event (position) value.
     *
     * @param value
     *        the normalised value to be converted [-1.0, 1.0]
     * @return the converted \em event (position) value [-16384, 16384]
     */
    static std::int16_t positionTo16K(double value) noexcept
    {
        return static_cast<std::int16_t>(std::round(std::clamp(value, -1.0, 1.0) * Max16KPosition));
    }

    /*!
     * Converts the \c percent to an \em event (position) value.
     *
     * @param percent
     *        the percent value to be converted [0, 100]
     * @return the converted \em event (position) value [-16384, 16384]
     */
    static std::int16_t percentTo16K(double percent) noexcept
    {
        return static_cast<std::int16_t>(std::round(percent * (double)Max16KPosition / 100.0));
    }

    void setupEvents() noexcept
    {
        // System event subscription
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SimStart), "SimStart");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::Pause), "Pause");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::Crashed), "Crashed");

        // Client events
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::PauseSet), "PAUSE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SimRateIncr), "SIM_RATE_INCR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SimRateDecr), "SIM_RATE_DECR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::FreezeLatituteLongitude), "FREEZE_LATITUDE_LONGITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::FreezeAltitude), "FREEZE_ALTITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::FreezeAttitude), "FREEZE_ATTITUDE_SET");
        // Engine
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::EngineAutoStart), "ENGINE_AUTO_START");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), "ENGINE_AUTO_SHUTDOWN");
        // Primary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::AxisAileronsSet), "AXIS_AILERONS_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::AxisElevatorSet), "AXIS_ELEVATOR_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::AxisRudderSet), "AXIS_RUDDER_SET");
        // Secondary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::FlapsDecrease), "FLAPS_DECR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::FlapsIncrease), "FLAPS_INCR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SpoilersSet), "SPOILERS_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::AxisSpoilerSet), "AXIS_SPOILER_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SpoilersOff), "SPOILERS_OFF");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SpoilersOn), "SPOILERS_ON");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SpoilersArmSet), "SPOILERS_ARM_SET");
        // Handles
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::GearUp), "GEAR_UP");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::GearDown), "GEAR_DOWN");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SetTailHookHandle), "SET_TAIL_HOOK_HANDLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SetWingFold), "SET_WING_FOLD");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::SmokeSet), "SMOKE_SET");
        // Lights
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleNavLights), "TOGGLE_NAV_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleBeaconLights), "TOGGLE_BEACON_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::LandingLightsToggle), "LANDING_LIGHTS_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleTaxiLights), "TOGGLE_TAXI_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::StrobesToggle), "STROBES_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::PanelLightsToggle), "PANEL_LIGHTS_TOGGLE");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleRecognitionLights), "TOGGLE_RECOGNITION_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleWingLights), "TOGGLE_WING_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleLogoLights), "TOGGLE_LOGO_LIGHTS");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(SimConnectEvent::Event::ToggleCabinLights), "TOGGLE_CABIN_LIGHTS");
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

    inline bool sendEngine(const SimConnectEngineAll &engine, TimeVariableData::Access access) noexcept
    {
        bool ok {true};
        if (access != TimeVariableData::Access::ContinuousSeek) {
            ok = sendEngineState(engine);
        }
        return ok;
    }

    inline bool sendPrimaryFlightControl(const SimConnectPrimaryFlightControlEvent &event) noexcept
    {
        // The recorded control surface values have opposite sign than the event values to be sent
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::AxisRudderSet), -positionTo16K(event.rudderPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::AxisAileronsSet), -positionTo16K(event.aileronPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::AxisElevatorSet), -positionTo16K(event.elevatorPosition), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

        return result == S_OK;
    }

    inline bool sendSecondaryFlightControl(const SimConnectSecondaryFlightControlEvent &event, TimeVariableData::Access access) noexcept
    {
        bool ok = sendSpoilersArmed(event.spoilersArmed);
        if (ok) {
            const bool spoilersArmed = event.spoilersArmed != 0;
            ok = sendSpoilerPosition(percentTo16K(event.spoilersHandlePosition), spoilersArmed);
        }
        if (ok && access != TimeVariableData::Access::ContinuousSeek) {
            m_flapsIndex.requested = event.flapsHandleIndex;
            ok = sendFlapsHandleIndex();
        }
        return ok;
    }

    inline bool sendAircraftHandle(const SimConnectAircraftHandleAll &aircraftHandle) noexcept
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

    inline bool sendLight(const SimConnectLightEvent &event) noexcept
    {
        // First set the requested values...
        m_navigationLightToggle.requested = event.navigation;
        m_beaconLightToggle.requested = event.beacon;
        m_landingLightToggle.requested = event.landing;
        m_taxiLightToggle.requested = event.taxi;
        m_strobeLightToggle.requested = event.taxi;
        m_panelLightToggle.requested = event.panel;
        m_recognitionLightToggle.requested = event.recognition;
        m_wingLightToggle.requested = event.wing;
        m_logoLightToggle.requested = event.logo;
        m_cabinLightToggle.requested = event.cabin;

        // ... then check the remote states (which will trigger
        // a "set light" if needed)
        bool ok = testLight(m_navigationLightToggle);
        ok = ok && testLight(m_beaconLightToggle);
        ok = ok && testLight(m_landingLightToggle);
        ok = ok && testLight(m_taxiLightToggle);
        ok = ok && testLight(m_strobeLightToggle);
        ok = ok && testLight(m_panelLightToggle);
        ok = ok && testLight(m_recognitionLightToggle);
        ok = ok && testLight(m_wingLightToggle);
        ok = ok && testLight(m_logoLightToggle);
        ok = ok && testLight(m_cabinLightToggle);

        return ok;
    }

    inline void setCurrentFlapsHandleIndex(std::int32_t index) noexcept
    {
        m_flapsIndex.current = index;
        m_flapsIndex.valid = true;
        sendFlapsHandleIndex();
    }

    inline bool setNavigationLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_navigationLightToggle);
    }

    inline bool setBeaconLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_beaconLightToggle);
    }

    inline bool setLandingLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_landingLightToggle);
    }

    inline bool setTaxiLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_taxiLightToggle);
    }

    inline bool setStrobeLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_strobeLightToggle);
    }

    inline bool setPanelLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_panelLightToggle);
    }

    inline bool setRecognitionLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_recognitionLightToggle);
    }

    inline bool setWingLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_wingLightToggle);
    }

    inline bool setLogoLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_logoLightToggle);
    }

    inline bool setCabinLight(std::int32_t enabled) noexcept
    {
        const bool remoteState = (enabled != 0);
        return sendLight(remoteState, m_cabinLightToggle);
    }

    inline void reset() noexcept
    {
        // Engine
        m_engineState = EventState::Engine::Unknown;
        // Flaps
        m_flapsIndex.reset();
        // Lights
        m_navigationLightToggle.reset();
        m_beaconLightToggle.reset();
        m_landingLightToggle.reset();
        m_taxiLightToggle.reset();
        m_strobeLightToggle.reset();
        m_panelLightToggle.reset();
        m_recognitionLightToggle.reset();
        m_wingLightToggle.reset();
        m_logoLightToggle.reset();
        m_cabinLightToggle.reset();
    }

private:
    ::HANDLE m_simConnectHandle {nullptr};
    EventState::Engine m_engineState {EventState::Engine::Unknown};

    EventState::StatefulSwitch<std::int32_t> m_flapsIndex;

    // Implementation note:
    // Some lights - notably the Navigation and Logo light in the A320neo - may interact with
    // each other, so we treat them as stateless ("test and set")
    EventState::StatelessToggle m_navigationLightToggle {SimConnectEvent::Event::ToggleNavLights, SimConnectType::DataRequest::NavigationLight, SimConnectType::DataDefinition::NavigationLight};
    EventState::StatelessToggle m_beaconLightToggle {SimConnectEvent::Event::ToggleBeaconLights, SimConnectType::DataRequest::BeaconLight, SimConnectType::DataDefinition::BeaconLight};
    EventState::StatelessToggle m_landingLightToggle {SimConnectEvent::Event::LandingLightsToggle, SimConnectType::DataRequest::LandingLight, SimConnectType::DataDefinition::LandingLight};
    EventState::StatelessToggle m_taxiLightToggle {SimConnectEvent::Event::ToggleTaxiLights, SimConnectType::DataRequest::TaxiLight, SimConnectType::DataDefinition::TaxiLight};
    EventState::StatelessToggle m_strobeLightToggle {SimConnectEvent::Event::StrobesToggle, SimConnectType::DataRequest::StrobeLight, SimConnectType::DataDefinition::StrobeLight};
    EventState::StatelessToggle m_panelLightToggle {SimConnectEvent::Event::PanelLightsToggle, SimConnectType::DataRequest::PanelLight, SimConnectType::DataDefinition::PanelLight};
    EventState::StatelessToggle m_recognitionLightToggle {SimConnectEvent::Event::ToggleRecognitionLights, SimConnectType::DataRequest::RecognitionLight, SimConnectType::DataDefinition::RecognitionLight};
    EventState::StatelessToggle m_wingLightToggle {SimConnectEvent::Event::ToggleWingLights, SimConnectType::DataRequest::WingLight, SimConnectType::DataDefinition::WingLight};
    EventState::StatelessToggle m_logoLightToggle {SimConnectEvent::Event::ToggleLogoLights, SimConnectType::DataRequest::LogoLight, SimConnectType::DataDefinition::LogoLight};
    EventState::StatelessToggle m_cabinLightToggle {SimConnectEvent::Event::ToggleCabinLights, SimConnectType::DataRequest::CabinLight, SimConnectType::DataDefinition::CabinLight};

    bool m_paused {false};

    inline bool sendEngineState(const SimConnectEngineAll &engine) noexcept
    {
        HRESULT result {S_OK};
        const SimConnectEngineCore &engineCore = engine.core;
        const SimConnectEngineEvent &engineEvent = engine.event;
        switch (m_engineState) {
        case EventState::Engine::Starting:
            if (engineEvent.hasCombustion()) {
                m_engineState = EventState::Engine::Started;
            } else if (!engineCore.hasEngineStarterEnabled()) {
                 // STARTING: Engine started disabled, no combustion -> STOPPED
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EventState::Engine::Stopped;
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendEventState::EngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: STARTING"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        case EventState::Engine::Started:
            if (!engineEvent.hasCombustion()) {
                // STARTED: No combustion -> STOPPED
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EventState::Engine::Stopped;
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendEventState::EngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: STARTED"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        case EventState::Engine::Stopped:
            // Either general engine starter has been enabled or combustion has started -> engine start
            // Note: apparently the engine starter can be disabled (false) and yet with an active combustion (= running engine)
            //       specifically in the case when the aircraft has been "auto-started" (CTRL + E)
            if (engineCore.hasEngineStarterEnabled() || engineEvent.hasCombustion()) {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EventState::Engine::Starting;
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendEventState::EngineState: starting engines"
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
                m_engineState = engineEvent.hasCombustion() ? EventState::Engine::Started : EventState::Engine::Starting;
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendEventState::EngineState: starting engines"
                         << "Current engine state:" << (engineEvent.hasCombustion() ? "STARTED" : "STARTING")
                         << "Previous engine state: UNKNOWN"
                         << "Success:" << (result == S_OK);
#endif
            } else {
                result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                m_engineState = EventState::Engine::Stopped;
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendEventState::EngineState: stopping engines"
                         << "Current engine state: STOPPED"
                         << "Previous engine state: UNKNOWN"
                         << "Success:" << (result == S_OK);
#endif
            }
            break;
        }

        return result == S_OK;
    }

    inline bool sendFlapsHandleIndex() noexcept
    {
        HRESULT result {S_OK};
        if (m_flapsIndex.needsUpdate()) {
            if (m_flapsIndex.valid) {
                SimConnectEvent::Event event = m_flapsIndex.requested > m_flapsIndex.current ? SimConnectEvent::Event::FlapsIncrease : SimConnectEvent::Event::FlapsDecrease;
                const int steps = std::abs(m_flapsIndex.current - m_flapsIndex.requested);
                for (int step = 0; step < steps; ++step) {
                    result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(event), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                }
#ifdef DEBUG
                qDebug() << "EventStateHandler::sendFlapsHandleIndex: incrementally setting flaps handle index to:" << m_flapsIndex.requested
                         << "Previous index:" << m_flapsIndex.current
                         << "Steps:" << steps
                         << "Event ID:" << Enum::underly(event)
                         << "Success:" << (result == S_OK);
#endif
                if (result == S_OK) {
                    m_flapsIndex.current = m_flapsIndex.requested;
                }
                m_flapsIndex.pending = false;
            } else if (!m_flapsIndex.pending) {
                // Request current flaps index
                result = ::SimConnect_RequestDataOnSimObject(m_simConnectHandle, Enum::underly(SimConnectType::DataRequest::FlapsHandleIndex),
                                                             Enum::underly(SimConnectType::DataDefinition::FlapsHandleIndex),
                                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
                if (result == S_OK) {
                    m_flapsIndex.pending = true;
                }
            }
        } // needsUpdate
        return result == S_OK;
    }

    inline bool sendSpoilerPosition(std::int32_t spoilersHandlePosition, bool armed) noexcept
    {
        // Implementation note:
        // Apparently not every aircraft reacts to every simulation event, so we combine a mixture of events here:
        // - Spoilers set: this SHOULD set the spoiler handle exactly to the value that we provide, but some 3rd party aircraft seemingly ignore this event altogether
        // - Axis spoiler set: while this seems to move the spoiler handle for most aircraft there is some "response curve" ("sensitivity") applied -> not what we generally want
        // - Spoilers on/off: some aircraft ignore this as well (but we send it anway, who knows what good it does for other aircraft)
        // - Oh well...
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SpoilersSet), spoilersHandlePosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        if (spoilersHandlePosition == 0 && !armed) {
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SpoilersOff), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::AxisSpoilerSet), Min16KPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        } else if (spoilersHandlePosition == Max16KPosition) {
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SpoilersOn), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            result |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::AxisSpoilerSet), Max16KPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }
        return result == S_OK;
    }

    inline bool sendSpoilersArmed(std::int32_t armed) noexcept
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SpoilersArmSet), armed, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendGearHandlePosition(bool gearDown) noexcept
    {
        const SIMCONNECT_CLIENT_EVENT_ID eventId = gearDown ? Enum::underly(SimConnectEvent::Event::GearDown) : Enum::underly(SimConnectEvent::Event::GearUp);
        HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, eventId, 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendTailhookHandlePosition(std::int32_t enable) noexcept
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SetTailHookHandle), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendSmokeEnabled(std::int32_t enable) noexcept
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SmokeSet), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool sendWingFold(std::int32_t enable) noexcept
    {
        const HRESULT result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::SetWingFold), enable, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return result == S_OK;
    }

    inline bool testLight(EventState::StatelessToggle &lightToggle) noexcept
    {
        HRESULT result {S_OK};

        if (!lightToggle.pending) {
            // Request current remote light state
            result = ::SimConnect_RequestDataOnSimObject(m_simConnectHandle, Enum::underly(lightToggle.dataRequest),Enum::underly(lightToggle.dataDefinition),
                                                         ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
            if (result == S_OK) {
                lightToggle.pending = true;
            }
        }
        return result == S_OK;
    }

    // Updates the remote light state according to 'lightToggle', given the current 'remoteState'
    // if needed and resets the 'pending' state (false) of the 'lightToggle'
    inline bool sendLight(bool remoteState, EventState::StatelessToggle &lightToggle) noexcept
    {
        HRESULT result {S_OK};

        // Implementation note:
        // - Setting the light (e.g. "NAV_LIGHTS_SET") is immediatelly "overridden" again by the switch logic of certain aircraft (e.g. PMDG 737-800)
        // - Some light switches are combined, e.g. "Navigation & Logo", so toggling both will "unset" the previous state change again (e.g. Asobo A320neo)
        // - So while "toggle" seems to work in most cases (except e.g. for "Logo" alone) we always need to query the current state before toggling - each time
        // - Oh well...
        if (lightToggle.needsUpdate(remoteState)) {
            result = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(lightToggle.toggleEvent), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
#ifdef DEBUG
            qDebug() << "EventStateHandler::setLight: requested state:" << lightToggle.requested
                     << "remoteState:" << remoteState
                     << "event:" << Enum::underly(lightToggle.toggleEvent)
                     << "success:" << (result == S_OK);
#endif
        }
        lightToggle.pending = false;
        return result == S_OK;
    }
};

#endif // EVENTSTATEHANDLER_H
