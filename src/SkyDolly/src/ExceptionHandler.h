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
#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <exception>
#include <system_error>

#include <QString>

/**
 * The exception handler handles unexpected application exceptions, due to
 * - Uncaught exceptions
 * - Unexpected exceptions (thrown from "noexcept" methods)
 * - Fatal signals such as "segmentation faults"
 */
class ExceptionHandler
{
public:
    static void onError(const QString &title, const QString &stackTrace, const std::exception &ex) noexcept;
    static void onError(const QString &title, const QString &stackTrace, const QString &exceptionMessage) noexcept;
    static void onTerminate() noexcept;
    static void onSignal(int signal) noexcept;
private:
    static QString errorCodeToString(const std::error_code &code);
    static QString exceptionToString(const std::exception &ex);
};

#endif // EXCEPTIONHANDLER_H
