#include "SkyConnectImpl.h"
#include "SkyConnect.h"

// PUBLIC

SkyConnect::SkyConnect()
    : d(new SkyConnectImpl())
{
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


