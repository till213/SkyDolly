/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef ENUM_H
#define ENUM_H

#include <type_traits>

/*!
 * C++ enumeration support functionality.
 */
namespace Enum
{
    /*!
     * Returns the underlying type for the given enumeration \p e.
     *
     * Note: Can be replaced in C++23 with https://en.cppreference.com/w/cpp/utility/to_underlying
     */
    template<typename E>
    constexpr auto underly(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    /*!
     * Returns whether the enumeration \p E contains the given underlying \p value.
     *
     * Assumptions:
     * - The enumeration \p E contains the following members: \p First (with lowest value) and \p Last (with highest value)
     * - The underlying values are without "gaps", that is First = 0, Value1 = First, Value2 = 1, Value3 = 2, ..., ValueN = 10, Last = ValueN
     *
     * \param value
     *        the underlying value to be checked
     * \return \c true if the \p value is a valid underlying value contained in enumeration of type \p E; \c false else
     */
    template<typename E> requires(std::is_enum_v<E>)
    constexpr auto contains(std::underlying_type_t<E> value)
    {
        return value >= static_cast<std::underlying_type_t<E>>(E::First) &&
               value <= static_cast<std::underlying_type_t<E>>(E::Last);
    }
}

#endif // ENUM_H
