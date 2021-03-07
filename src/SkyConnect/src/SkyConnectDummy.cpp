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
#include "SkyConnectDummy.h"

class SkyConnectDummyPrivate
{
public:
    SkyConnectDummyPrivate()
    {
    }

    AircraftData currentAircraftData;
    Aircraft aircraft;
};

// PUBLIC

SkyConnectDummy::SkyConnectDummy(QObject *parent)
    : SkyConnectIntf(parent),
      d(new SkyConnectDummyPrivate())
{
}


SkyConnectDummy::~SkyConnectDummy()
{
    delete d;
}

void SkyConnectDummy::startDataSample()
{

}

void SkyConnectDummy::stopDataSample()
{

}

void SkyConnectDummy::startReplay(bool fromStart)
{

}

void SkyConnectDummy::stopReplay()
{

}

void SkyConnectDummy::setPaused(bool enabled)
{

}

bool SkyConnectDummy::isPaused() const {
    return false;
}

void SkyConnectDummy::skipToBegin()
{

}

void SkyConnectDummy::skipBackward()
{

}

void SkyConnectDummy::skipForward()
{

}

void SkyConnectDummy::skipToEnd()
{

}

Aircraft &SkyConnectDummy::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnectDummy::getAircraft() const
{
    return d->aircraft;
}

void SkyConnectDummy::setTimeScale(double timeScale)
{

}

double SkyConnectDummy::getTimeScale() const
{
    return 1.0;
}

Connect::State SkyConnectDummy::getState() const
{
    return Connect::State::Idle;
}

void SkyConnectDummy::setCurrentTimestamp(qint64 timestamp)
{

}

qint64 SkyConnectDummy::getCurrentTimestamp() const
{
    return 0;
}

bool SkyConnectDummy::isAtEnd() const
{
    return true;
}

const AircraftData &SkyConnectDummy::getCurrentAircraftData() const
{
    return d->currentAircraftData;
}

