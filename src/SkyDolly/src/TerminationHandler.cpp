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
#include <csignal>

#include <QtGlobal>
#include <QString>
#include <QStringBuilder>
#include <QMessageBox>

#include <Kernel/StackTrace.h>
#include <UserInterface/Dialog/TerminationDialog.h>
#include "TerminationHandler.h"

namespace
{
    // Send signal to process:
    // - Linux: kill -s SIGSEGV [pid]
    //          killall -<signal> <process_name>
    volatile std::sig_atomic_t receivedSignal {0};
}

// PUBLIC

void TerminationHandler::handleError(const QString &title, const QString &stackTrace, const std::exception &ex) noexcept
{
    try {
        const QString exceptionMessage = exceptionToString(ex);
        handleError(title, stackTrace, exceptionMessage);
    } catch (const std::exception &ex) {
        qCritical() << "Could not handle the original exception. Another std::exception occurred:" << ex.what();
    } catch (...) {
        qCritical() << "Could not handle the original exception. Another unknown exception occurred.";
    }
}

void TerminationHandler::handleError(const QString &title, const QString &stackTrace, const QString &reason) noexcept
{
    try {
        qCritical() << "Exception message:" << reason;
        TerminationDialog(title, reason, stackTrace).exec();
    } catch (const std::exception &ex) {
        qCritical() << "Could not handle the original exception. Another standard exception occurred:" << ex.what();
    } catch (...) {
        qCritical() << "Could not handle the original exception. Another unknown exception occurred.";
    }
}

void TerminationHandler::handleTerminate() noexcept
{
    // Really make sure that we are not getting into an "endless termination loop"
    std::set_terminate(nullptr);
    try {
        const QString stackTrace = StackTrace::generate();
        if (::receivedSignal != 0) {
            const QString message = signalToString(::receivedSignal);
            handleError("Signal Received", stackTrace, message);
        } else {
            std::exception_ptr ex = std::current_exception();
            if (ex != nullptr) {
                try {
                    std::rethrow_exception(ex);
                } catch (const std::exception &ex) {
                    handleError("Terminate", stackTrace, ex);
                } catch(...) {
                    handleError("Terminate", stackTrace, "Non std::exception");
                }
            } else {
                handleError("Unknown Error", stackTrace, "An unknown error occurred");
            }
        }
    } catch (const std::exception &ex) {
        qCritical() << "Could not handle the errorneous program termination. Another standard exception occurred:" << ex.what();
    } catch (...) {
        qCritical() << "Could not handle the errorneous program termination. Another unknown (non-standard) exception occurred.";
    }

    std::exit(ErrorCode);
}

void TerminationHandler::handleSignal(int signal) noexcept
{
    ::receivedSignal = signal;
    qCritical() << "Signal received:" << signal;
    // A signal handler may only invoke a *very* limited set of std functions
    // (-> "the behavior is undefined if any signal handler performs any of the following: [...]")
    //   https://en.cppreference.com/w/cpp/utility/program/signal
    // However since we already received a "fatal signal" (e.g. segmentation fault)
    // we risk it for the biscuit and try to gather as much as possible information
    std::terminate();
}

// PRIVATE

inline QString TerminationHandler::errorCodeToString(const std::error_code &code)
{
    return QString("Error code: %1\nMessage: %2\nCategory: %3")
        .arg(code.value()).arg(code.message().c_str(), code.category().name());
}

QString TerminationHandler::signalToString(int signal)
{
    QString message;

    switch (signal) {
//    case SIGHUP:
//        message = "The application received a hangup signal (signal SIGHUP)";
//        break;
    case SIGINT:
        message = "The application received an interrupt (signal SIGINT)";
        break;
//    case SIGQUIT:
//        message = "The application received a quit request (signal SIGQUIT)";
//        break;
    case SIGILL:
        message = "An attempt to execute an illegal instruction was made (signal SIGILL)";
        break;
    case SIGABRT:
        message = "An abnormal termination occurred (signal SIGABRT)";
        break;
    case SIGFPE:
        message = "A floating-point exception occurred (signal SIGFPE)";
        break;
    case SIGSEGV:
        message = "A segmentation fault occurred (signal SIGSEGV)";
        break;
//    case SIGPIPE:
//        message = "An attempt to write to a pipe with no readers was made (signal SIGPIPE)";
//        break;
    case SIGTERM:
        message = "A termination request was made (signal SIGTERM)";
        break;
    default:
        message = "An unhandled signal terminated the application, signal: " % QString::number(signal);
        break;
    }

    return message;
}

QString TerminationHandler::exceptionToString(const std::exception &ex)
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
        message = QString("A std::filesystem::filesystem_error:\n%1\npath 1: %2\npath 2: %3")
            .arg(fsex->what(), fsex->path1().c_str(), fsex->path2().c_str());
        if (fsex->code()) {
            message = message % '\n' % errorCodeToString(fsex->code());
        }
        return message;
    }

    // I/O
    auto iosex = dynamic_cast<const std::ios_base::failure *>(&ex);
    if (iosex != nullptr) {
        message = QString("A std::ios_base::failure occurred: %1")
            .arg(iosex->what());
        if (iosex->code()) {
            message = message % '\n' % errorCodeToString(iosex->code());
        }
        return message;
    }

    // Basic exception message
    message = QString("A std::exception occurred:\n%1").arg(ex.what());
    return message;
}
