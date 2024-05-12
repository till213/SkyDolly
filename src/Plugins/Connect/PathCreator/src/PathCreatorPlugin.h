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
#ifndef PATHCREATORPLUGIN_H
#define PATHCREATORPLUGIN_H

#include <memory>
#include <cstdint>
#include <optional>

#include <QObject>

#include <Model/TimeVariableData.h>
#include <PluginManager/Connect/AbstractSkyConnect.h>
#include <PluginManager/Connect/FlightSimulatorShortcuts.h>
#include <PluginManager/Connect/SkyConnectIntf.h>

class Flight;
class Aircraft;
struct PositionData;
struct AttitudeData;
struct InitialPosition;
class ConnectPluginBaseSettings;
class OptionWidgetIntf;
struct PathCreatorPluginPrivate;

class PathCreatorPlugin final : public AbstractSkyConnect
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SKYCONNECT_INTERFACE_IID FILE "PathCreatorPlugin.json")
    Q_INTERFACES(SkyConnectIntf)
public:
    PathCreatorPlugin(QObject *parent = nullptr) noexcept;
    PathCreatorPlugin(const PathCreatorPlugin &rhs) = delete;
    PathCreatorPlugin(PathCreatorPlugin &&rhs) = delete;
    PathCreatorPlugin &operator=(const PathCreatorPlugin &rhs) = delete;
    PathCreatorPlugin &operator=(PathCreatorPlugin &&rhs) = delete;
    ~PathCreatorPlugin() override;
    
    bool setUserAircraftPositionAndAttitude(const PositionData &positionData, const AttitudeData &attitudeData) noexcept override;

protected:
    ConnectPluginBaseSettings &getPluginSettings() const noexcept override;
    std::optional<std::unique_ptr<OptionWidgetIntf>> createExtendedOptionWidget() const noexcept override;

    bool onSetupFlightSimulatorShortcuts() noexcept override;

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

    void onSeek(std::int64_t currentTimestampS, SeekMode seekMode) noexcept override;

    bool sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept override;
    bool isConnectedWithSim() const noexcept override;
    bool connectWithSim() noexcept override;
    void onDisconnectFromSim() noexcept override;

    void onAddAiObject(const Aircraft &aircraft) noexcept override;
    void onRemoveAiObject(std::int64_t aircraftId) noexcept override;
    void onRemoveAllAiObjects() noexcept override;

    bool onRequestLocation() noexcept override;
    bool onRequestSimulationRate() noexcept override;
    bool onSendDateAndTime(int year, int day, int hour, int minute) const noexcept override;

private:
    const std::unique_ptr<PathCreatorPluginPrivate> d;

    void frenchConnection() noexcept;
    void recordPositionData(std::int64_t timestamp) noexcept;
    void recordEngineData(std::int64_t timestamp) noexcept;
    void recordPrimaryControls(std::int64_t timestamp) noexcept;
    void recordSecondaryControls(std::int64_t timestamp) noexcept;
    void recordAircraftHandle(std::int64_t timestamp) noexcept;
    void recordLights(std::int64_t timestamp) noexcept;
    void recordWaypoint(std::int64_t timestamp) noexcept;
    void recordFlightInfo() noexcept;
    void recordFlightCondition() noexcept;
    void recordAircraftInfo() noexcept;
    void closeConnection() noexcept;

private slots:
    void replay() noexcept;
    void recordData() noexcept;
};

#endif // PATHCREATORPLUGIN_H
