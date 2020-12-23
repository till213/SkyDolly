#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <QObject>
#include <QByteArray>
#include <QVector>

#include "Position.h"

class AircraftPrivate;

class Aircraft : public QObject
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

    void clear();

signals:
    void infoChanged();
    void positionChanged();

private:
    AircraftPrivate *d;
};

#endif // AIRCRAFT_H
