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
#include <vector>

#include <QVariant>
#include <QString>
#include <QStringBuilder>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "Platform.h"
#include "LinkedOptionGroup.h"

namespace
{
    constexpr const char *OptionValue {"OptionValue"};
}

struct LinkedOptionGroupPrivate
{
    QHBoxLayout *layout {nullptr};
    std::vector<QPushButton *> buttons;
};

// PUBLIC

QPushButton *createButton(QString const &name,
                          bool checked,
                          QString const &sheet = QString())
{
    auto pushButton = new QPushButton(name);
    pushButton->setCheckable(true);
    pushButton->setChecked(checked);
    if (!sheet.isEmpty()) {
        pushButton->setStyleSheet(sheet);
    }
    return pushButton;
}

LinkedOptionGroup::LinkedOptionGroup(QWidget *parent) noexcept
    : QWidget {parent},
      d {std::make_unique<LinkedOptionGroupPrivate>()}
{
    initUi();
}

LinkedOptionGroup::~LinkedOptionGroup() = default;

void LinkedOptionGroup::addOption(const QString &name, const QVariant &optionValue, const QString &toolTip) noexcept
{
    static const QLatin1String SingleButtonCss {"QPushButton {border-radius: 6px;}"};
    static const QLatin1String FirstButtonCss {"QPushButton {border-top-left-radius: 6px; border-bottom-left-radius: 6px;}"};
    static const QLatin1String LastButtonCss {"QPushButton {border-top-right-radius: 6px; border-bottom-right-radius: 6px;}"};

    auto pushButton = new QPushButton(name, this);
    pushButton->setCheckable(true);
    pushButton->setProperty(::OptionValue, optionValue);
    pushButton->setToolTip(toolTip);
    d->buttons.push_back(pushButton);
    std::size_t buttonCount = d->buttons.size();
    if (buttonCount == 1) {
        pushButton->setStyleSheet(SingleButtonCss);
    } else if (buttonCount == 2) {
        pushButton->setStyleSheet(LastButtonCss);
        // Also update the first button
        d->buttons.front()->setStyleSheet(FirstButtonCss);
    } else {
        pushButton->setStyleSheet(LastButtonCss);
        // Also update the second to last button
        d->buttons[buttonCount - 2]->setStyleSheet({});
    }

    d->layout->addWidget(pushButton);
    connect(pushButton, &QPushButton::toggled,
            this, &LinkedOptionGroup::onButtonToggled);
}

void LinkedOptionGroup::clearOptions() noexcept
{
    for (auto button : d->buttons) {
        button->setChecked(false);
    }
}

void LinkedOptionGroup::setOptionEnabled(const QVariant &optionValue, bool enable) noexcept
{
    for (QPushButton *button : d->buttons) {
        if (button->property(::OptionValue) == optionValue) {
            button->setChecked(enable);
            break;
        }
    }
}

// PRIVATE

void LinkedOptionGroup::initUi() noexcept
{
    static const QString NormalButtonCss {"QPushButton {margin: 0; padding: 4px; border: 0px; background-color: " % Platform::getButtonBGColor().name() % ";} "};
    static const QString CheckedButtonCss {"QPushButton:checked { background-color: " % Platform::getActiveButtonBGColor().name() % "; color: white;}"};

    setStyleSheet(NormalButtonCss % CheckedButtonCss);

    d->layout = new QHBoxLayout(this);
    d->layout->setSpacing(0);

    setLayout(d->layout);
}

// PRIVATE SLOTS

void LinkedOptionGroup::onButtonToggled(bool enable) noexcept
{
    auto button = qobject_cast<QPushButton *>(sender());
    if (button != nullptr) {
        emit optionToggled(button->property(::OptionValue), enable);
    }
}
