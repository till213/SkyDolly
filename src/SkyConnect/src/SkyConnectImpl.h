#ifndef SKYCONNECTIMPL_H
#define SKYCONNECTIMPL_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"

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

    Aircraft &getAircraft();
    const Aircraft &getAircraft() const;

private:
    SkyConnectPrivate *d;

    void frenchConnection();
    void setupRequestData();
    void setupInitialPosition();
    static void CALLBACK sampleDataCallback(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);

private slots:
    void replay();
    void stopAll();
    void sampleData();
};

#endif // SKYCONNECTIMPL_H
