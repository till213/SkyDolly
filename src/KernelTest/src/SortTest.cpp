/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <stack>

#include <QtTest/QtTest>

#include <Kernel/QUuidHasher.h>
#include <Kernel/Sort.h>
#include "SortTest.h"

// PRIVATE SLOTS

void SortTest::initTestCase()
{}

void SortTest::cleanupTestCase()
{}

void SortTest::topologicalSort_data()
{
//    QTest::addColumn<double>("p0");
//    QTest::addColumn<double>("p1");
//    QTest::addColumn<double>("p2");
//    QTest::addColumn<double>("p3");
//    QTest::addColumn<double>("mu");
//    QTest::addColumn<double>("expected");

}

void SortTest::topologicalSort()
{
    // Setup
//    QFETCH(double, p0);
//    QFETCH(double, p1);
//    QFETCH(double, p2);
//    QFETCH(double, p3);
//    QFETCH(double, mu);
//    QFETCH(double, expected);

    Sort<int>::Graph graph;
    Sort<int>::Node n1;
    n1.id = 1;
    graph[n1.id] = &n1;
    Sort<int>::Node n2;
    n2.id = 2;
    graph[n2.id] = &n2;
    Sort<int>::Node n3;
    n3.id = 3;
    graph[n3.id] = &n3;

    n1.vertices.push_back(&n2);
    n1.vertices.push_back(&n3);
    n2.vertices.push_back(&n3);

    // Exercise
    std::stack<Sort<int>::Node *> sorted = Sort<int>::topolocicalSort(graph);

    qDebug("List");
    // Verify
    QCOMPARE(sorted.top()->id, 1);
    sorted.pop();
    QCOMPARE(sorted.top()->id, 2);
    sorted.pop();
    QCOMPARE(sorted.top()->id, 3);
    sorted.pop();
}

QTEST_MAIN(SortTest)
