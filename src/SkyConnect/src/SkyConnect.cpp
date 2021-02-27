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

Aircraft &SkyConnect::getAircraft()
{
    return d->getAircraft();
}

const Aircraft &SkyConnect::getAircraft() const
{
    return d->getAircraft();
}

void SkyConnect::setSampleFrequency(Frequency::Frequency frequency)
{
    d->setSampleFrequency(frequency);
}

Frequency::Frequency SkyConnect::getSampleFrequency() const
{
    return d->getSampleFrequency();
}

void SkyConnect::setReplayFrequency(Frequency::Frequency frequency)
{
    d->setReplayFrequency(frequency);
}

Frequency::Frequency SkyConnect::getReplayFrequency() const
{
    return d->getReplayFrequency();
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

