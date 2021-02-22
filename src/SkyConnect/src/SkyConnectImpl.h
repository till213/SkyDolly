#ifndef SKYCONNECTIMPL_H
#define SKYCONNECTIMPL_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"
#include "Frequency.h"
#include "Connect.h"

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

    void startReplay();
    void stopReplay();

    void setPaused(bool enabled);
    bool isPaused() const;

    Aircraft &getAircraft();
    const Aircraft &getAircraft() const;

    void setSampleFrequency(Frequency::Frequency frequency);
    Frequency::Frequency getSampleFrequency() const;

    void setReplayFrequency(Frequency::Frequency frequency);
    Frequency::Frequency getReplayFrequency() const;

    void setTimeScale(double timeScale);
    double getTimeScale() const;

    Connect::State getState() const;

signals:
    void playPositionChanged(qint64 timeStamp);
    void stateChanged(Connect::State state);

private:
    SkyConnectPrivate *d;

    void frenchConnection();
    void setupRequestData();
    void setupInitialPosition();
    void setSimulationFrozen(bool enable);
    bool isSimulationFrozen() const;
    static void CALLBACK sampleDataCallback(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);

private slots:
    void setState(Connect::State state);
    void replay();
    void stopAll();
    void sampleData();
};

#endif // SKYCONNECTIMPL_H
