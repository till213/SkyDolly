/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <vector>

#include <QVariant>
#include <QString>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "LinkedOptionGroup.h"

namespace
{
constexpr const char *UserData {"UserData"};
}

static QString strip_normal(
"QPushButton {"
"   margin: 0; padding: 4px; border: 0px;"
"   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                              stop: 0 #f6f7fa, stop: 1 #aaabae);"
"}");
static QString strip_checked(
"QPushButton:checked {"
"   background-color: #aaa;"
"}");
static QString strip_single(
"QPushButton{"
"   border-radius: 6px;"
"}");
static QString strip_first(
"QPushButton{"
"   border-top-left-radius: 6px;"
"   border-bottom-left-radius: 6px;"
"}");
static QString strip_last(
"QPushButton{"
"   border-top-right-radius: 6px;"
"   border-bottom-right-radius: 6px;"
"}");

static QString widget_back(
"QWidget {"
"   background-color: blue;"
"}");

struct LinkedOptionGroupPrivate
{
    QHBoxLayout *layout {nullptr};
    std::vector<QPushButton *> buttons;
};

// PUBLIC

QPushButton *createButton(QString const& name,
                          bool checked,
                          QString const& sheet = QString())
{
    QPushButton *pb = new QPushButton(name);
    pb->setCheckable(true);
    pb->setChecked(checked);
    if (!sheet.isEmpty())
        pb->setStyleSheet(sheet);
    return pb;
}

LinkedOptionGroup::LinkedOptionGroup(QWidget *parent) noexcept
    : QWidget(parent),
      d(std::make_unique<LinkedOptionGroupPrivate>())
{
    initUi();
}

LinkedOptionGroup::~LinkedOptionGroup() = default;

void LinkedOptionGroup::addOption(const QString &name, const QVariant &userData) noexcept
{
    QPushButton *button = new QPushButton(name, this);
    button->setCheckable(true);
    button->setProperty(::UserData, userData);
    d->buttons.push_back(button);
    std::size_t buttonCount = d->buttons.size();
    if (buttonCount == 1) {
        button->setStyleSheet(strip_single);
    } else if (buttonCount == 2) {
        button->setStyleSheet(strip_last);
        // Also update the first button
        d->buttons.front()->setStyleSheet(strip_first);
    } else {
        button->setStyleSheet(strip_last);
        // Also update the second to last button
        d->buttons[buttonCount - 2]->setStyleSheet("");
    }

    d->layout->addWidget(button);
    connect(button, &QPushButton::toggled,
            this, &LinkedOptionGroup::onButtonToggled);
}

// PRIVATE

void LinkedOptionGroup::initUi() noexcept
{
    /* style sheet applies to this widget and its children */
    setStyleSheet(widget_back+strip_normal+strip_checked);

    /* Layout with no spacing */
    d->layout = new QHBoxLayout(this);
    d->layout->setSpacing(0);

    setLayout(d->layout);
}

// PRIVATE SLOTS

void LinkedOptionGroup::onButtonToggled(bool enable) noexcept
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button != nullptr) {
        emit optionToggled(enable, button->property(::UserData));
    }
}
