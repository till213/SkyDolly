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
#include <memory>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Connect/ConnectPluginBaseSettings.h>
#include "BasicConnectOptionWidget.h"
#include "ui_BasicConnectOptionWidget.h"

struct BasicConnectOptionWidgetPrivate
{
    BasicConnectOptionWidgetPrivate(ConnectPluginBaseSettings &pluginSettings)
        : pluginSettings(pluginSettings)
    {}

    ConnectPluginBaseSettings &pluginSettings;
};

// PUBLIC

BasicConnectOptionWidget::BasicConnectOptionWidget(ConnectPluginBaseSettings &pluginSettings, QWidget *parent)
    : OptionWidgetIntf(parent),
      ui(new Ui::BasicConnectOptionWidget),
      d(std::make_unique<BasicConnectOptionWidgetPrivate>(pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

BasicConnectOptionWidget::~BasicConnectOptionWidget()
{
#ifdef DEBUG
    qDebug() << "BasicConnectOptionWidget::~BasicConnectOptionWidget: DELETED";
#endif
    delete ui;
}

void BasicConnectOptionWidget::accept() noexcept
{
    FlightSimulatorShortcuts shortcuts;
    shortcuts.record = ui->recordSequenceEdit->keySequence();
    shortcuts.replay = ui->replaySequenceEdit->keySequence();
    shortcuts.pause = ui->pauseSequenceEdit->keySequence();
    shortcuts.stop = ui->stopSequenceEdit->keySequence();
    shortcuts.backward = ui->backwardSequenceEdit->keySequence();
    shortcuts.forward = ui->forwardSequenceEdit->keySequence();
    shortcuts.begin = ui->beginSequenceEdit->keySequence();
    shortcuts.end = ui->endSequenceEdit->keySequence();
    d->pluginSettings.setFlightSimulatorShortcuts(shortcuts);
}

// PRIVATE

void BasicConnectOptionWidget::initUi() noexcept
{
    ui->recordSequenceEdit->setMaximumSequenceLength(1);
    ui->replaySequenceEdit->setMaximumSequenceLength(1);
    ui->pauseSequenceEdit->setMaximumSequenceLength(1);
    ui->stopSequenceEdit->setMaximumSequenceLength(1);
    ui->backwardSequenceEdit->setMaximumSequenceLength(1);
    ui->forwardSequenceEdit->setMaximumSequenceLength(1);
    ui->beginSequenceEdit->setMaximumSequenceLength(1);
    ui->endSequenceEdit->setMaximumSequenceLength(1);
}

void BasicConnectOptionWidget::frenchConnection() noexcept
{
    connect(ui->recordSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleRecordKeySequence);
    connect(ui->replaySequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleReplayKeySequence);
    connect(ui->pauseSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handlePauseKeySequence);
    connect(ui->stopSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleStopKeySequence);
    connect(ui->backwardSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleBackwardKeySequence);
    connect(ui->forwardSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleForwardKeySequence);
    connect(ui->beginSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleBeginKeySequence);
    connect(ui->endSequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &BasicConnectOptionWidget::handleEndKeySequence);
}

void BasicConnectOptionWidget::handleDuplicateKeySequences(const QKeySequence &keySequence, KeySequence source) const noexcept
{
    if (source != KeySequence::Record && ui->recordSequenceEdit->keySequence() == keySequence) {
        ui->recordSequenceEdit->clear();
    }
    if (source != KeySequence::Replay && ui->replaySequenceEdit->keySequence() == keySequence) {
        ui->replaySequenceEdit->clear();
    }
    if (source != KeySequence::Pause && ui->pauseSequenceEdit->keySequence() == keySequence) {
        ui->pauseSequenceEdit->clear();
    }
    if (source != KeySequence::Stop && ui->stopSequenceEdit->keySequence() == keySequence) {
        ui->stopSequenceEdit->clear();
    }
    if (source != KeySequence::Backward && ui->backwardSequenceEdit->keySequence() == keySequence) {
        ui->backwardSequenceEdit->clear();
    }
    if (source != KeySequence::Forward && ui->forwardSequenceEdit->keySequence() == keySequence) {
        ui->forwardSequenceEdit->clear();
    }
    if (source != KeySequence::Begin && ui->beginSequenceEdit->keySequence() == keySequence) {
        ui->beginSequenceEdit->clear();
    }
    if (source != KeySequence::End && ui->endSequenceEdit->keySequence() == keySequence) {
        ui->endSequenceEdit->clear();
    }
}

// PRIVATE SLOTS

void BasicConnectOptionWidget::updateUi() noexcept
{
    const FlightSimulatorShortcuts &shortcuts = d->pluginSettings.getFlightSimulatorShortcuts();
    ui->recordSequenceEdit->setKeySequence(shortcuts.record);
    ui->replaySequenceEdit->setKeySequence(shortcuts.replay);
    ui->pauseSequenceEdit->setKeySequence(shortcuts.pause);
    ui->stopSequenceEdit->setKeySequence(shortcuts.stop);
    ui->backwardSequenceEdit->setKeySequence(shortcuts.backward);
    ui->forwardSequenceEdit->setKeySequence(shortcuts.forward);
    ui->beginSequenceEdit->setKeySequence(shortcuts.begin);
    ui->endSequenceEdit->setKeySequence(shortcuts.end);
}

void BasicConnectOptionWidget::handleRecordKeySequence() const noexcept
{
    auto sequence = ui->recordSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Record);
}

void BasicConnectOptionWidget::handleReplayKeySequence() const noexcept
{
    auto sequence = ui->replaySequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Replay);
}

void BasicConnectOptionWidget::handlePauseKeySequence() const noexcept
{
    auto sequence = ui->pauseSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Pause);
}

void BasicConnectOptionWidget::handleStopKeySequence() const noexcept
{
    auto sequence = ui->stopSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Stop);
}

void BasicConnectOptionWidget::handleBackwardKeySequence() const noexcept
{
    auto sequence = ui->backwardSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Backward);
}

void BasicConnectOptionWidget::handleForwardKeySequence() const noexcept
{
    auto sequence = ui->forwardSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Forward);
}

void BasicConnectOptionWidget::handleBeginKeySequence() const noexcept
{
    auto sequence = ui->beginSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::Begin);
}

void BasicConnectOptionWidget::handleEndKeySequence() const noexcept
{
    auto sequence = ui->endSequenceEdit->keySequence();
    handleDuplicateKeySequences(sequence, KeySequence::End);
}
