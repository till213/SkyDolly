#ifndef SIMCONNECTPOSITION_H
#define SIMCONNECTPOSITION_H

struct SimConnectPosition
{
    double latitude;  // degrees
    double longitude; // degrees
    double altitude;  // feet

    double pitch;     // degrees
    double bank;      // degrees
    double heading;   // degrees
};

#endif // SIMCONNECTPOSITION_H
