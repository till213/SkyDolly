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

QTEST_MAIN(SkyMathTest)
