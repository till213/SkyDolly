#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include <windows.h>

#include <QTimer>
#include <QObject>

#include "SimConnect.h"

class SimConnectPrivate;
class Aircraft;

class SkyConnect : public QObject
{
    Q_OBJECT
public:
    SkyConnect(QObject *parent = nullptr);
    virtual ~SkyConnect();

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
    SimConnectPrivate *d;

    static void CALLBACK sampleDataCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void frenchConnection();
    void setupRequestData();

private slots:
    void sampleData();
    void replay();
};

#endif // SKYCONNECT_H
