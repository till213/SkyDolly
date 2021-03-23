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
#include <memory>
#include <iostream>
#include <vector>

#include "World.h"

class WorldPrivate
{
public:
    WorldPrivate() noexcept
    {}

    ~WorldPrivate() noexcept
    {}

    std::vector<std::unique_ptr<Scenario>> scenarios;

    static World *instance;

};

World *WorldPrivate::instance = nullptr;

// PUBLIC

World &World::getInstance() noexcept
{
    if (WorldPrivate::instance == nullptr) {
        WorldPrivate::instance = new World();
    }
    return *WorldPrivate::instance;
}

void World::destroyInstance() noexcept
{
    if (WorldPrivate::instance != nullptr) {
        delete WorldPrivate::instance;
        WorldPrivate::instance = nullptr;
    }
}

Scenario &World::getCurrentScenario() const
{
    return *(*d->scenarios.cbegin());
}

// PROTECTED

World::~World()
{
}

// PRIVATE

World::World() noexcept
    : d(std::make_unique<WorldPrivate>())
{
    // World may support several scenarios, but for now there will be always
    // exactly one
    std::unique_ptr<Scenario> defaultScenario = std::make_unique<Scenario>();
    d->scenarios.push_back(std::move(defaultScenario));
}
