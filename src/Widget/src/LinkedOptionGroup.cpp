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

void LinkedOptionGroup::addOption(const QString &name, const QVariant &optionValue, const QString &toolTip) noexcept
{
    static QLatin1String singleButtonCss {"QPushButton {border-radius: 6px;}"};
    static QLatin1String firstButtonCss {"QPushButton {border-top-left-radius: 6px; border-bottom-left-radius: 6px;}"};
    static QLatin1String lastButtonCss {"QPushButton {border-top-right-radius: 6px; border-bottom-right-radius: 6px;}"};

    QPushButton *button = new QPushButton(name, this);
    button->setCheckable(true);
    button->setProperty(::OptionValue, optionValue);
    button->setToolTip(toolTip);
    d->buttons.push_back(button);
    std::size_t buttonCount = d->buttons.size();
    if (buttonCount == 1) {
        button->setStyleSheet(singleButtonCss);
    } else if (buttonCount == 2) {
        button->setStyleSheet(lastButtonCss);
        // Also update the first button
        d->buttons.front()->setStyleSheet(firstButtonCss);
    } else {
        button->setStyleSheet(lastButtonCss);
        // Also update the second to last button
        d->buttons[buttonCount - 2]->setStyleSheet({});
    }

    d->layout->addWidget(button);
    connect(button, &QPushButton::toggled,
            this, &LinkedOptionGroup::onButtonToggled);
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
    static QString normalButtonCss {"QPushButton {margin: 0; padding: 4px; border: 0px; background-color: " % Platform::getButtonBGColor().name() % ";} "};
    static QString checkedButtonCss {"QPushButton:checked { background-color: " % Platform::getActiveButtonBGColor().name() % "; color: white;}"};

    setStyleSheet(normalButtonCss % checkedButtonCss);

    d->layout = new QHBoxLayout(this);
    d->layout->setSpacing(0);

    setLayout(d->layout);
}

// PRIVATE SLOTS

void LinkedOptionGroup::onButtonToggled(bool enable) noexcept
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button != nullptr) {
        emit optionToggled(enable, button->property(::OptionValue));
    }
}
