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
#ifndef LOCATIONSELECTOR_H
#define LOCATIONSELECTOR_H

#include <unordered_set>
#include <cstdint>

#include <QString>

#include <Kernel/Const.h>
#include "Service/EnumerationService.h"
#include "PersistedEnumerationItem.h"
#include "PersistenceLib.h"

struct PERSISTENCE_API LocationSelector
{
    using TypeSelection = std::unordered_set<std::int64_t>;
    TypeSelection typeSelection;
    std::int64_t categoryId {Const::InvalidId};
    std::int64_t countryId {Const::InvalidId};
    QString searchKeyword;

    inline bool hasSelectors() const noexcept
    {
        return typeSelection.size() > 0 ||
               categoryId != Const::InvalidId ||
               countryId != Const::InvalidId ||
               !searchKeyword.isEmpty();
    }

    inline bool showUserLocations() const noexcept
    {
        static const std::int64_t userLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeUserSymId).id()};
        return typeSelection.empty() || typeSelection.contains(userLocationTypeId);
    }
};

#endif // LOCATIONSELECTOR_H
