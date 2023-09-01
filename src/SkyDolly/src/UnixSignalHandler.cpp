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
#include <csignal>
#include <vector>
#include <unistd.h>

#include <sys/socket.h>

#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QSocketNotifier>
#include <QDebug>

#include <Kernel/StackTrace.h>
#include <UserInterface/Dialog/TerminationDialog.h>
#include "ErrorCodes.h"
#include "UnixSignalHandler.h"

namespace
{
    // Send signal to process:
    // - Linux: kill -s <signal> <pid>
    //          killall -<signal> <process_name>
    volatile std::sig_atomic_t receivedSignal {0};
}

// PUBLIC

UnixSignalHandler::UnixSignalHandler(QObject *parent)
    : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_signalSocketPair)) {
        qFatal("Couldn't create socketpair");
    }

    m_signalNotifier = new QSocketNotifier(m_signalSocketPair[1], QSocketNotifier::Read, this);
    frenchConnection();
}

UnixSignalHandler::~UnixSignalHandler() = default;

int UnixSignalHandler::m_signalSocketPair[2];

void UnixSignalHandler::registerSignals() noexcept
{
   // const std::vector unixSignals {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV};
    const std::vector unixSignals {SIGINT};
    struct sigaction action;

    ::memset(&action, '\0', sizeof(action));
    action.sa_handler = UnixSignalHandler::handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_RESTART;

    for (int signal : unixSignals) {
        if (::sigaction(signal, &action, nullptr) != 0) {
            break;;
        }
    }

    // TODO: This is not an optimal solution:
    //       - std::signal does not block signals while executing the handler
    //       - We should map signals onto Qt signals, via a (Unix) socket pair
    //         (https://doc.qt.io/qt-6/unix-signals.html)
    //       - Unix signals are not sent on Windows anyway: we should use SetConsoleCtrlHandler and friends
    // Fatal signals (if not caught)
    // https://stackoverflow.com/questions/13219071/which-fatal-signals-should-a-user-level-program-catch
//    std::signal(SIGHUP, UnixSignalHandler::handle);
//    std::signal(SIGINT, UnixSignalHandler::handle);
//    std::signal(SIGQUIT, UnixSignalHandler::handle);
//    std::signal(SIGILL, UnixSignalHandler::handle);
//    std::signal(SIGABRT, UnixSignalHandler::handle);
//    std::signal(SIGFPE, UnixSignalHandler::handle);
//    std::signal(SIGSEGV, UnixSignalHandler::handle);
//    std::signal(SIGPIPE, UnixSignalHandler::handle);
//    std::signal(SIGTERM, UnixSignalHandler::handle);
//    std::signal(SIGUSR1, UnixSignalHandler::handle);
//    std::signal(SIGUSR2, UnixSignalHandler::handle);
}

// PRIVATE

void UnixSignalHandler::frenchConnection() noexcept
{
    connect(m_signalNotifier, &QSocketNotifier::activated,
            this, &UnixSignalHandler::process);
}

QString UnixSignalHandler::signalToString(int signal)
{
    QString message;

    switch (signal) {
    case SIGHUP:
        message = "The application received a hangup signal (signal SIGHUP)";
        break;
    case SIGINT:
        message = "The application received an interrupt (signal SIGINT)";
        break;
    case SIGQUIT:
        message = "The application received a quit request (signal SIGQUIT)";
        break;
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
    case SIGPIPE:
        message = "An attempt to write to a pipe with no readers was made (signal SIGPIPE)";
        break;
    case SIGTERM:
        message = "A termination request was made (signal SIGTERM)";
        break;
    default:
        message = "An unhandled signal terminated the application, signal: " % QString::number(signal);
        break;
    }

    return message;
}

void UnixSignalHandler::handle(int signal)
{
    ::receivedSignal = signal;
    qCritical() << "Signal received:" << signal;
    ::write(m_signalSocketPair[0], &signal, sizeof(int));
}

// PRIVATE SLOTS

void UnixSignalHandler::process()
{
    int signal {0};
    ::read(m_signalSocketPair[1], &signal, sizeof(int));

    const QString stackTrace = StackTrace::generate();
    const QString reason = signalToString(signal);
    try {
        qCritical() << "Received signal:" << signal;
        TerminationDialog(tr("Signal Received"), reason, stackTrace).exec();
    } catch (const std::exception &ex) {
        qCritical() << "Could not handle the original exception. Another standard exception occurred:" << ex.what();
    } catch (...) {
        qCritical() << "Could not handle the original exception. Another unknown exception occurred.";
    }
    exit(ErrorCodes::Signal);
}
