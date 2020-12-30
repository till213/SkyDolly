#ifndef SKYCONNECTIMPL_H
#define SKYCONNECTIMPL_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"

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

    Aircraft &getAircraft();
    const Aircraft &getAircraft() const;

private:
    HANDLE m_simConnectHandler;
    QTimer m_timer;
    qint64 m_currentTimestamp;
    QElapsedTimer m_elapsedTimer;
    Aircraft m_aircraft;
    static const int IntervalMilliseconds;

    void frenchConnection();
    void setupRequestData();
    static void CALLBACK sampleDataCallback(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);

private slots:
    void replay();
    void stopAll();
    void sampleData();
};

#endif // SKYCONNECTIMPL_H
