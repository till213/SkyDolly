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
#include <cstdint>

#include <QtTest>

#include <Model/PositionData.h>
#include <Model/SkySearch.h>
#include "SkySearchTest.h"

namespace
{
    constexpr std::int64_t t0 = 0;
    constexpr std::int64_t t1 = 10;
    constexpr std::int64_t t2 = 20;
    constexpr std::int64_t t3 = 30;
}

// PRIVATE SLOTS

void SkySearchTest::initTestCase()
{
    PositionData data1;
    data1.timestamp = ::t0;
    PositionData data2;
    data2.timestamp = ::t1;
    PositionData data3;
    data3.timestamp = ::t2;
    PositionData data4;
    data4.timestamp = ::t3;

    m_positionData.reserve(4);
    m_positionData.push_back(data1);
    m_positionData.push_back(data2);
    m_positionData.push_back(data3);
    m_positionData.push_back(data4);
}

void SkySearchTest::cleanupTestCase()
{}

void SkySearchTest::binaryIntervalSearch_data()
{
    QTest::addColumn<std::int64_t>("timestamp");
    QTest::addColumn<std::size_t>("low");
    QTest::addColumn<std::size_t>("high");
    QTest::addColumn<int>("expected");

    QTest::newRow("Index for ::t1") << ::t0 << std::size_t(0) << m_positionData.size() - 1 << 0;
    QTest::newRow("Index for ::t2") << ::t1 << std::size_t(0) << m_positionData.size() - 1 << 1;
    QTest::newRow("Index for ::t3") << ::t2 << std::size_t(0) << m_positionData.size() - 1 << 2;
    QTest::newRow("Index for t4") << ::t3 << std::size_t(0) << m_positionData.size() - 1 << 3;

    QTest::newRow("Middle")       << (::t3 - ::t0) / 2 << std::size_t(0) << m_positionData.size() - 1 << 1;
    QTest::newRow("After end")    << ::t3 + 1        << std::size_t(0) << m_positionData.size() - 1 << SkySearch::InvalidIndex;
    QTest::newRow("Before start") << ::t0 - 1        << std::size_t(0) << m_positionData.size() - 1 << SkySearch::InvalidIndex;

    QTest::newRow("Start interval 1")   << ::t1 << std::size_t(0) << std::size_t(2) << 1;
    QTest::newRow("Start interval 2")   << ::t3 << m_positionData.size() - 2 << m_positionData.size() -1 << 3;
    QTest::newRow("Outside interval 1") << ::t1 << std::size_t(2) << std::size_t(3) << SkySearch::InvalidIndex;
    QTest::newRow("Outside interval 2") << ::t3 << std::size_t(0) << std::size_t(1) << SkySearch::InvalidIndex;
}

void SkySearchTest::binaryIntervalSearch()
{
    // Setup
    QFETCH(std::int64_t, timestamp);
    QFETCH(std::size_t, low);
    QFETCH(std::size_t, high);
    QFETCH(int, expected);

    // Exercise
    int result = SkySearch::binaryIntervalSearch(m_positionData, timestamp, low, high);

    // Verify
    QCOMPARE(result, expected);
}

void SkySearchTest::linearIntervalSearch_data()
{
    QTest::addColumn<std::int64_t>("timestamp");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("expected");

    QTest::newRow("Index for ::t1") << ::t0 << 0 << 0;
    QTest::newRow("Index for ::t2") << ::t1 << 0 << 1;
    QTest::newRow("Index for ::t3") << ::t2 << 0 << 2;
    QTest::newRow("Index for t4") << ::t3 << 0 << 3;

    QTest::newRow("Middle")       << (::t3 - ::t0) / 2 << 0 << 1;
    QTest::newRow("After end")    << ::t3 + 1        << 0 << SkySearch::InvalidIndex;
    QTest::newRow("Before start") << ::t0 - 1        << 0 << SkySearch::InvalidIndex;

    QTest::newRow("Start interval 1")   << ::t1 << 1 << 1;
    QTest::newRow("Start interval 2")   << ::t3 << 2 << 3;
    QTest::newRow("Outside interval 1") << ::t1 << 2 << SkySearch::InvalidIndex;
    QTest::newRow("Start interval 3")   << ::t3 << 3 << 3;
}

void SkySearchTest::linearIntervalSearch()
{
    // Setup
    QFETCH(std::int64_t, timestamp);
    QFETCH(int, start);
    QFETCH(int, expected);

    // Exercise
    int result = SkySearch::linearIntervalSearch(m_positionData, timestamp, start);

    // Verify
    QCOMPARE(result, expected);
}

QTEST_MAIN(SkySearchTest)
