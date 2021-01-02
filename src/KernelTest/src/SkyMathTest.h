#ifndef SKYMATHTEST_H
#define SKYMATHTEST_H

#include <QObject>

/*!
 * Test cases for the SkyMath module.
 */
class SkyMathTest : public QObject
{
    Q_OBJECT

private:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void interpolateHermite180_data();
    void interpolateHermite180();

};

#endif // SKYMATHTEST_H
