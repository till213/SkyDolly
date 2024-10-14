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
#include <exception>

#include <QDebug>
#include <QString>
#include <QTextStream>
#include <QIODeviceBase>

#include <cpptrace/cpptrace.hpp>

#include "StackTrace.h"

namespace {
    constexpr unsigned n_digits(unsigned value) {
        return value < 10 ? 1 : 1 + n_digits(value / 10);
    }

    QString formatTrace(const cpptrace::stacktrace &trace) noexcept
    {
        QString stackTrace;
        QTextStream out(&stackTrace, QIODeviceBase::WriteOnly);

        std::size_t counter = 0;
        if (!trace.empty()) {
            const auto frames = trace.frames;
            out << "Stack trace (most recent call first):" << Qt::endl;
            const auto frame_number_width = n_digits(static_cast<int>(frames.size()) - 1);
            for(const auto &frame : frames) {
                out << '#'
                    << qSetFieldWidth(static_cast<int>(frame_number_width))
                    << Qt::left
                    << counter++
                    << Qt::right
                    << " "
                    << Qt::hex
                    << "0x"
                    << qSetFieldWidth(2 * sizeof(uintptr_t))
                    << qSetPadChar('0')
                    << frame.object_address
                    << qSetFieldWidth(0)
                    << Qt::dec
                    << qSetPadChar(' ')
                    << " in "
                    << QString::fromStdString(frame.symbol)
                    << " at "
                    << QString::fromStdString(frame.filename)
                    << ":"
                    << QString::number(frame.line.value_or(0))
                    << ":"
                    << QString::number(frame.column.value_or(0))
                    << Qt::endl;
            }
        } else {
            out << "No stack trace available." << Qt::endl;
        }
        return stackTrace;
    }
}

// PUBLIC

QString StackTrace::generate() noexcept
{
    QString stackTrace;
    try {
        const auto trace = cpptrace::generate_trace();
        trace.print();
        stackTrace = ::formatTrace(trace);
    } catch (const std::exception &ex) {
        qCritical() << "An exception occurred while trying to generate stack trace:" << ex.what();
    } catch (...) {
        qCritical() << "Unknown exception occurred while trying to generate stack trace.";
    }
    return stackTrace;
}
