#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/SampleRate.h"
#include "Connect.h"
#include "SkyConnectLib.h"

struct AircraftData;
class SkyConnectImpl;

class SKYCONNECT_API SkyConnect : public QObject
{
    Q_OBJECT
public:
    SkyConnect();
    ~SkyConnect();

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
    SkyConnectImpl *d;

private slots:
    void frenchConnection();
};

#endif // SKYCONNECT_H
