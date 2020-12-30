#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "KernelLib.h"
#include "Position.h"

class AircraftPrivate;

class KERNEL_API Aircraft : public QObject
{
    Q_OBJECT
public:    
    Aircraft(QObject *parent = nullptr);
    virtual ~Aircraft();

    void setName(QByteArray name);
    const QByteArray &getName() const;

    void appendPosition(Position position);
    const Position &getLastPosition() const;
    const QVector<Position> getPositions() const;    
    const Position &getPosition(qint64 timestamp) const;

    void clear();

signals:
    void infoChanged();
    void positionChanged();

private:
    AircraftPrivate *d;
};

#endif // AIRCRAFT_H
