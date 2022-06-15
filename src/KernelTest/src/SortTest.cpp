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

void SortTest::initTestCase()
{}

void SortTest::cleanupTestCase()
{}

void SortTest::topologicalSort_data()
{
    QTest::addColumn<std::vector<ID>>("vertices");
    // Key: node ID - value: list of node IDs ("edges")
    QTest::addColumn<std::vector<PerVertexEdgeList>>("edgeLists");
    QTest::addColumn<std::vector<ID>>("expectedIDs");


    QTest::newRow("Case 1") << std::vector<ID> {1, 2, 3}
                            << std::vector<PerVertexEdgeList> { std::make_pair(1, std::vector<int> {2, 3}),
                                                                std::make_pair(2, std::vector<int> {3})
                                                              }
                            << std::vector<ID> {1, 2, 3};

}

void SortTest::topologicalSort()
{

    // Setup
    QFETCH(std::vector<ID>, vertices);
    QFETCH(std::vector<PerVertexEdgeList>, edgeLists);
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
        for (const int id : edgeList) {
            sourceVertex->edges.push_back(graph.at(id));
        }
    }

    // Exercise
    std::stack<std::shared_ptr<Vertex>> sorted = Sort<ID>::topolocicalSort(graph);

    // Verify
    for (int id : expectedIDs) {
        QCOMPARE(sorted.top()->id, id);
        sorted.pop();
    }
}

QTEST_MAIN(SortTest)
