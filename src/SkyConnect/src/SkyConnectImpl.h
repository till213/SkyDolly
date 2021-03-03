#ifndef SKYCONNECTIMPL_H
#define SKYCONNECTIMPL_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/SampleRate.h"
#include "Connect.h"

struct AircraftData;
class SkyConnectPrivate;

class SkyConnectImpl : public QObject
{
    Q_OBJECT
public:
    SkyConnectImpl(QObject *parent = nullptr);
    virtual ~SkyConnectImpl();

    bool open();
    bool close();
    bool isConnected() const;

    void startDataSample();
    void stopDataSample();

    void startReplay(bool fromStart);
    void stopReplay();

    void setPaused(bool enabled);
    bool isPaused() const;

    void skipToBegin();
    void skipBackward();
    void skipForward();
    void skipToEnd();

    Aircraft &getAircraft();
    const Aircraft &getAircraft() const;

    void setTimeScale(double timeScale);
    double getTimeScale() const;

    Connect::State getState() const;

    void setCurrentTimestamp(qint64 timestamp);
    qint64 getCurrentTimestamp() const;
    bool isAtEnd() const;

    const AircraftData &getCurrentAircraftData() const;

signals:
    void aircraftDataSent(qint64 timestamp);
    void stateChanged(Connect::State state);

private:
    SkyConnectPrivate *d;

    void frenchConnection();
    void setupRequestData();
    void setupInitialPosition();
    void setSimulationFrozen(bool enable);
    bool isSimulationFrozen() const;
    bool sendAircraftPosition() const;
    void replay();
    void stopAll();
    void updateCurrentTimestamp();
    void setState(Connect::State state);
    bool hasRecordingStarted() const;

    static void CALLBACK dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);

private slots:
    void processEvents();

    void handleRecordSampleRateChanged(double sampleRateValue);
    void handlePlaybackSampleRateChanged(double sampleRateValue);
};

#endif // SKYCONNECTIMPL_H
