/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <limits>
#include <utility>
#include <cstdint>
#include <cmath>
#include <array>

#include <QtTest>
#include <QDateTime>

#include <Kernel/SkyMath.h>
#include "SkyMathTest.h"

namespace
{
    constexpr double Middle = 0.5;
    constexpr double P1     = 0.0;
    constexpr double P2     = 1.0;
}

// PRIVATE SLOTS

void SkyMathTest::initTestCase()
{}

void SkyMathTest::cleanupTestCase()
{}

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
    QTest::newRow("Different sign @180 (from negative) @middle") << -160.0 << -170.0 << 170.0 << 160.0 << ::Middle << -180.0;
    QTest::newRow("Different sign @180 (from negative) @P1") << -160.0 << -170.0 << 170.0 << 160.0 << ::P1 << -170.0;
    QTest::newRow("Different sign @180 (from negative) @P2") << -160.0 << -170.0 << 170.0 << 160.0 << ::P2 << 170.0;

    QTest::newRow("Different sign @180 (from positive) @middle") << 160.0 << 170.0 << -170.0 << -160.0 << ::Middle << -180.0;
    QTest::newRow("Different sign @180 (from positive) @P1") << 160.0 << 170.0 << -170.0 << -160.0 << ::P1 << 170.0;
    QTest::newRow("Different sign @180 (from positive) @P2") << 160.0 << 170.0 << -170.0 << -160.0 << ::P2 << -170.0;

    // Different sign, switching at 0 [degrees]
    QTest::newRow("Different sign @0 (from negative) @middle") << -20.0 << -10.0 << 10.0 << 20.0 << ::Middle << 0.0;
    QTest::newRow("Different sign @0 (from negative) @P1") << -20.0 << -10.0 << 10.0 << 20.0 << ::P1 << -10.0;
    QTest::newRow("Different sign @0 (from negative) @P2") << -20.0 << -10.0 << 10.0 << 20.0 << ::P2 << 10.0;

    QTest::newRow("Different sign @0 (from positive) @middle") << 20.0 << 10.0 << -10.0 << -20.0 << ::Middle << 0.0;
    QTest::newRow("Different sign @0 (from positive) @P1") << 20.0 << 10.0 << -10.0 << -20.0 << ::P1 << 10.0;
    QTest::newRow("Different sign @0 (from positive) @P2") << 20.0 << 10.0 << -10.0 << -20.0 << ::P2 << -10.0;
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

    // Exercise
    double result = SkyMath::interpolateHermite180(p0, p1, p2, p3, mu);

    // Verify
    QCOMPARE(result, expected);
}

// Quadrants (for testing purposes)
//      N
//   Q4 | Q1
// W ------- E
//   Q3 | Q2
//      S
//
// - Quadrant 1: [0, 90[
// - Quadrant 2: [90, 180[
// - Quadrant 3: [180, 270[
// - Quadrant 4: [270, 360[

void SkyMathTest::interpolateHermite360_data()
{
    QTest::addColumn<double>("p0");
    QTest::addColumn<double>("p1");
    QTest::addColumn<double>("p2");
    QTest::addColumn<double>("p3");
    QTest::addColumn<double>("mu");
    QTest::addColumn<double>("expected");

    // Same quadrant
    QTest::newRow("Quadrant 1 values @middle") << 10.0 << 20.0 << 30.0 << 40.0 << ::Middle << 25.0;
    QTest::newRow("Quadrant 1 values @P1") << 10.0 << 20.0 << 30.0 << 40.0 << ::P1 << 20.0;
    QTest::newRow("Quadrant 1 values @P2") << 10.0 << 20.0 << 30.0 << 40.0 << ::P2 << 30.0;
    QTest::newRow("Quadrant 4 values @middle") << 350.0 << 340.0 << 330.0 << 320.0 << ::Middle << 335.0;
    QTest::newRow("Quadrant 4 values @P1") << 350.0 << 340.0 << 330.0 << 320.0 << ::P1 << 340.0;
    QTest::newRow("Quadrant 4 values @P2") << 350.0 << 340.0 << 330.0 << 320.0 << ::P2 << 330.0;

    // Quadrant 1/4 switch (crossing 0/360 degrees)
    QTest::newRow("Quadrant 1/4 switch (from Q1) @middle") << 20.0 << 10.0 << 350.0 << 340.0 << ::Middle << 0.0;
    QTest::newRow("Quadrant 1/4 switch (from Q1) @P1") << 20.0 << 10.0 << 350.0 << 340.0 << ::P1 << 10.0;
    QTest::newRow("Quadrant 1/4 switch (from Q1) @P2") << 20.0 << 10.0 << 350.0 << 340.0 << ::P2 << 350.0;

    QTest::newRow("Quadrant 4/1 switch (from Q4) @middle") << 340.0 << 350.0 << 10.0 << 20.0 << ::Middle << 0.0;
    QTest::newRow("Quadrant 4/1 switch (from Q4) @P1") << 340.0 << 350.0 << 10.0 << 20.0 << ::P1 << 350.0;
    QTest::newRow("Quadrant 4/1 switch (from Q4) @P2") << 340.0 << 350.0 << 10.0 << 20.0 << ::P2 << 10.0;

    // Quadrant 2/3 switch (crossing 180 degrees)
    QTest::newRow("Quadrant 2/3 switch (from Q2) @middle") << 160.0 << 170.0 << 190.0 << 200.0 << ::Middle << 180.0;
    QTest::newRow("Quadrant 2/3 switch (from Q2) @P1") << 160.0 << 170.0 << 190.0 << 209.0 << ::P1 << 170.0;
    QTest::newRow("Quadrant 2/3 switch (from Q2) @P2") << 160.0 << 170.0 << 190.0 << 209.0 << ::P2 << 190.0;

    QTest::newRow("Quadrant 3/2 switch (from Q3) @middle") << 200.0 << 190.0 << 170.0 << 160.0 << ::Middle << 180.0;
    QTest::newRow("Quadrant 3/2 switch (from Q3) @P1") << 200.0 << 190.0 << 170.0 << 160.0 << ::P1 << 190.0;
    QTest::newRow("Quadrant 3/2 switch (from Q3) @P2") << 200.0 << 190.0 << 170.0 << 160.0 << ::P2 << 170.0;

    // From Q1 to...
    QTest::newRow("Right turn < 90 degrees from Q1 to Q2 @middle") << 35.0 << 45.0 << 125.0 << 135.0 << ::Middle << 85.0;
    QTest::newRow("Right turn < 90 degrees from Q1 to Q2 @P1") << 35.0 << 45.0 << 125.0 << 135.0 << ::P1 << 45.0;
    QTest::newRow("Right turn < 90 degrees from Q1 to Q2 @P2") << 35.0 << 45.0 << 125.0 << 135.0 << ::P2 << 125.0;

    QTest::newRow("Right turn 90 degrees from Q1 to Q2 @middle") << 35.0 << 45.0 << 135.0 << 145.0 << ::Middle << 90.0;
    QTest::newRow("Right turn 90 degrees from Q1 to Q2 @P1") << 35.0 << 45.0 << 135.0 << 145.0 << ::P1 << 45.0;
    QTest::newRow("Right turn 90 degrees from Q1 to Q2 @P2") << 35.0 << 45.0 << 135.0 << 145.0 << ::P2 << 135.0;

    QTest::newRow("Right turn > 90 degrees from Q1 to Q2 @middle") << 35.0 << 45.0 << 145.0 << 155.0 << ::Middle << 95.0;
    QTest::newRow("Right turn > 90 degrees from Q1 to Q2 @P1") << 35.0 << 45.0 << 145.0 << 155.0 << ::P1 << 45.0;
    QTest::newRow("Right turn > 90 degrees from Q1 to Q2 @P2") << 35.0 << 45.0 << 145.0 << 155.0 << ::P2 << 145.0;

    QTest::newRow("Right turn < 180 degrees from Q1 to Q3 @middle") << 35.0 << 45.0 << 215.0 << 225.0 << ::Middle << 130.0;
    QTest::newRow("Right turn < 180 degrees from Q1 to Q3 @P1") << 35.0 << 45.0 << 215.0 << 225.0 << ::P1 << 45.0;
    QTest::newRow("Right turn < 180 degrees from Q1 to Q3 @P2") << 35.0 << 45.0 << 215.0 << 225.0 << ::P2 << 215.0;

    QTest::newRow("Right turn 180 degrees from Q1 to Q3 @middle") << 35.0 << 45.0 << 225.0 << 235.0 << ::Middle << 135.0;
    QTest::newRow("Right turn 180 degrees from Q1 to Q3 @P1") << 35.0 << 45.0 << 225.0 << 235.0 << ::P1 << 45.0;
    QTest::newRow("Right turn 180 degrees from Q1 to Q3 @P2") << 35.0 << 45.0 << 225.0 << 235.0 << ::P2 << 225.0;

    QTest::newRow("Left turn > 180 degrees from Q1 to Q3 @middle") << 35.0 << 45.0 << 235.0 << 245.0 << ::Middle << 320.0;
    QTest::newRow("Left turn > 180 degrees from Q1 to Q3 @P1") << 35.0 << 45.0 << 235.0 << 245.0 << ::P1 << 45.0;
    QTest::newRow("Left turn > 180 degrees from Q1 to Q3 @P2") << 35.0 << 45.0 << 235.0 << 245.0 << ::P2 << 235.0;

    QTest::newRow("Left turn < 90 degrees from Q1 to Q4 @middle") << 35.0 << 45.0 << 325.0 << 335.0 << ::Middle << 5.0;
    QTest::newRow("Left turn < 90 degrees from Q1 to Q4 @P1") << 35.0 << 45.0 << 325.0 << 335.0 << ::P1 << 45.0;
    QTest::newRow("Left turn < 90 degrees from Q1 to Q4 @P2") << 35.0 << 45.0 << 325.0 << 335.0 << ::P2 << 325.0;

    QTest::newRow("Left turn 90 degrees from Q1 to Q4 @middle") << 35.0 << 45.0 << 315.0 << 325.0 << ::Middle << 0.0;
    QTest::newRow("Left turn 90 degrees from Q1 to Q4 @P1") << 35.0 << 45.0 << 315.0 << 325.0 << ::P1 << 45.0;
    QTest::newRow("Left turn 90 degrees from Q1 to Q4 @P2") << 35.0 << 45.0 << 315.0 << 325.0 << ::P2 << 315.0;

    QTest::newRow("Left turn > 90 degrees from Q1 to Q4 @middle") << 35.0 << 45.0 << 305.0 << 315.0 << ::Middle << 355.0;
    QTest::newRow("Left turn > 90 degrees from Q1 to Q4 @P1") << 35.0 << 45.0 << 305.0 << 315.0 << ::P1 << 45.0;
    QTest::newRow("Left turn > 90 degrees from Q1 to Q4 @P2") << 35.0 << 45.0 << 305.0 << 315.0 << ::P2 << 305.0;

    // From Q2 to...
    QTest::newRow("Right turn < 90 degrees from Q2 to Q3 @middle") << 125.0 << 135.0 << 215.0 << 225.0 << ::Middle << 175.0;
    QTest::newRow("Right turn < 90 degrees from Q2 to Q3 @P1") << 125.0 << 135.0 << 215.0 << 225.0 << ::P1 << 135.0;
    QTest::newRow("Right turn < 90 degrees from Q2 to Q3 @P2") << 125.0 << 135.0 << 215.0 << 225.0 << ::P2 << 215.0;

    QTest::newRow("Right turn 90 degrees from Q2 to Q3 @middle") << 125.0 << 135.0 << 225.0 << 235.0 << ::Middle << 180.0;
    QTest::newRow("Right turn 90 degrees from Q2 to Q3 @P1") << 125.0 << 135.0 << 225.0 << 235.0 << ::P1 << 135.0;
    QTest::newRow("Right turn 90 degrees from Q2 to Q3 @P2") << 125.0 << 135.0 << 225.0 << 235.0 << ::P2 << 225.0;

    QTest::newRow("Right turn > 90 degrees from Q2 to Q3 @middle") << 125.0 << 135.0 << 235.0 << 245.0 << ::Middle << 185.0;
    QTest::newRow("Right turn > 90 degrees from Q2 to Q3 @P1") << 125.0 << 135.0 << 235.0 << 245.0 << ::P1 << 135.0;
    QTest::newRow("Right turn > 90 degrees from Q2 to Q3 @P2") << 125.0 << 135.0 << 235.0 << 245.0 << ::P2 << 235.0;

    QTest::newRow("Right turn < 180 degrees from Q2 to Q4 @middle") << 125.0 << 135.0 << 305.0 << 315.0 << ::Middle << 220.0;
    QTest::newRow("Right turn < 180 degrees from Q2 to Q4 @P1") << 125.0 << 135.0 << 305.0 << 315.0 << ::P1 << 135.0;
    QTest::newRow("Right turn < 180 degrees from Q2 to Q4 @P2") << 125.0 << 135.0 << 305.0 << 315.0 << ::P2 << 305.0;

    QTest::newRow("Right turn 180 degrees from Q2 to Q4 @middle") << 125.0 << 135.0 << 315.0 << 325.0 << ::Middle << 225.0;
    QTest::newRow("Right turn 180 degrees from Q2 to Q4 @P1") << 125.0 << 135.0 << 315.0 << 325.0 << ::P1 << 135.0;
    QTest::newRow("Right turn 180 degrees from Q2 to Q4 @P2") << 125.0 << 135.0 << 315.0 << 325.0 << ::P2 << 315.0;

    QTest::newRow("Left turn > 180 degrees from Q2 to Q4 @middle") << 125.0 << 135.0 << 305.0 << 315.0 << ::Middle << 220.0;
    QTest::newRow("Left turn > 180 degrees from Q2 to Q4 @P1") << 125.0 << 135.0 << 305.0 << 315.0 << ::P1 << 135.0;
    QTest::newRow("Left turn > 180 degrees from Q2 to Q4 @P2") << 125.0 << 135.0 << 305.0 << 315.0 << ::P2 << 305.0;

    QTest::newRow("Left turn < 90 degrees from Q2 to Q1 @middle") << 125.0 << 135.0 << 55.0 << 65.0 << ::Middle << 95.0;
    QTest::newRow("Left turn < 90 degrees from Q2 to Q1 @P1") << 125.0 << 135.0 << 55.0 << 65.0 << ::P1 << 135.0;
    QTest::newRow("Left turn < 90 degrees from Q2 to Q1 @P2") << 125.0 << 135.0 << 55.0 << 65.0 << ::P2 << 55.0;

    QTest::newRow("Left turn 90 degrees from Q2 to Q1 @middle") << 125.0 << 135.0 << 45.0 << 55.0 << ::Middle << 90.0;
    QTest::newRow("Left turn 90 degrees from Q2 to Q1 @P1") << 125.0 << 135.0 << 45.0 << 55.0 << ::P1 << 135.0;
    QTest::newRow("Left turn 90 degrees from Q2 to Q1 @P2") << 125.0 << 135.0 << 45.0 << 55.0 << ::P2 << 45.0;

    QTest::newRow("Left turn > 90 degrees from Q2 to Q1 @middle") << 125.0 << 135.0 << 35.0 << 45.0 << ::Middle << 85.0;
    QTest::newRow("Left turn > 90 degrees from Q2 to Q1 @P1") << 125.0 << 135.0 << 35.0 << 45.0 << ::P1 << 135.0;
    QTest::newRow("Left turn > 90 degrees from Q2 to Q1 @P2") << 125.0 << 135.0 << 35.0 << 45.0 << ::P2 << 35.0;

    // From Q3 to...
    QTest::newRow("Right turn < 90 degrees from Q3 to Q4 @middle") << 215.0 << 225.0 << 305.0 << 315.0 << ::Middle << 265.0;
    QTest::newRow("Right turn < 90 degrees from Q3 to Q4 @P1") << 215.0 << 225.0 << 305.0 << 315.0 << ::P1 << 225.0;
    QTest::newRow("Right turn < 90 degrees from Q3 to Q4 @P2") << 215.0 << 225.0 << 305.0 << 315.0 << ::P2 << 305.0;

    QTest::newRow("Right turn 90 degrees from Q3 to Q4 @middle") << 215.0 << 225.0 << 315.0 << 325.0 << ::Middle << 270.0;
    QTest::newRow("Right turn 90 degrees from Q3 to Q4 @P1") << 215.0 << 225.0 << 315.0 << 325.0 << ::P1 << 225.0;
    QTest::newRow("Right turn 90 degrees from Q3 to Q4 @P2") << 215.0 << 225.0 << 315.0 << 325.0 << ::P2 << 315.0;

    QTest::newRow("Right turn > 90 degrees from Q3 to Q4 @middle") << 215.0 << 225.0 << 325.0 << 335.0 << ::Middle << 275.0;
    QTest::newRow("Right turn > 90 degrees from Q3 to Q4 @P1") << 215.0 << 225.0 << 325.0 << 335.0 << ::P1 << 225.0;
    QTest::newRow("Right turn > 90 degrees from Q3 to Q4 @P2") << 215.0 << 225.0 << 325.0 << 335.0 << ::P2 << 325.0;

    QTest::newRow("Right turn < 180 degrees from Q3 to Q1 @middle") << 215.0 << 225.0 << 35.0 << 45.0 << ::Middle << 310.0;
    QTest::newRow("Right turn < 180 degrees from Q3 to Q1 @P1") << 215.0 << 225.0 << 35.0 << 45.0 << ::P1 << 225.0;
    QTest::newRow("Right turn < 180 degrees from Q3 to Q1 @P2") << 215.0 << 225.0 << 35.0 << 45.0 << ::P2 << 35.0;

    QTest::newRow("Left turn 180 degrees from Q3 to Q1 @middle") << 215.0 << 225.0 << 45.0 << 55.0 << ::Middle << 135.0;
    QTest::newRow("Left turn 180 degrees from Q3 to Q1 @P1") << 215.0 << 225.0 << 45.0 << 55.0 << ::P1 << 225.0;
    QTest::newRow("Left turn 180 degrees from Q3 to Q1 @P2") << 215.0 << 225.0 << 45.0 << 55.0 << ::P2 << 45.0;

    QTest::newRow("Left turn > 180 degrees from Q3 to Q1 @middle") << 215.0 << 225.0 << 55.0 << 65.0 << ::Middle << 140.0;
    QTest::newRow("Left turn > 180 degrees from Q3 to Q1 @P1") << 215.0 << 225.0 << 55.0 << 65.0 << ::P1 << 225.0;
    QTest::newRow("Left turn > 180 degrees from Q3 to Q1 @P2") << 215.0 << 225.0 << 55.0 << 65.0 << ::P2 << 55.0;

    QTest::newRow("Left turn < 90 degrees from Q3 to Q2 @middle") << 215.0 << 225.0 << 145.0 << 155.0 << ::Middle << 185.0;
    QTest::newRow("Left turn < 90 degrees from Q3 to Q2 @P1") << 215.0 << 225.0 << 145.0 << 155.0 << ::P1 << 225.0;
    QTest::newRow("Left turn < 90 degrees from Q3 to Q2 @P2") << 215.0 << 225.0 << 145.0 << 155.0 << ::P2 << 145.0;

    QTest::newRow("Left turn 90 degrees from Q3 to Q2 @middle") << 215.0 << 225.0 << 135.0 << 145.0 << ::Middle << 180.0;
    QTest::newRow("Left turn 90 degrees from Q3 to Q2 @P1") << 215.0 << 225.0 << 135.0 << 145.0 << ::P1 << 225.0;
    QTest::newRow("Left turn 90 degrees from Q3 to Q2 @P2") << 215.0 << 225.0 << 135.0 << 145.0 << ::P2 << 135.0;

    QTest::newRow("Left turn > 90 degrees from Q3 to Q2 @middle") << 215.0 << 225.0 << 125.0 << 135.0 << ::Middle << 175.0;
    QTest::newRow("Left turn > 90 degrees from Q3 to Q2 @P1") << 215.0 << 225.0 << 125.0 << 135.0 << ::P1 << 225.0;
    QTest::newRow("Left turn > 90 degrees from Q3 to Q2 @P2") << 215.0 << 225.0 << 125.0 << 135.0 << ::P2 << 125.0;

    // From Q4 to...
    QTest::newRow("Right turn < 90 degrees from Q4 to Q1 @middle") << 305.0 << 315.0 << 35.0 << 45.0 << ::Middle << 355.0;
    QTest::newRow("Right turn < 90 degrees from Q4 to Q1 @P1") << 305.0 << 315.0 << 35.0 << 45.0 << ::P1 << 315.0;
    QTest::newRow("Right turn < 90 degrees from Q4 to Q1 @P2") << 305.0 << 315.0 << 35.0 << 45.0 << ::P2 << 35.0;

    QTest::newRow("Right turn 90 degrees from Q4 to Q1 @middle") << 305.0 << 315.0 << 45.0 << 55.0 << ::Middle << 0.0;
    QTest::newRow("Right turn 90 degrees from Q4 to Q1 @P1") << 305.0 << 315.0 << 45.0 << 55.0 << ::P1 << 315.0;
    QTest::newRow("Right turn 90 degrees from Q4 to Q1 @P2") << 305.0 << 315.0 << 45.0 << 55.0 << ::P2 << 45.0;

    QTest::newRow("Right turn > 90 degrees from Q4 to Q1 @middle") << 305.0 << 315.0 << 55.0 << 65.0 << ::Middle << 5.0;
    QTest::newRow("Right turn > 90 degrees from Q4 to Q1 @P1") << 305.0 << 315.0 << 55.0 << 65.0 << ::P1 << 315.0;
    QTest::newRow("Right turn > 90 degrees from Q4 to Q1 @P2") << 305.0 << 315.0 << 55.0 << 65.0 << ::P2 << 55.0;

    QTest::newRow("Right turn < 180 degrees from Q4 to Q2 @middle") << 305.0 << 315.0 << 125.0 << 135.0 << ::Middle << 40.0;
    QTest::newRow("Right turn < 180 degrees from Q4 to Q2 @P1") << 305.0 << 315.0 << 125.0 << 135.0 << ::P1 << 315.0;
    QTest::newRow("Right turn < 180 degrees from Q4 to Q2 @P2") << 305.0 << 315.0 << 125.0 << 135.0 << ::P2 << 125.0;

    QTest::newRow("Left turn 180 degrees from Q4 to Q2 @middle") << 305.0 << 315.0 << 135.0 << 145.0 << ::Middle << 225.0;
    QTest::newRow("Left turn 180 degrees from Q4 to Q2 @P1") << 305.0 << 315.0 << 135.0 << 145.0 << ::P1 << 315.0;
    QTest::newRow("Left turn 180 degrees from Q4 to Q2 @P2") << 305.0 << 315.0 << 135.0 << 145.0 << ::P2 << 135.0;

    QTest::newRow("Left turn > 180 degrees from Q4 to Q2 @middle") << 305.0 << 315.0 << 145.0 << 155.0 << ::Middle << 230.0;
    QTest::newRow("Left turn > 180 degrees from Q4 to Q2 @P1") << 305.0 << 315.0 << 145.0 << 155.0 << ::P1 << 315.0;
    QTest::newRow("Left turn > 180 degrees from Q4 to Q2 @P2") << 305.0 << 315.0 << 145.0 << 155.0 << ::P2 << 145.0;

    QTest::newRow("Left turn < 90 degrees from Q4 to Q3 @middle") << 305.0 << 315.0 << 235.0 << 245.0 << ::Middle << 275.0;
    QTest::newRow("Left turn < 90 degrees from Q4 to Q3 @P1") << 305.0 << 315.0 << 235.0 << 245.0 << ::P1 << 315.0;
    QTest::newRow("Left turn < 90 degrees from Q4 to Q3 @P2") << 305.0 << 315.0 << 235.0 << 245.0 << ::P2 << 235.0;

    QTest::newRow("Left turn 90 degrees from Q4 to Q3 @middle") << 305.0 << 315.0 << 225.0 << 235.0 << ::Middle << 270.0;
    QTest::newRow("Left turn 90 degrees from Q4 to Q3 @P1") << 305.0 << 315.0 << 225.0 << 235.0 << ::P1 << 315.0;
    QTest::newRow("Left turn 90 degrees from Q4 to Q3 @P2") << 305.0 << 315.0 << 225.0 << 235.0 << ::P2 << 225.0;

    QTest::newRow("Left turn > 90 degrees from Q4 to Q3 @middle") << 305.0 << 315.0 << 215.0 << 225.0 << ::Middle << 265.0;
    QTest::newRow("Left turn > 90 degrees from Q4 to Q3 @P1") << 305.0 << 315.0 << 215.0 << 225.0 << ::P1 << 315.0;
    QTest::newRow("Left turn > 90 degrees from Q4 to Q3 @P2") << 305.0 << 315.0 << 215.0 << 225.0 << ::P2 << 215.0;
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

    // Exercise
    double result = SkyMath::interpolateHermite360(p0, p1, p2, p3, mu);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::interpolateNearestNeighbour_data()
{
    QTest::addColumn<double>("p0");
    QTest::addColumn<double>("p1");
    QTest::addColumn<double>("mu");
    QTest::addColumn<double>("expected");

    QTest::newRow("First") << 1.0 << 2.0 << 0.1 << 1.0;
    QTest::newRow("Second") << 1.0 << 2.0 << 0.5 << 2.0;
    QTest::newRow("Negative time") << 1.0 << 2.0 << -1.0 << 1.0;
    QTest::newRow("Exceeding time") << 1.0 << 2.0 << 2.0 << 2.0;
    QTest::newRow("First negative") << -1.0 << 2.0 << 0.1 << -1.0;
    QTest::newRow("Second negative") << 1.0 << -2.0 << 0.5 << -2.0;
    QTest::newRow("First boundary") << 1.0 << 2.0 << 0.0 << 1.0;
    QTest::newRow("Second boundary") << 1.0 << 2.0 << 1.0 << 2.0;
}

void SkyMathTest::interpolateNearestNeighbour()
{
    // Setup
    QFETCH(double, p0);
    QFETCH(double, p1);
    QFETCH(double, mu);
    QFETCH(double, expected);

    // Exercise
    double result = SkyMath::interpolateNearestNeighbour(p0, p1, mu);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::fromPosition_data()
{
    QTest::addColumn<double>("p");
    QTest::addColumn<std::int16_t>("expected");

    QTest::newRow("Minimum") << -1.0 << static_cast<std::int16_t>(SkyMath::PositionMin16);
    QTest::newRow("Maximum") <<  1.0 << static_cast<std::int16_t>(SkyMath::PositionMax16);
    QTest::newRow("Zero") << 0.0 << static_cast<std::int16_t>(0);
    QTest::newRow("Negative value") << -0.5 << static_cast<std::int16_t>(-16384);
    QTest::newRow("Positive value") <<  0.5 << static_cast<std::int16_t>( 16384);
}

void SkyMathTest::fromPosition()
{
    // Setup
    QFETCH(double, p);
    QFETCH(std::int16_t, expected);

    // Exercise
    std::int16_t result = SkyMath::fromNormalisedPosition(p);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::toPosition_data()
{
    QTest::addColumn<std::int16_t>("p16");
    QTest::addColumn<double>("expected");

    QTest::newRow("Minimum") << static_cast<std::int16_t>(SkyMath::PositionMin16) << -1.0;
    QTest::newRow("Maximum") << static_cast<std::int16_t>(SkyMath::PositionMax16) <<  1.0;
    QTest::newRow("Zero") << static_cast<std::int16_t>(0) << 0.0;
}

void SkyMathTest::toPosition()
{
    // Setup
    QFETCH(std::int16_t, p16);
    QFETCH(double, expected);

    // Exercise
    double result = SkyMath::toNormalisedPosition(p16);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::fromPercent_data()
{
    QTest::addColumn<double>("p");
    QTest::addColumn<std::uint8_t>("expected");

    QTest::newRow("Minimum") <<   0.0 << static_cast<std::uint8_t>(SkyMath::PercentMin8);
    QTest::newRow("Maximum") << 100.0 << static_cast<std::uint8_t>(SkyMath::PercentMax8);
    QTest::newRow("Half") << 50.0 << static_cast<std::uint8_t>(128);
}

void SkyMathTest::fromPercent()
{
    // Setup
    QFETCH(double, p);
    QFETCH(std::uint8_t, expected);

    // Exercise
    std::uint8_t result = SkyMath::fromPercent(p);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::toPercent_data()
{
    QTest::addColumn<std::uint8_t>("p8");
    QTest::addColumn<double>("expected");

    QTest::newRow("Minimum") << static_cast<std::uint8_t>(SkyMath::PercentMin8) << 0.0;
    QTest::newRow("Maximum") << static_cast<std::uint8_t>(SkyMath::PercentMax8) << 100.0;
}

void SkyMathTest::toPercent()
{
    // Setup
    QFETCH(std::uint8_t, p8);
    QFETCH(double, expected);

    // Exercise
    double result = SkyMath::toPercent(p8);

    // Verify
    QCOMPARE(result, expected);
}

void SkyMathTest::relativePosition_data()
{
    QTest::addColumn<double>("latitude");
    QTest::addColumn<double>("longitude");
    QTest::addColumn<double>("bearing");
    QTest::addColumn<double>("distance");
    QTest::addColumn<double>("expectedLatitude");
    QTest::addColumn<double>("expectedLongitude");

    // DMS to degrees: https://boulter.com/gps/
    // https://www.movable-type.co.uk/scripts/latlong.html
    QTest::newRow("Northern Hemisphere") << 47.0 << 8.0 << 90.0 << 100000.0 << 46.9925 << 9.3147;
    QTest::newRow("Southern Hemisphere") << -47.0 << -8.0 << -90.0 << 100000.0 << -46.9925 << -9.3147;
    QTest::newRow("Northpole") << 90.0 << 0.0 << 0.0 <<100000.0 << 89.1047 << 180.0;
    QTest::newRow("Southpole") << -90.0 << 0.0 << 0.0 << 100000.0 << -89.1047 << 0.0;
    QTest::newRow("Same point") << -47.0 << -8.0 << -90.0 << 0.0 << -47.0 << -8.0;
}

void SkyMathTest::relativePosition()
{
    constexpr double PrecisionFactor = 10000;

    // Setup
    QFETCH(double, latitude);
    QFETCH(double, longitude);
    QFETCH(double, bearing);
    QFETCH(double, distance);
    QFETCH(double, expectedLatitude);
    QFETCH(double, expectedLongitude);

    SkyMath::Coordinate position(latitude, longitude);
    SkyMath::Coordinate expectedDestination(expectedLatitude, expectedLongitude);

    // Exercise
    SkyMath::Coordinate destination = SkyMath::relativePosition(position, bearing, distance);

    // Verify
    const double lat = std::round(destination.first * PrecisionFactor) / PrecisionFactor;
    const double lon = std::round(destination.second * PrecisionFactor) / PrecisionFactor;
    QCOMPARE(lat, expectedDestination.first);
    QCOMPARE(lon, expectedDestination.second);
}

void SkyMathTest::headingChange_data()
{
    QTest::addColumn<double>("currentHeading");
    QTest::addColumn<double>("targetHeading");
    QTest::addColumn<double>("expectedHeadingChange");

    // Selected examples
    QTest::newRow("Left turn by 170") << 270.0 << 100.0 << +170.0;
    QTest::newRow("Left turn across north") << 5.0 << 355.0 << +10.0;
    QTest::newRow("Right turn by 5") << 5.0 << 10.0 << -5.0;
    QTest::newRow("Right turn by 100") << 270.0 << 10.0 << -100.0;

    // 180 degree change
    QTest::newRow("Right turn from north to south") << 0.0 << 180.0 << -180.0;
    QTest::newRow("Right turn from south to north") << 180.0 << 0.0 << +180.0;
    QTest::newRow("Right turn from  east to west") << 90.0 << 270.0 << -180.0;
    QTest::newRow("Right turn from  west to east") << 270.0 << 90.0 << +180.0;

    // From Q1 to...
    QTest::newRow("Right turn from Q1 to Q2 #1") << 45.0 << 130.0 << -85.0;
    QTest::newRow("Right turn from Q1 to Q2 #2") << 45.0 << 135.0 << -90.0;
    QTest::newRow("Right turn from Q1 to Q2 #3") << 45.0 << 140.0 << -95.0;
    QTest::newRow("Right turn from Q1 to Q3 #1") << 45.0 << 220.0 << -175.0;
    QTest::newRow("Right turn from Q1 to Q3 #2") << 45.0 << 225.0 << -180.0;
    QTest::newRow("Left turn from Q1 to Q3 #3")  << 45.0 << 230.0 << +175.0;
    QTest::newRow("Left turn from Q1 to Q4 #1") << 45.0 << 320.0 << +85.0;
    QTest::newRow("Left turn from Q1 to Q4 #2") << 45.0 << 315.0 << +90.0;
    QTest::newRow("Left turn from Q1 to Q4 #3") << 45.0 << 310.0 << +95.0;

    // From Q2 to...
    QTest::newRow("Right turn from Q2 to Q3 #1") << 135.0 << 220.0 << -85.0;
    QTest::newRow("Right turn from Q2 to Q3 #2") << 135.0 << 225.0 << -90.0;
    QTest::newRow("Right turn from Q2 to Q3 #3") << 135.0 << 230.0 << -95.0;
    QTest::newRow("Right turn from Q2 to Q4 #1") << 135.0 << 310.0 << -175.0;
    QTest::newRow("Right turn from Q2 to Q4 #2") << 135.0 << 315.0 << -180.0;
    QTest::newRow("Left turn from Q2 to Q4 #3") << 135.0 << 320.0 << +175.0;
    QTest::newRow("Left turn from Q2 to Q1 #1") << 135.0 << 50.0 << +85.0;
    QTest::newRow("Left turn from Q2 to Q1 #2") << 135.0 << 45.0 << +90.0;
    QTest::newRow("Left turn from Q2 to Q1 #3") << 135.0 << 40.0 << +95.0;

    // From Q3 to...
    QTest::newRow("Right turn from Q3 to Q4 #1") << 225.0 << 310.0 << -85.0;
    QTest::newRow("Right turn from Q3 to Q4 #2") << 225.0 << 315.0 << -90.0;
    QTest::newRow("Right turn from Q3 to Q4 #3") << 225.0 << 320.0 << -95.0;
    QTest::newRow("Right turn from Q3 to Q1 #1") << 225.0 << 40.0 << -175.0;
    QTest::newRow("Left turn from Q3 to Q1 #2") << 225.0 << 45.0 << +180.0;
    QTest::newRow("Left turn from Q3 to Q1 #3") << 225.0 << 50.0 << +175.0;
    QTest::newRow("Left turn from Q3 to Q2 #1") << 225.0 << 140.0 << +85.0;
    QTest::newRow("Left turn from Q3 to Q2 #2") << 225.0 << 135.0 << +90.0;
    QTest::newRow("Left turn from Q3 to Q2 #3") << 225.0 << 130.0 << +95.0;

    // From Q4 to...
    QTest::newRow("Right turn from Q4 to Q1 #1") << 315.0 << 40.0 << -85.0;
    QTest::newRow("Right turn from Q4 to Q1 #2") << 315.0 << 45.0 << -90.0;
    QTest::newRow("Right turn from Q4 to Q1 #3") << 315.0 << 50.0 << -95.0;
    QTest::newRow("Right turn from Q4 to Q2 #1") << 315.0 << 130.0 << -175.0;
    QTest::newRow("Left turn from Q4 to Q2 #2") << 315.0 << 135.0 << +180.0;
    QTest::newRow("Left turn from Q4 to Q2 #3") << 315.0 << 140.0 << +175.0;
    QTest::newRow("Left turn from Q4 to Q3 #1") << 315.0 << 230.0 << +85.0;
    QTest::newRow("Left turn from Q4 to Q3 #2") << 315.0 << 225.0 << +90.0;
    QTest::newRow("Left turn from Q4 to Q3 #3") << 315.0 << 220.0 << +95.0;

    // No turn
    QTest::newRow("No turn in Q1") << 45.0 << 45.0 << 0.0;
    QTest::newRow("No turn in Q2") << 135.0 << 135.0 << 0.0;
    QTest::newRow("No turn in Q3") << 225.0 << 225.0 << 0.0;
    QTest::newRow("No turn in Q4") << 315.0 << 315.0 << 0.0;
}

void SkyMathTest::headingChange()
{
    // Setup
    QFETCH(double, currentHeading);
    QFETCH(double, targetHeading);
    QFETCH(double, expectedHeadingChange);

    // Exercise
    const double headingChange = SkyMath::headingChange(currentHeading, targetHeading);

    // Verify
    QCOMPARE(headingChange, expectedHeadingChange);
}

void SkyMathTest::bankAngle_data()
{
    QTest::addColumn<double>("headingChange");
    QTest::addColumn<double>("maxBankAngleForHeadingChange");
    QTest::addColumn<double>("maxBankAngle");
    QTest::addColumn<double>("expectedBankAngle");

    // Left
    QTest::newRow("Left turn by 10 degrees (max bank angle: 40@20)") << 10.0 << 20.0 << 40.0 << 20.0;
    QTest::newRow("Left turn by 10 degrees (max bank angle: 40@10)") << 10.0 << 10.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 10 degrees (max bank angle: 40@5)") << 10.0 << 5.0 << 40.0 << 40.0;

    QTest::newRow("Left turn by 45 degrees (max bank angle: 40@90)") << 45.0 << 90.0 << 40.0 << 20.0;
    QTest::newRow("Left turn by 45 degrees (max bank angle: 40@45)") << 45.0 << 45.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 45 degrees (max bank angle: 40@5)") << 45.0 << 5.0 << 40.0 << 40.0;

    QTest::newRow("Left turn by 90 degrees (max bank angle: 40@180)") << 90.0 << 180.0 << 40.0 << 20.0;
    QTest::newRow("Left turn by 90 degrees (max bank angle: 40@90)") << 90.0 << 90.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 90 degrees (max bank angle: 40@5)") << 90.0 << 5.0 << 40.0 << 40.0;

    QTest::newRow("Left turn by 135 degrees (max bank angle: 40@180)") << 135.0 << 180.0 << 40.0 << 30.0;
    QTest::newRow("Left turn by 135 degrees (max bank angle: 40@90)") << 135.0 << 90.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 135 degrees (max bank angle: 40@5)") << 135.0 << 5.0 << 40.0 << 40.0;

    QTest::newRow("Left turn by 180 degrees (max bank angle: 40@180)") << 180.0 << 180.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 180 degrees (max bank angle: 40@90)") << 180.0 << 90.0 << 40.0 << 40.0;
    QTest::newRow("Left turn by 180 degrees (max bank angle: 40@5)") << 180.0 << 5.0 << 40.0 << 40.0;

    // Right
    QTest::newRow("Right turn by 10 degrees (max bank angle: 40@20)") << -10.0 << 20.0 << 40.0 << -20.0;
    QTest::newRow("Right turn by 10 degrees (max bank angle: 40@10)") << -10.0 << 10.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 10 degrees (max bank angle: 40@5)") << -10.0 << 5.0 << 40.0 << -40.0;

    QTest::newRow("Right turn by 45 degrees (max bank angle: 40@90)") << -45.0 << 90.0 << 40.0 << -20.0;
    QTest::newRow("Right turn by 45 degrees (max bank angle: 40@45)") << -45.0 << 45.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 45 degrees (max bank angle: 40@5)") << -45.0 << 5.0 << 40.0 << -40.0;

    QTest::newRow("Right turn by 90 degrees (max bank angle: 40@180)") << -90.0 << 180.0 << 40.0 << -20.0;
    QTest::newRow("Right turn by 90 degrees (max bank angle: 40@90)") << -90.0 << 90.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 90 degrees (max bank angle: 40@5)") << -90.0 << 5.0 << 40.0 << -40.0;

    QTest::newRow("Right turn by 135 degrees (max bank angle: 40@180)") << -135.0 << 180.0 << 40.0 << -30.0;
    QTest::newRow("Right turn by 135 degrees (max bank angle: 40@90)") << -135.0 << 90.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 135 degrees (max bank angle: 40@5)") << -135.0 << 5.0 << 40.0 << -40.0;

    QTest::newRow("Right turn by 180 degrees (max bank angle: 40@180)") << -180.0 << 180.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 180 degrees (max bank angle: 40@90)") << -180.0 << 90.0 << 40.0 << -40.0;
    QTest::newRow("Right turn by 180 degrees (max bank angle: 40@5)") << -180.0 << 5.0 << 40.0 << -40.0;

    // No turn
    QTest::newRow("No turn (max bank angle: 40@20)") << 0.0 << 20.0 << 40.0 << 0.0;
}

void SkyMathTest::bankAngle()
{
    // Setup
    QFETCH(double, headingChange);
    QFETCH(double, maxBankAngleForHeadingChange);
    QFETCH(double, maxBankAngle);
    QFETCH(double, expectedBankAngle);

    // Exercise
    const double bankAngle = SkyMath::bankAngle(headingChange, maxBankAngleForHeadingChange, maxBankAngle);

    // Verify
    QCOMPARE(bankAngle, expectedBankAngle);
}

void SkyMathTest::calculateTimeOffset_data()
{
    QTest::addColumn<SkyMath::TimeOffsetSync>("timeOffsetSync");
    QTest::addColumn<QDateTime>("fromDateTime");
    QTest::addColumn<QDateTime>("toDateTime");
    QTest::addColumn<std::int64_t>("expectedTimeOffset");

    // @TimeOnly

    // Same day, same timezone

    QDateTime fromDateTime = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    QDateTime toDateTime   = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    std::int64_t expectedTimeOffset = 5000;
    QTest::newRow("Same day, same timezone, from < to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = 0;
    QTest::newRow("Same day, same timezone, from = to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = -5000;
    QTest::newRow("Same day, same timezone, from > to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    // Different day, different timezone, crossing the date line (Wake Island (UTC+12), Midway (UTC-11)

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:05-11:00", Qt::ISODate);
    expectedTimeOffset = 5000;
    QTest::newRow("Different day, different timezone, from < to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = 0;
    QTest::newRow("Different day, different timezone, from = to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:05+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = -5000;
    QTest::newRow("Different day, different timezone, from > to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    // Different day, same timezone

    fromDateTime = QDateTime::fromString("2022-02-16T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    expectedTimeOffset = 5000;
    QTest::newRow("Different day, same timezone, from < to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-16T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = 0;
    QTest::newRow("Different day, same timezone, from = to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-16T09:30:05+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = -5000;
    QTest::newRow("Different day, same timezone, from > to @TimeOnly)") << SkyMath::TimeOffsetSync::TimeOnly << fromDateTime << toDateTime << expectedTimeOffset;

    // @DateAndTime

    // Same day, same timezone

    fromDateTime = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    expectedTimeOffset = 5000;
    QTest::newRow("Same day, same timezone, from < to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = 0;
    QTest::newRow("Same day, same timezone, from = to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = -5000;
    QTest::newRow("Same day, same timezone, from > to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    // Different day, different timezone, crossing the date line (Wake Island (UTC+12), Midway (UTC-11)

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:05-11:00", Qt::ISODate);
    expectedTimeOffset = 5000;
    QTest::newRow("Different day, different timezone, from < to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = 0;
    QTest::newRow("Different day, different timezone, from = to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-17T20:45:05+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = -5000;
    QTest::newRow("Different day, different timezone, from > to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    // Different day, same timezone

    fromDateTime = QDateTime::fromString("2022-02-16T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:05+01:00", Qt::ISODate);
    expectedTimeOffset = 5000 + (24 * 60 * 60 * 1000);
    QTest::newRow("Different day, same timezone, from < to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-16T09:30:00+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = 24 * 60 * 60 * 1000;
    QTest::newRow("Different day, same timezone, from = to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-18T09:30:05+01:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-17T09:30:00+01:00", Qt::ISODate);
    expectedTimeOffset = -5000 - (24 * 60 * 60 * 1000);
    QTest::newRow("Different day, same timezone, from > to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    // Different timezone, crossing the date line (Wake Island (UTC+12), Midway (UTC-11)

    fromDateTime = QDateTime::fromString("2022-02-16T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:05-11:00", Qt::ISODate);
    expectedTimeOffset = 5000 + (24 * 60 * 60 * 1000);
    QTest::newRow("Different day, differemt timezone, from < to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-16T20:45:00+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = 24 * 60 * 60 * 1000;
    QTest::newRow("Different day, differemt timezone, from = to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;

    fromDateTime = QDateTime::fromString("2022-02-18T20:45:05+12:00", Qt::ISODate);
    toDateTime   = QDateTime::fromString("2022-02-16T21:45:00-11:00", Qt::ISODate);
    expectedTimeOffset = -5000 - (24 * 60 * 60 * 1000);
    QTest::newRow("Different day, differemt timezone, from > to @DateAndTime)") << SkyMath::TimeOffsetSync::DateAndTime << fromDateTime << toDateTime << expectedTimeOffset;
}

void SkyMathTest::calculateTimeOffset()
{
    // Setup
    QFETCH(SkyMath::TimeOffsetSync, timeOffsetSync);
    QFETCH(QDateTime, fromDateTime);
    QFETCH(QDateTime, toDateTime);
    QFETCH(std::int64_t, expectedTimeOffset);

    // Exercise
    const auto timeOffset = SkyMath::calculateTimeOffset(timeOffsetSync, fromDateTime, toDateTime);

    // Verify
    QCOMPARE(timeOffset, expectedTimeOffset);
}

void SkyMathTest::calculateFibonacci()
{
    // Exercise
    const auto fibonaccis1 = SkyMath::calculateFibonacci<1>(1);

    // Verify
    QCOMPARE(fibonaccis1.back(), 0);

    // Exercise
    const auto fibonaccis2 = SkyMath::calculateFibonacci<2>(2);

    // Verify
    QCOMPARE(fibonaccis2.back(), 1);

    // Exercise
    const auto fibonaccis3 = SkyMath::calculateFibonacci<3>(3);

    // Verify
    QCOMPARE(fibonaccis3.back(), 1);

    // Exercise
    const auto fibonaccis4 = SkyMath::calculateFibonacci<4>(4);

    // Verify
    QCOMPARE(fibonaccis4.back(), 2);

    // Exercise
    const auto fibonaccis5 = SkyMath::calculateFibonacci<5>(5);

    // Verify
    QCOMPARE(fibonaccis5.back(), 3);

    // Exercise
    const auto fibonaccis12 = SkyMath::calculateFibonacci<12>(12);

    // Verify
    QCOMPARE(fibonaccis12.back(), 89);
}

QTEST_MAIN(SkyMathTest)
