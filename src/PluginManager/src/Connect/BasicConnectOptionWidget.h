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
#ifndef BASICCONNECTOPTIONWIDGET_H
#define BASICCONNECTOPTIONWIDGET_H

#include <memory>

#include <QKeySequence>

#include <OptionWidgetIntf.h>

namespace Ui {
    class BasicConnectOptionWidget;
}

class ConnectPluginBaseSettings;
struct BasicConnectOptionWidgetPrivate;

class BasicConnectOptionWidget : public OptionWidgetIntf
{
    Q_OBJECT
public:
    explicit BasicConnectOptionWidget(ConnectPluginBaseSettings &pluginSettings, QWidget *parent = nullptr);
    BasicConnectOptionWidget(const BasicConnectOptionWidget &rhs) = delete;
    BasicConnectOptionWidget(BasicConnectOptionWidget &&rhs) = delete;
    BasicConnectOptionWidget &operator=(const BasicConnectOptionWidget &rhs) = delete;
    BasicConnectOptionWidget &operator=(BasicConnectOptionWidget &&rhs) = delete;
    ~BasicConnectOptionWidget() override;

    void accept() noexcept override;

private:
    enum struct KeySequence
    {
        Record,
        Replay,
        Pause,
        Stop,
        Backward,
        Forward,
        Begin,
        End
    };

    Ui::BasicConnectOptionWidget *ui;
    std::unique_ptr<BasicConnectOptionWidgetPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    void handleDuplicateKeySequences(const QKeySequence &keySequence, KeySequence source) const noexcept;

private slots:
    void updateUi() noexcept;

    void handleRecordKeySequence() const noexcept;
    void handleReplayKeySequence() const noexcept;
    void handlePauseKeySequence() const noexcept;
    void handleStopKeySequence() const noexcept;
    void handleBackwardKeySequence() const noexcept;
    void handleForwardKeySequence() const noexcept;
    void handleBeginKeySequence() const noexcept;
    void handleEndKeySequence() const noexcept;
};

#endif // BASICCONNECTOPTIONWIDGET_H
