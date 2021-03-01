#ifndef SKYSEARCHTEST_H
#define SKYSEARCHTEST_H

#include <QObject>
#include <QVector>

#include "../../Kernel/src/AircraftData.h"

/*!
 * Test cases for the SkySearch module.
 */
class SkySearchTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void binaryIntervalSearch_data();
    void binaryIntervalSearch();

    void linearIntervalSearch_data();
    void linearIntervalSearch();

private:
     QVector<AircraftData> m_aircraftData;
};

#endif // SKYSEARCHTEST_H
