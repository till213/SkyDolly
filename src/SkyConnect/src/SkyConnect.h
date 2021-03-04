/**
 * Sky Dolly - The black sheep for your flight recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
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
