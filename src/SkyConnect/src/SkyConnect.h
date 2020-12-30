#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include "../../Kernel/src/Aircraft.h"
#include "SkyConnectLib.h"

class SkyConnectImpl;

class SKYCONNECT_API SkyConnect
{
public:
    SkyConnect();
    ~SkyConnect();

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
    SkyConnectImpl *d;
};

#endif // SKYCONNECT_H
