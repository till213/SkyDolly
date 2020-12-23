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

    const Aircraft &getAircraft() const;

signals:
    void aircraftChanged();

private:
    SimConnectPrivate *d;

    static void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
    void frenchConnection();
    void setupRequestData();

private slots:
    void sampleData();
};

#endif // SKYCONNECT_H
