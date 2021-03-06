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
    const AircraftData &getAircraftData(qint64 timestamp) const;

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
