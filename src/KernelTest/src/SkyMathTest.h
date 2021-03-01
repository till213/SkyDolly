#ifndef SKYMATHTEST_H
#define SKYMATHTEST_H

#include <QObject>

/*!
 * Test cases for the SkyMath module.
 */
class SkyMathTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void interpolateHermite180_data();
    void interpolateHermite180();

    void interpolateHermite360_data();
    void interpolateHermite360();
};

#endif // SKYMATHTEST_H
