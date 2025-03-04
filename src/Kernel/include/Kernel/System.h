/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SYSTEM_H
#define SYSTEM_H

#include <QString>
#include <QSysInfo>

#include "KernelLib.h"

class KERNEL_API System final
{
public:
    /*!
     * Returns the name of the currently logged in user.
     *
     * Implementation note: the current implementation is extremely simplistic
     * and relies on environment variables USERNAME or USER.
     *
     * \return the name of the logged in user; may be an empty string
     */
    static QString getUsername() noexcept;

    /*!
     * Returns whether the application runs on Windows (e.g. 10 or 11).
     *
     * \return \c true if the operating system is Windows; \c false else
     * \sa isWindows10
     * \sa isWindows11
     */
    static inline bool isWindows()
    {
        return QSysInfo::productType() == "windows";
    }

    /*!
     * Returns whether the application runs on Windows 10.
     *
     * \return \c true if the operating system is Windows 10; \c false else
     */
    static inline bool isWindows10()
    {
        return isWindows() && QSysInfo::productVersion() == "10";
    }

    /*!
     * Returns whether the application runs on Windows 11.
     *
     * \return \c true if the operating system is Windows 11; \c false else
     */
    static inline bool isWindows11()
    {
        return isWindows() && QSysInfo::productVersion() == "11";
    }
};

#endif // SYSTEM_H
