#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "KernelLib.h"
#include "AircraftInfo.h"
#include "AircraftData.h"

class AircraftPrivate;

class KERNEL_API Aircraft : public QObject
{
    Q_OBJECT
public:    
    Aircraft(QObject *parent = nullptr);
    virtual ~Aircraft();

    void setAircraftInfo(AircraftInfo aircraftInfo);
    const AircraftInfo &getAircraftInfo() const;

    void upsertAircraftData(AircraftData aircraftData);
    const AircraftData &getLastAircraftData() const;
    const QVector<AircraftData> getAllAircraftData() const;
    const AircraftData &getAllAircraftData(qint64 timestamp) const;

    void clear();

signals:
    void infoChanged();
    void dataChanged();

private:
    AircraftPrivate *d;

    bool updateCurrentIndex(qint64 timestamp) const;
    bool getSupportData(qint64 timestamp, const AircraftData **p0, const AircraftData **p1, const AircraftData **p2, const AircraftData **p3) const;
    static double normaliseTimestamp(const AircraftData &p1, const AircraftData &p2, quint64 timestamp);
};

#endif // AIRCRAFT_H
