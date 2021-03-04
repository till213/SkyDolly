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
#include "SkyConnectImpl.h"
#include "SkyConnect.h"

// PUBLIC

SkyConnect::SkyConnect()
    : d(new SkyConnectImpl())
{
    this->frenchConnection();
}

SkyConnect::~SkyConnect()
{
    delete d;
}

bool SkyConnect::open()
{
    return d->open();
}

bool SkyConnect::close()
{
    return d->close();
}

bool SkyConnect::isConnected() const
{
    return d->isConnected();
}

void SkyConnect::startDataSample()
{
    d->startDataSample();
}

void SkyConnect::stopDataSample()
{
    d->stopDataSample();
}

void SkyConnect::startReplay(bool fromStart)
{
    d->startReplay(fromStart);
}

void SkyConnect::stopReplay()
{
    d->stopReplay();
}

void SkyConnect::setPaused(bool enabled)
{
    d->setPaused(enabled);
}

bool SkyConnect::isPaused() const
{
    return d->isPaused();
}

void SkyConnect::skipToBegin()
{
    d->skipToBegin();
}

void SkyConnect::skipBackward()
{
    d->skipBackward();
}

void SkyConnect::skipForward()
{
    d->skipForward();
}

void SkyConnect::skipToEnd()
{
    d->skipToEnd();
}

Aircraft &SkyConnect::getAircraft()
{
    return d->getAircraft();
}

const Aircraft &SkyConnect::getAircraft() const
{
    return d->getAircraft();
}

void SkyConnect::setTimeScale(double timeScale)
{
    d->setTimeScale(timeScale);
}

double SkyConnect::getTimeScale() const
{
    return d->getTimeScale();
}

Connect::State SkyConnect::getState() const
{
    return d->getState();
}

void SkyConnect::setCurrentTimestamp(qint64 timestamp)
{
    d->setCurrentTimestamp(timestamp);
}

qint64 SkyConnect::getCurrentTimestamp() const
{
    return d->getCurrentTimestamp();
}

bool SkyConnect::isAtEnd() const
{
    return d->isAtEnd();
}

const AircraftData &SkyConnect::getCurrentAircraftData() const
{
    return d->getCurrentAircraftData();
}

// PRIVATE SLOTS

void SkyConnect::frenchConnection() {
    connect(d, &SkyConnectImpl::aircraftDataSent,
            this, &SkyConnect::aircraftDataSent);
    connect(d, &SkyConnectImpl::stateChanged,
            this, &SkyConnect::stateChanged);
}

