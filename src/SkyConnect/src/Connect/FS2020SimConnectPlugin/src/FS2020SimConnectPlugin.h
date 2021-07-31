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

#include <QObject>

#include <windows.h>
#include <SimConnect.h>

#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Model/src/TimeVariableData.h"
#include "../../../../../Model/src/InitialPosition.h"
#include "../../../AbstractSkyConnect.h"

struct PositionData;
class Aircraft;
class SkyConnectPrivate;

class FS2020SimConnectPlugin : public AbstractSkyConnect
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SKYCONNECT_INTERFACE_IID FILE "FS2020SimConnectPlugin.json")
    Q_INTERFACES(SkyConnectIntf)
public:
    FS2020SimConnectPlugin(QObject *parent = nullptr) noexcept;
    virtual ~FS2020SimConnectPlugin() noexcept;

    virtual bool setUserAircraftPosition(const PositionData &positionData) noexcept override;

protected:
    virtual bool isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept override;

    virtual bool onInitialPositionSetup(const InitialPosition &initialPosition) noexcept override;
    virtual bool onFreezeUserAircraft(bool enable) noexcept override;

    virtual bool onStartRecording() noexcept override;
    virtual void onRecordingPaused(bool paused ) noexcept override;
    virtual void onStopRecording() noexcept override;

    virtual bool onStartReplay(qint64 currentTimestamp) noexcept override;
    virtual void onReplayPaused(bool paused) noexcept override;
    virtual void onStopReplay() noexcept override;

    virtual void onSeek(qint64 currentTimestamp) noexcept override;
    virtual void onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept override;

    virtual bool sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept override;
    virtual bool isConnectedWithSim() const noexcept override;
    virtual bool connectWithSim() noexcept override;

    virtual bool onCreateAIObjects() noexcept override;
    virtual void onDestroyAIObjects() noexcept override;
    virtual void onDestroyAIObject(Aircraft &aircraft) noexcept override;

protected slots:
    virtual void recordData() noexcept override;

private:
    std::unique_ptr<SkyConnectPrivate> d;

    void frenchConnection() noexcept;

    void resetCurrentSampleData() noexcept;
    bool reconnectWithSim() noexcept;
    bool close() noexcept;
    void setupRequestData() noexcept;
    bool setAircraftFrozen(::SIMCONNECT_OBJECT_ID objectId, bool enable) noexcept;
    bool sendAircraftData(TimeVariableData::Access access) noexcept;
    inline bool updateAndSendEngineStartEvent(qint64 objectId, const EngineData &engineData, TimeVariableData::Access access) noexcept;
    void replay() noexcept;
    void updateRecordingFrequency(SampleRate::SampleRate sampleRate) noexcept;
    void updateRequestPeriod(::SIMCONNECT_PERIOD period) noexcept;

    static void CALLBACK dispatch(::SIMCONNECT_RECV *receivedData, DWORD cbData, void *context) noexcept;

private slots:
    void processSimConnectEvent() noexcept;
};

#endif // FS2020SIMCONNNECTPLUGIN_H
