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
#include <stack>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>

#include <QtTest/QtTest>

#include <Kernel/QUuidHasher.h>
#include <Kernel/Sort.h>
#include "SortTest.h"

// PRIVATE SLOTS

using ID = int;
// The edges to the vertices, identified by their ID
using EdgeList = std::vector<ID>;
// Defines an edge from 'first' to all other vertices in 'second', identified by their ID
using PerVertexEdgeList = std::pair<ID, EdgeList>;
using Vertex = Sort<ID>::Vertex;
using Sorting = Sort<ID>::Sorting;

void SortTest::initTestCase()
{}

void SortTest::cleanupTestCase()
{}

Q_DECLARE_METATYPE(Sorting);

void SortTest::topologicalSort_data()
{
    QTest::addColumn<std::vector<ID>>("vertices");
    // Key: node ID - value: list of node IDs ("edges")
    QTest::addColumn<std::vector<PerVertexEdgeList>>("edgeLists");
    QTest::addColumn<Sort<ID>::Sorting>("sorting");
    QTest::addColumn<std::vector<ID>>("expectedIDs");

    QTest::newRow("Normal order") << std::vector<ID> {1, 2, 3}
                                  << std::vector<PerVertexEdgeList> { std::make_pair(1, std::vector<int> {2, 3}),
                                                                      std::make_pair(2, std::vector<int> {3})
                                                                    }
                                  << Sorting::Normal
                                  << std::vector<ID> {1, 2, 3};
    QTest::newRow("Reverse order") << std::vector<ID> {1, 2, 3}
                                   << std::vector<PerVertexEdgeList> { std::make_pair(1, std::vector<int> {2, 3}),
                                                                       std::make_pair(2, std::vector<int> {3})
                                                                     }
                                   << Sorting::Reverse
                                   << std::vector<ID> {3, 2, 1};
    QTest::newRow("No edges 1") << std::vector<ID> {1, 2, 3}
                                << std::vector<PerVertexEdgeList> {}
                                << Sorting::Normal
                                << std::vector<ID> {3, 2, 1};
    QTest::newRow("No edges 2") << std::vector<ID> {1, 2, 3}
                                << std::vector<PerVertexEdgeList> {}
                                << Sorting::Reverse
                                << std::vector<ID> {1, 2, 3};
    QTest::newRow("No edges 3") << std::vector<ID> {3, 2, 1}
                                << std::vector<PerVertexEdgeList> {}
                                << Sorting::Normal
                                << std::vector<ID> {1, 2, 3};
    QTest::newRow("No edges 4") << std::vector<ID> {3, 2, 1}
                                << std::vector<PerVertexEdgeList> {}
                                << Sorting::Reverse
                                << std::vector<ID> {3, 2, 1};
    QTest::newRow("Not a DAG") << std::vector<ID> {1, 2, 3}
                               << std::vector<PerVertexEdgeList> { std::make_pair(1, std::vector<int> {2, 3}),
                                                                   std::make_pair(2, std::vector<int> {3}),
                                                                   std::make_pair(3, std::vector<int> {1})
                                                                 }
                               << Sorting::Normal
                               << std::vector<ID> {};
    QTest::newRow("Empty DAG") << std::vector<ID> {}
                               << std::vector<PerVertexEdgeList> {}
                               << Sorting::Normal
                               << std::vector<ID> {};
}

void SortTest::topologicalSort()
{

    // Setup
    QFETCH(std::vector<ID>, vertices);
    QFETCH(std::vector<PerVertexEdgeList>, edgeLists);
    QFETCH(Sorting, sorting);
    QFETCH(std::vector<ID>, expectedIDs);

    Sort<int>::Graph graph;
    for (ID id : vertices) {
        std::shared_ptr<Vertex> vertex = std::make_shared<Vertex>(id);
        graph[id] = vertex;
    }

    for (const auto &l : edgeLists) {
        int nodeId = l.first;
        std::vector<ID> edgeList = l.second;
        std::shared_ptr<Vertex> sourceVertex = graph.at(nodeId);
        sourceVertex->edges.reserve(edgeList.size());
        for (const int id : edgeList) {
            sourceVertex->edges.push_back(graph.at(id).get());
        }
    }

    // Exercise
    std::deque<Vertex *> sorted = Sort<ID>::topologicalSort(graph, sorting);

    // Verify
    int i {0};
    if (expectedIDs.size() > 0) {
        for (int id : expectedIDs) {
            QCOMPARE(sorted.at(i)->id, id);
            ++i;
        }
    } else {
        QCOMPARE(sorted.size(), 0);
    }
}

QTEST_MAIN(SortTest)
