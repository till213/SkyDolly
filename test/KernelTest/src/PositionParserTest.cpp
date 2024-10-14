/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <utility>

#include <QTest>
#include <QString>

#include <Kernel/PositionParser.h>
#include "PositionParserTest.h"

using Coordinate = std::pair<double, double>;

// PRIVATE SLOTS

void PositionParserTest::initTestCase()
{}

void PositionParserTest::cleanupTestCase()
{}

void PositionParserTest::parseTest_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<Coordinate>("expectedCoordinate");
    QTest::addColumn<bool>("expectedOk");

    QTest::newRow("Lat/lon w/o comma") << "46.94809 7.44744" << std::pair<double, double>(46.94809, 7.44744) << true;
    QTest::newRow("Lat/lon w/ comma") << "46.94809, 7.44744" << std::pair<double, double>(46.94809, 7.44744) << true;
    QTest::newRow("Neg. lat/lon w/O comma") << "-46.94809 -7.44744" << std::pair<double, double>(-46.94809, -7.44744) << true;
    QTest::newRow("Neg. lat/lon w/ comma") << "-46.94809, -7.44744" << std::pair<double, double>(-46.94809, -7.44744) << true;

    QTest::newRow("DMS w/o comma") << R"(46°56'53.12" N 7°26'50.78 E)" << std::pair<double, double>(46.94809, 7.44744) << true;
    QTest::newRow("DMS w/ comma") << R"(46°56'53.12" N, 7°26'50.78" E)" << std::pair<double, double>(46.94809, 7.44744) << true;
    QTest::newRow("Invert. DMSB w/o comma") << R"(7°26'50.78" E 46°56'53.12" N)" << std::pair<double, double>(46.94809, 7.44744) << true;
    QTest::newRow("Invert. DMSB w/ comma") << R"(7°26'50.78" E, 46°56'53.12" N)" << std::pair<double, double>(46.94809, 7.44744) << true;

    QTest::newRow("Neg. DMS w/o comma") << R"(46°56'53.12" S 7°26'50.78 W)" << std::pair<double, double>(-46.94809, -7.44744) << true;
    QTest::newRow("Neg. DMS w/ comma") << R"(46°56'53.12" S, 7°26'50.78" W)" << std::pair<double, double>(-46.94809, -7.44744) << true;
    QTest::newRow("Neg. invert. DMSB w/o comma") << R"(7°26'50.78" W 46°56'53.12" S)" << std::pair<double, double>(-46.94809, -7.44744) << true;
    QTest::newRow("Neg. invert. DMSB w/ comma") << R"(7°26'50.78" W, 46°56'53.12" S)" << std::pair<double, double>(-46.94809, -7.44744) << true;

    QTest::newRow("No position") << "No position" << std::pair<double, double>(0.0, 0.0) << false;
}

void PositionParserTest::parseTest()
{
    constexpr double Tolerance {0.00001};
    // Setup
    QFETCH(QString, value);
    QFETCH(Coordinate, expectedCoordinate);
    QFETCH(bool, expectedOk);

    // Exercise
    bool ok {false};
    Coordinate result = PositionParser::parse(value, &ok);

    // Verify
    QVERIFY(std::abs(result.first - expectedCoordinate.first) < Tolerance);
    QVERIFY(std::abs(result.second - expectedCoordinate.second) < Tolerance);
    QCOMPARE(ok, expectedOk);
}

QTEST_MAIN(PositionParserTest)
