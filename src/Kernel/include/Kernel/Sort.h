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
#ifndef SORT_H
#define SORT_H

#include <vector>
#include <unordered_map>
#include <stack>
#include <functional>

template <typename T, typename H = std::hash<T>> class Sort
{
public:
    enum struct State
    {
        NotVisited,
        Visiting,
        Done
    };

    struct Node
    {
        explicit Node()
            : id(T())
        {};

        explicit Node(T theId)
            : id(theId)
        {};

        T id;
        std::vector<Node *> vertices;
        State state {State::NotVisited};
    };

    using Graph = std::unordered_map<T, Node, H>;

    /*!
     * Sorts the nodes in the \c graph in topological order. A topological sort or topological
     * ordering of a directed graph is a linear ordering of its vertices such that for every
     * directed edge uv from vertex u to vertex v, u comes before v in the ordering.
     *
     * \param graph
     *        the graph to be sorted
     * \return the nodes sorted in topological order, with the first node on top of the stack;
     *         an empty stack if the \c graph is empty as well or not a directed acyclic graph (DAG)
     */
    static std::stack<Node *> topolocicalSort(Graph &graph) noexcept
    {
        std::stack<Node *> sortedStack;
        for (auto &it : graph) {
            it.second.state = State::NotVisited;
        }
        for (auto &it : graph) {
            if (it.second.state != State::Done) {
                visit(it.second, sortedStack);
            }
        }
        return sortedStack;
    }

private:
    static void visit(Node &node, std::stack<Node *> &sorted) noexcept
    {
        if (node.state == State::Done) {
            return;
        } else if (node.state == State::Visiting) {
            // Not a DAG -> reset the stack
            sorted = std::stack<Node *>();
            return;
        }

        node.state = State::Visiting;
        for (Node *n : node.vertices) {
            visit(*n, sorted);
        }
        node.state = State::Done;
        sorted.push(&node);
    }
};

#endif // SORT_H
