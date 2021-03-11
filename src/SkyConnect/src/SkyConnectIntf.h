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

class SKYCONNECT_API SkyConnectIntf : public QObject
{
    Q_OBJECT
public:

    virtual ~SkyConnectIntf() = default;

    virtual void startDataSample() = 0;
    virtual void stopDataSample() = 0;

    virtual void startReplay(bool fromStart) = 0;
    virtual void stopReplay() = 0;
    virtual void stop() = 0;

    virtual void setPaused(bool enabled) = 0;
    virtual bool isPaused() const = 0;

    virtual void skipToBegin() = 0;
    virtual void skipBackward() = 0;
    virtual void skipForward() = 0;
    virtual void skipToEnd() = 0;
    virtual void seek(qint64 timestamp) = 0;
    virtual qint64 getCurrentTimestamp() const = 0;
    virtual bool isAtEnd() const = 0;

    virtual void setTimeScale(double timeScale) = 0;
    virtual double getTimeScale() const = 0;

    virtual Connect::State getState() const = 0;

    virtual Aircraft &getAircraft() = 0;
    virtual const Aircraft &getAircraft() const = 0;
    virtual const AircraftData &getCurrentAircraftData() const = 0;

    virtual double calculateRecordedSamplesPerSecond() const = 0;

protected:
    SkyConnectIntf(QObject *parent = nullptr)
        : QObject(parent)
    {}

signals:
    void aircraftDataSent(qint64 timestamp) const;
    void stateChanged(Connect::State state) const;
};

#endif // SKYCONNECT_H
