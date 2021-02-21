#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include "../../Kernel/src/Aircraft.h"
#include "Frequency.h"
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

    Aircraft &getAircraft();
    const Aircraft &getAircraft() const;

    void setSampleFrequency(Frequency::Frequency frequency);
    Frequency::Frequency getSampleFrequency() const;

    void setReplayFrequency(Frequency::Frequency frequency);
    Frequency::Frequency getReplayFrequency() const;

    void setTimeScale(double timeScale);
    double getTimeScale() const;

signals:
    void playPositionChanged(qint64 timeStamp);

private:
    SkyConnectImpl *d;

private slots:
    void frenchConnection();
};

#endif // SKYCONNECT_H
