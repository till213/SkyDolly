#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include "../../Kernel/src/Aircraft.h"
#include "Frequency.h"
#include "Connect.h"
#include "SkyConnectLib.h"

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
    SkyConnectImpl *d;

private slots:
    void frenchConnection();
};

#endif // SKYCONNECT_H
