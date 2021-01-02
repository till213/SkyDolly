#include <QtTest/QtTest>

#include "../../Kernel/src/SkyMath.h"
#include "SkyMathTest.h"

namespace
{
    const double Middle = 0.5;
    const double P1     = 0.0;
    const double P2     = 1.0;
}

// PRIVATE SLOTS

void SkyMathTest::initTestCase()
{
}

void SkyMathTest::cleanupTestCase()
{
}

void SkyMathTest::interpolateHermite180_data()
{
    QTest::addColumn<double>("p0");
    QTest::addColumn<double>("p1");
    QTest::addColumn<double>("p2");
    QTest::addColumn<double>("p3");
    QTest::addColumn<double>("mu");
    QTest::addColumn<double>("expected");

    // Same sign
    QTest::newRow("Positive values middle") << 10.0 << 20.0 << 30.0 << 40.0 << ::Middle << 25.0;
    QTest::newRow("Positive values P1") << 10.0 << 20.0 << 30.0 << 40.0 << ::P1 << 20.0;
    QTest::newRow("Positive values P2") << 10.0 << 20.0 << 30.0 << 40.0 << ::P2 << 30.0;
    QTest::newRow("Negative values middle") << -10.0 << -20.0 << -30.0 << -40.0 << ::Middle << -25.0;
    QTest::newRow("Negative values P1") << -10.0 << -20.0 << -30.0 << -40.0 << ::P1 << -20.0;
    QTest::newRow("Negative values P2") << -10.0 << -20.0 << -30.0 << -40.0 << ::P2 << -30.0;

    // Different sign, switching at 180 [degrees]
    QTest::newRow("Different sign @180 (from negative) middle") << -160.0 << -170.0 << 170.0 << 160.0 << ::Middle << -180.0;
    QTest::newRow("Different sign @180 (from negative) P1") << -160.0 << -170.0 << 170.0 << 160.0 << ::P1 << -170.0;
    QTest::newRow("Different sign @180 (from negative) P2") << -160.0 << -170.0 << 170.0 << 160.0 << ::P2 << 170.0;

    QTest::newRow("Different sign @180 (from positive) middle") << 160.0 << 170.0 << -170.0 << -160.0 << ::Middle << -180.0;
    QTest::newRow("Different sign @180 (from positive) P1") << 160.0 << 170.0 << -170.0 << -160.0 << ::P1 << 170.0;
    QTest::newRow("Different sign @180 (from positive) P2") << 160.0 << 170.0 << -170.0 << -160.0 << ::P2 << -170.0;

    // Different sign, switching at 0 [degrees]
    QTest::newRow("Different sign @0 (from negative) middle") << -20.0 << -10.0 << 10.0 << 20.0 << ::Middle << 0.0;
    QTest::newRow("Different sign @0 (from negative) P1") << -20.0 << -10.0 << 10.0 << 20.0 << ::P1 << -10.0;
    QTest::newRow("Different sign @0 (from negative) P2") << -20.0 << -10.0 << 10.0 << 20.0 << ::P2 << 10.0;

    QTest::newRow("Different sign @0 (from positive) middle") << 20.0 << 10.0 << -10.0 << -20.0 << ::Middle << 0.0;
    QTest::newRow("Different sign @0 (from positive) P1") << 20.0 << 10.0 << -10.0 << -20.0 << ::P1 << 10.0;
    QTest::newRow("Different sign @0 (from positive) P2") << 20.0 << 10.0 << -10.0 << -20.0 << ::P2 << -10.0;
}

void SkyMathTest::interpolateHermite180()
{
    // Setup
    QFETCH(double, p0);
    QFETCH(double, p1);
    QFETCH(double, p2);
    QFETCH(double, p3);
    QFETCH(double, mu);
    QFETCH(double, expected);

    double result = SkyMath::interpolateHermite180(p0, p1, p2, p3, mu);
    QCOMPARE(result, expected);
}

/*!
 * Tests the \c interpolateHermite360 template.
 *
 * Naming conventions:
 * - Quadrant 1: [0, 90[
 * - Quadrant 2: [90, 180[
 * - Quadrant 3: [180, 270[
 * - Quadrant 4: [270, 360[
 */
void SkyMathTest::interpolateHermite360_data()
{
    QTest::addColumn<double>("p0");
    QTest::addColumn<double>("p1");
    QTest::addColumn<double>("p2");
    QTest::addColumn<double>("p3");
    QTest::addColumn<double>("mu");
    QTest::addColumn<double>("expected");

    // Same quadrant
    QTest::newRow("Quadrant 1 values middle") << 10.0 << 20.0 << 30.0 << 40.0 << ::Middle << 25.0;
    QTest::newRow("Quadrant 1 values P1") << 10.0 << 20.0 << 30.0 << 40.0 << ::P1 << 20.0;
    QTest::newRow("Quadrant 1 values P2") << 10.0 << 20.0 << 30.0 << 40.0 << ::P2 << 30.0;
    QTest::newRow("Quadrant 4 values middle") << 350.0 << 340.0 << 330.0 << 320.0 << ::Middle << 335.0;
    QTest::newRow("Quadrant 4 values P1") << 350.0 << 340.0 << 330.0 << 320.0 << ::P1 << 340.0;
    QTest::newRow("Quadrant 4 values P2") << 350.0 << 340.0 << 330.0 << 320.0 << ::P2 << 330.0;

    // Quadrant 1/4 switch (crossing 0/360 degrees)
    QTest::newRow("Quadrant 1/4 switch (from Q1) middle") << 20. << 10.0 << 350.0 << 340.0 << ::Middle << 0.0;
    QTest::newRow("Quadrant 1/4 switch (from Q1) P1") << 20. << 10.0 << 350.0 << 340.0 << ::P1 << 10.0;
    QTest::newRow("Quadrant 1/4 switch (from Q1) P2") << 20. << 10.0 << 350.0 << 340.0 << ::P2 << 350.0;

    QTest::newRow("Quadrant 4/1 switch (from Q4) middle") << 340.0 << 350.0 << 10.0 << 20.0 << ::Middle << 0.0;
    QTest::newRow("Quadrant 4/1 switch (from Q4) P1") << 340.0 << 350.0 << 10.0 << 20.0 << ::P1 << 350.0;
    QTest::newRow("Quadrant 4/1 switch (from Q4) P2") << 340.0 << 350.0 << 10.0 << 20.0 << ::P2 << 10.0;

    // Quadrant 2/3 switch (crossing 180 degrees)
    QTest::newRow("Quadrant 2/3 switch (from Q2) middle") << 160.0 << 170.0 << 190.0 << 200.0 << ::Middle << 180.0;
    QTest::newRow("Quadrant 2/3 switch (from Q2) P1") << 160.0 << 170.0 << 190.0 << 209.0 << ::P1 << 170.0;
    QTest::newRow("Quadrant 2/3 switch (from Q2) P2") << 160.0 << 170.0 << 190.0 << 209.0 << ::P2 << 190.0;

    QTest::newRow("Quadrant 3/2 switch (from Q3) middle") << 200.0 << 190.0 << 170.0 << 160.0 << ::Middle << 180.0;
    QTest::newRow("Quadrant 3/2 switch (from Q3) P1") << 200.0 << 190.0 << 170.0 << 160.0 << ::P1 << 190.0;
    QTest::newRow("Quadrant 3/2 switch (from Q3) P2") << 200.0 << 190.0 << 170.0 << 160.0 << ::P2 << 170.0;
}

void SkyMathTest::interpolateHermite360()
{
    // Setup
    QFETCH(double, p0);
    QFETCH(double, p1);
    QFETCH(double, p2);
    QFETCH(double, p3);
    QFETCH(double, mu);
    QFETCH(double, expected);

    double result = SkyMath::interpolateHermite360(p0, p1, p2, p3, mu);
    QCOMPARE(result, expected);
}

QTEST_MAIN(SkyMathTest)
