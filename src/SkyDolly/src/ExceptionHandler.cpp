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
#include <exception>
#include <filesystem>
#include <system_error>

#include <QtGlobal>
#include <QString>
#include <QStringBuilder>
#include <QMessageBox>

#include <Kernel/StackTrace.h>
#include "ExceptionHandler.h"

// PUBLIC

void ExceptionHandler::handle(const QString &message, const std::exception &ex) noexcept
{
    try {
        const QString exceptionMessage = exceptionToString(ex);
        qCritical() << "Exception message:" << exceptionMessage;
        const QString dialogMessage = message % "\n\n" % exceptionMessage;
        QMessageBox::critical(nullptr, "Error", dialogMessage);
    } catch (std::exception &ex) {
        qFatal() << "Could not handle the original exception. Another standard exception occurred:" << ex.what();
    } catch (...) {
        qFatal() << "Could not handle the original exception. Another unknown exception occurred.";
    }
}

void ExceptionHandler::handleTerminate() noexcept
{
    // Really make sure that we are not getting into an "endless termination loop"
    std::set_terminate(nullptr);

    const QString stackTrace = StackTrace::generate();
    try {
        QString message = QStringLiteral("The application quit unexpectedly. "
                                         "This is due to a programming error - Sky Dolly deeply apologises.\n\n"
                                         "You can help fixing this bug by creating an issue at https://github.com/till213/SkyDolly/issues."
                                         "Please provide a screenshot of this dialog.");
        std::exception_ptr ex = std::current_exception();
        try {
            std::rethrow_exception(ex);
        } catch (std::exception &ex) {
            handle(message, ex);
        } catch(...) {
            QMessageBox::critical(nullptr, "Error", message % "An unknown (non-standard) exception occurred.");
        }
    } catch (std::exception &ex) {
        qFatal() << "Could not handle the errorneous program termination. Another standard exception occurred:" << ex.what();
    } catch (...) {
        qFatal() << "Could not handle the errorneous program termination. Another unknown (non-standard) exception occurred.";
    }

    std::abort();
}

// PRIVATE

inline QString ExceptionHandler::errorCodeToString(const std::error_code &code)
{
    return QString("Error code: %1\nMessage: %2\nCategory: %3")
        .arg(code.value()).arg(code.message().c_str(), code.category().name());
}

QString ExceptionHandler::exceptionToString(const std::exception &ex)
{
    QString message;

    // File system
    auto baex = dynamic_cast<const std::bad_alloc *>(&ex);
    if (baex != nullptr) {
        message = QString("Memory could not be allocated: %1")
            .arg(baex->what());
        return message;
    }

    // File system
    auto fsex = dynamic_cast<const std::filesystem::filesystem_error *>(&ex);
    if (fsex != nullptr) {
        message = QString("A filesystem error occurred:\n\n%1\npath 1: %2\npath 2: %3")
            .arg(fsex->what(), fsex->path1().c_str(), fsex->path2().c_str());
        if (fsex->code()) {
            message = message % '\n' % errorCodeToString(fsex->code());
        }
        return message;
    }

    // I/O
    auto iosex = dynamic_cast<const std::ios_base::failure *>(&ex);
    if (iosex != nullptr) {
        message = QString("An input/output error occurred: %1")
            .arg(iosex->what());
        if (iosex->code()) {
            message = message % '\n' % errorCodeToString(iosex->code());
        }
        return message;
    }

    // Basic exception message
    message = QString("An exception occurred:\n\n%1").arg(ex.what());
    return message;
}
