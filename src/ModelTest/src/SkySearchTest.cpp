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
#include <QtTest/QtTest>
#include <QVector>

#include "../../Model/src/PositionData.h"
#include "../../Model/src/SkySearch.h"
#include "SkySearchTest.h"

namespace
{
    constexpr int t0 = 0;
    constexpr int t1 = 10;
    constexpr int t2 = 20;
    constexpr int t3 = 30;
}

// PRIVATE SLOTS

void SkySearchTest::initTestCase()
{
    PositionData data1;
    data1.timestamp = t0;
    PositionData data2;
    data2.timestamp = t1;
    PositionData data3;
    data3.timestamp = t2;
    PositionData data4;
    data4.timestamp = t3;

    m_positionData.push_back(data1);
    m_positionData.push_back(data2);
    m_positionData.push_back(data3);
    m_positionData.push_back(data4);
}

void SkySearchTest::cleanupTestCase()
{
}

void SkySearchTest::binaryIntervalSearch_data()
{
    // Note: somehow QTest does not like the type qint64
    QTest::addColumn<int>("timestamp");
    QTest::addColumn<int>("low");
    QTest::addColumn<int>("high");
    QTest::addColumn<int>("expected");

    QTest::newRow("Index for t1") << t0 << 0 << m_positionData.size() - 1 << 0;
    QTest::newRow("Index for t2") << t1 << 0 << m_positionData.size() - 1 << 1;
    QTest::newRow("Index for t3") << t2 << 0 << m_positionData.size() - 1 << 2;
    QTest::newRow("Index for t4") << t3 << 0 << m_positionData.size() - 1 << 3;

    QTest::newRow("Middle")       << (t3 - t0) / 2 << 0 << m_positionData.size() - 1 << 1;
    QTest::newRow("After end")    << t3 + 1        << 0 << m_positionData.size() - 1 << SkySearch::InvalidIndex;
    QTest::newRow("Before start") << t0 - 1        << 0 << m_positionData.size() - 1 << SkySearch::InvalidIndex;

    QTest::newRow("Start interval 1")   << t1 << 0 << 2 << 1;
    QTest::newRow("Start interval 2")   << t3 << m_positionData.size() - 2 << m_positionData.size() -1 << 3;
    QTest::newRow("Outside interval 1") << t1 << 2 << 3 << SkySearch::InvalidIndex;
    QTest::newRow("Outside interval 2") << t3 << 0 << 1 << SkySearch::InvalidIndex;
}

void SkySearchTest::binaryIntervalSearch()
{
    // Setup
    QFETCH(int, timestamp);
    QFETCH(int, low);
    QFETCH(int, high);
    QFETCH(int, expected);

    // Exercise
    int result = SkySearch::binaryIntervalSearch(m_positionData, timestamp, low, high);

    // Verify
    QCOMPARE(result, expected);
}

void SkySearchTest::linearIntervalSearch_data()
{
    // Note: somehow QTest does not like the type qint64
    QTest::addColumn<int>("timestamp");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("expected");

    QTest::newRow("Index for t1") << t0 << 0 << 0;
    QTest::newRow("Index for t2") << t1 << 0 << 1;
    QTest::newRow("Index for t3") << t2 << 0 << 2;
    QTest::newRow("Index for t4") << t3 << 0 << 3;

    QTest::newRow("Middle")       << (t3 - t0) / 2 << 0 << 1;
    QTest::newRow("After end")    << t3 + 1        << 0 << SkySearch::InvalidIndex;
    QTest::newRow("Before start") << t0 - 1        << 0 << SkySearch::InvalidIndex;

    QTest::newRow("Start interval 1")   << t1 << 1 << 1;
    QTest::newRow("Start interval 2")   << t3 << 2 << 3;
    QTest::newRow("Outside interval 1") << t1 << 2 << SkySearch::InvalidIndex;
    QTest::newRow("Start interval 3")   << t3 << 3 << 3;
}

void SkySearchTest::linearIntervalSearch()
{
    // Setup
    QFETCH(int, timestamp);
    QFETCH(int, start);
    QFETCH(int, expected);

    // Exercise
    int result = SkySearch::linearIntervalSearch(m_positionData, timestamp, start);

    // Verify
    QCOMPARE(result, expected);
}

QTEST_MAIN(SkySearchTest)
