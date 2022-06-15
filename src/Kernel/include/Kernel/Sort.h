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
#include <deque>
#include <functional>
#include <memory>

#include <tsl/ordered_map.h>

/*!
 * Sorts elements (vertices) in topological order, depending on their \c edges which define the
 * dependencies. In case no edges exist the order is the reverse order in which the elements (vertices)
 * have been added to the Graph with Sorting = Normal (due to the depth first visit pattern) respectively
 * the same order of insertion with Sorting = Reverse.
 */
template <typename T, typename H = std::hash<T>> class Sort
{
public:
    enum struct State
    {
        NotVisited,
        Visiting,
        Done
    };
    enum struct Sorting
    {
        Normal,
        Reverse,
    };

    struct Vertex
    {
        explicit Vertex()
            : id(T())
        {};

        explicit Vertex(T theId)
            : id(theId)
        {};

        T id;
        /*!
         * The edges are defined by this and their end vertices given in the \c edges list.
         */
        std::vector<std::shared_ptr<Vertex>> edges;
        State state {State::NotVisited};
    };

    using Graph = tsl::ordered_map<T, std::shared_ptr<Vertex>, H>;

    /*!
     * Sorts the vertices in the \c graph in topological order. A topological sort or topological
     * ordering of a directed graph is a linear ordering of its vertices such that for every
     * directed edge uv from vertex u to vertex v, u comes before v in the ordering, unless
     * \c reverse is set to \c true in which case a directed edge from vertex u to v means "u depends
     * on v" (or "u comes after v").
     *
     * \param graph
     *        the graph to be sorted
     * \return the nodes sorted in topological order, with the first node at beginning of the deque;
     *         an empty deque if the \c graph is either empty or not a directed acyclic graph (DAG)
     */
    static std::deque<std::shared_ptr<Vertex>> topologicalSort(Graph &graph, Sorting sorting = Sorting::Normal) noexcept
    {
        std::deque<std::shared_ptr<Vertex>> sorted;
        for (auto &it : graph) {
            it.second->state = State::NotVisited;
        }
        bool ok {true};
        for (auto &it : graph) {
            if (it.second->state != State::Done) {
                ok = visit(it.second, sorting, sorted);
                if (!ok) {
                    // Not a DAG -> clear sorted vertices
                    sorted.clear();
                    break;
                }
            }
        }
        return sorted;
    }

private:
    // Traverses the vertices (starting from the given 'vertex') in depth-first order, adding the 'vertex' to
    // the 'sorted' deque (at the front when sorted = Normal, at the end when Reverse) once all its neighbouring
    // vertices have completely been recursively visited.
    // Returns true if the visit was successful or false in case a cycle was detected (in which case the 'sorted' result
    // is left as is, without cycle)
    static bool visit(std::shared_ptr<Vertex> vertex, Sorting sorting, std::deque<std::shared_ptr<Vertex>> &sorted) noexcept
    {
        if (vertex->state == State::Done) {
            // Already visited
            return true;
        } else if (vertex->state == State::Visiting) {
            // Not a DAG (cycle detected)
            return false;
        }

        vertex->state = State::Visiting;
        bool ok {true};
        for (std::shared_ptr<Vertex> n : vertex->edges) {
            ok = visit(n, sorting, sorted);
            if (!ok) {
                break;
            }
        }
        vertex->state = State::Done;
        if (sorting == Sorting::Normal) {
            sorted.push_front(vertex);
        } else {
            sorted.push_back(vertex);
        }
        return ok;
    }
};

#endif // SORT_H
