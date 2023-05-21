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
#ifndef FS2020SIMCONNNECTPLUGIN_H
#define FS2020SIMCONNNECTPLUGIN_H

#include <memory>
#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <QObject>

#include <Kernel/FlightSimulatorShortcuts.h>
#include <Kernel/SampleRate.h>
#include <Model/TimeVariableData.h>
#include <Model/InitialPosition.h>
#include <PluginManager/Connect/AbstractSkyConnect.h>

struct PositionData;
struct EngineData;
class Aircraft;
class SkyConnectPrivate;

class MSFSSimConnectPlugin : public AbstractSkyConnect
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SKYCONNECT_INTERFACE_IID FILE "MSFSSimConnectPlugin.json")
    Q_INTERFACES(SkyConnectIntf)
public:
    MSFSSimConnectPlugin(QObject *parent = nullptr) noexcept;
    MSFSSimConnectPlugin(const MSFSSimConnectPlugin &rhs) = delete;
    MSFSSimConnectPlugin(MSFSSimConnectPlugin &&rhs) = delete;
    MSFSSimConnectPlugin &operator=(const MSFSSimConnectPlugin &rhs) = delete;
    MSFSSimConnectPlugin &operator=(MSFSSimConnectPlugin &&rhs) = delete;
    ~MSFSSimConnectPlugin() noexcept override;

    bool setUserAircraftPosition(const PositionData &positionData) noexcept override;

protected:
    bool isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept override;

    bool onSetupFlightSimulatorShortcuts(const FlightSimulatorShortcuts &shortcuts) noexcept override;

    bool onInitialPositionSetup(const InitialPosition &initialPosition) noexcept override;
    bool onFreezeUserAircraft(bool enable) const noexcept override;
    bool onSimulationEvent(SimulationEvent event, float arg1) const noexcept override;

    bool onStartFlightRecording() noexcept override;
    bool onStartAircraftRecording() noexcept override;
    void onRecordingPaused(Initiator initiator, bool enable) noexcept override;
    void onStopRecording() noexcept override;

    bool onStartReplay(std::int64_t currentTimestamp) noexcept override;
    void onReplayPaused(Initiator initiator, bool enable) noexcept override;
    void onStopReplay() noexcept override;

    void onSeek(std::int64_t currentTimestamp, SeekMode seekMode) noexcept override;
    void onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept override;

    bool sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept override;
    bool isConnectedWithSim() const noexcept override;
    bool connectWithSim() noexcept override;

    void onAddAiObject(const Aircraft &aircraft) noexcept override;
    void onRemoveAiObject(std::int64_t aircraftId) noexcept override;
    void onRemoveAllAiObjects() noexcept override;

    bool onRequestLocation() noexcept override;

protected slots:
    void recordData() noexcept override;

private:
    std::unique_ptr<SkyConnectPrivate> d;

    void frenchConnection() noexcept;

    void resetCurrentSampleData() noexcept;
    bool reconnectWithSim() noexcept;
    bool close() noexcept;
    void setupRequestData() noexcept;
    bool sendAircraftData(TimeVariableData::Access access) noexcept;
    void replay() noexcept;
    void updateRecordingFrequency(SampleRate::SampleRate sampleRate) noexcept;
    void updateRequestPeriod(::SIMCONNECT_PERIOD period) noexcept;
    void resetEventStates() noexcept;

    static void CALLBACK dispatch(::SIMCONNECT_RECV *receivedData, DWORD cbData, void *context) noexcept;

private slots:
    void processSimConnectEvent() noexcept;
};

#endif // FS2020SIMCONNNECTPLUGIN_H
