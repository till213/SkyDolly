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

void SkyConnect::startReplay()
{
    d->startReplay();
}

void SkyConnect::stopReplay()
{
    d->stopReplay();
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

// Private slots

void SkyConnect::frenchConnection() {
    connect(d, &SkyConnectImpl::playPositionChanged,
            this, &SkyConnect::playPositionChanged);
}

