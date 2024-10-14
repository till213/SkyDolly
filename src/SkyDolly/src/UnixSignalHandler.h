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
#ifndef UNIXSIGNALHANDLER_H
#define UNIXSIGNALHANDLER_H

#include <memory>

#include <QObject>
#include <QObject>
#include <QString>

class QSocketNotifier;

class UnixSignalHandler : public QObject
{
    Q_OBJECT
public:
    UnixSignalHandler(QObject *parent = nullptr);
    virtual ~UnixSignalHandler();

    void registerSignals() noexcept;

private:
    static int m_signalSocketPair[2];    
    std::unique_ptr<QSocketNotifier> m_signalNotifier;

    void frenchConnection() noexcept;
    static QString signalToString(int signal);
    static void on(int signal);

private slots:
    void process();
};

#endif // UNIXSIGNALHANDLER_H
