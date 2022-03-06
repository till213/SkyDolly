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
#include <QComboBox>
#include <QButtonGroup>
#include <QString>
#include <QStringBuilder>
#include <QColorDialog>

#include "../../../../../Kernel/src/Enum.h"
#include "KMLExportOptionWidget.h"
#include "KMLExportSettings.h"
#include "ui_KMLExportOptionWidget.h"

namespace
{
    enum struct ColorButton {
        JetStartColor,
        JetEndColor,
        TurbopropStartColor,
        TurbopropEndColor,
        PistonStartColor,
        PistonEndColor,
        AllStartColor,
        AllEndColor
    };

    constexpr char DisabledColor[] = "#aaa";
}

class KMLExportOptionWidgetPrivate
{
public:
    KMLExportOptionWidgetPrivate(KMLExportSettings &theExportSettings) noexcept
        : exportSettings(theExportSettings),
          colorButtonGroup(std::make_unique<QButtonGroup>())
    {}

    std::unique_ptr<QButtonGroup> colorButtonGroup;
    KMLExportSettings &exportSettings;
};

// PUBLIC

KMLExportOptionWidget::KMLExportOptionWidget(KMLExportSettings &exportSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::KMLExportOptionWidget),
      d(std::make_unique<KMLExportOptionWidgetPrivate>(exportSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("KMLExportOptionWidget::KMLExportOptionWidget: CREATED");
#endif
}

KMLExportOptionWidget::~KMLExportOptionWidget() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("KMLExportOptionWidget::~KMLExportOptionWidget: DELETED");
#endif
}

// PRIVATE

void KMLExportOptionWidget::frenchConnection() noexcept
{
    connect(&d->exportSettings, &KMLExportSettings::changed,
            this, &KMLExportOptionWidget::updateUi);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        connect(d->colorButtonGroup.get(), QOverload<int>::of(&QButtonGroup::buttonClicked),
                this, &KMLExportOptionWidget::selectColor);
#else
        connect(d->colorButtonGroup.get(), &QButtonGroup::idClicked,
                this, &KMLExportOptionWidget::selectColor);
#endif
}

void KMLExportOptionWidget::initUi() noexcept
{
    ui->colorStyleComboBox->addItem(tr("One color"), Enum::toUnderlyingType(KMLExportSettings::ColorStyle::OneColor));
    ui->colorStyleComboBox->addItem(tr("One color per engine type"), Enum::toUnderlyingType(KMLExportSettings::ColorStyle::OneColorPerEngineType));
    ui->colorStyleComboBox->addItem(tr("Color ramp"), Enum::toUnderlyingType(KMLExportSettings::ColorStyle::ColorRamp));
    ui->colorStyleComboBox->addItem(tr("Color ramp per engine type"), Enum::toUnderlyingType(KMLExportSettings::ColorStyle::ColorRampPerEngineType));

    d->colorButtonGroup->addButton(ui->allStartColorToolButton, Enum::toUnderlyingType(ColorButton::AllStartColor));
    d->colorButtonGroup->addButton(ui->allEndColorToolButton, Enum::toUnderlyingType(ColorButton::AllEndColor));
    d->colorButtonGroup->addButton(ui->jetStartColorToolButton, Enum::toUnderlyingType(ColorButton::JetStartColor));
    d->colorButtonGroup->addButton(ui->jetEndColorToolButton, Enum::toUnderlyingType(ColorButton::JetEndColor));
    d->colorButtonGroup->addButton(ui->turbopropStartColorToolButton, Enum::toUnderlyingType(ColorButton::TurbopropStartColor));
    d->colorButtonGroup->addButton(ui->turbopropEndColorToolButton, Enum::toUnderlyingType(ColorButton::TurbopropEndColor));
    d->colorButtonGroup->addButton(ui->pistonStartColorToolButton, Enum::toUnderlyingType(ColorButton::PistonStartColor));
    d->colorButtonGroup->addButton(ui->pistonEndColorToolButton, Enum::toUnderlyingType(ColorButton::PistonEndColor));
}

// PRIVATE SLOTS

void KMLExportOptionWidget::updateUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->colorStyleComboBox->count() &&
           static_cast<KMLExportSettings::ColorStyle>(ui->colorStyleComboBox->itemData(currentIndex).toInt()) != d->exportSettings.colorStyle) {
        ++currentIndex;
    }
    ui->colorStyleComboBox->setCurrentIndex(currentIndex);

    switch (static_cast<KMLExportSettings::ColorStyle>(ui->colorStyleComboBox->currentIndex())) {
    case KMLExportSettings::ColorStyle::OneColor:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLExportSettings::ColorStyle::OneColorPerEngineType:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(true);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(true);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(true);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLExportSettings::ColorStyle::ColorRamp:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(true);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLExportSettings::ColorStyle::ColorRampPerEngineType:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(true);
        ui->jetStartColorToolButton->setEnabled(true);
        ui->jetEndColorToolButton->setEnabled(true);
        ui->turbopropStartColorToolButton->setEnabled(true);
        ui->turbopropEndColorToolButton->setEnabled(true);
        ui->pistonStartColorToolButton->setEnabled(true);
        ui->pistonEndColorToolButton->setEnabled(true);
        break;
    default:
        break;
    }

    QString css;
    if (ui->allStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.allStartColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->allStartColorToolButton->setStyleSheet(css);
    if (ui->allEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.allEndColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->allEndColorToolButton->setStyleSheet(css);

    if (ui->jetStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.jetStartColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->jetStartColorToolButton->setStyleSheet(css);
    if (ui->jetEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.jetEndColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->jetEndColorToolButton->setStyleSheet(css);

    if (ui->turbopropStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.turbopropStartColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->turbopropStartColorToolButton->setStyleSheet(css);
    if (ui->turbopropEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.turbopropEndColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->turbopropEndColorToolButton->setStyleSheet(css);

    if (ui->pistonStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.pistonStartColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->pistonStartColorToolButton->setStyleSheet(css);
    if (ui->pistonEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->exportSettings.pistonEndColor.name() % ";";
    } else {
        css = QString("background-color: ") % DisabledColor % ";";
    }
    ui->pistonEndColorToolButton->setStyleSheet(css);
}

void KMLExportOptionWidget::selectColor(int id) noexcept
{
    QColor initialColor;
    switch (static_cast<ColorButton>(id)) {
    case ColorButton::JetStartColor:
        initialColor = d->exportSettings.jetStartColor;
        break;
    case ColorButton::JetEndColor:
        initialColor = d->exportSettings.jetEndColor;
        break;
    case ColorButton::TurbopropStartColor:
        initialColor = d->exportSettings.turbopropStartColor;
        break;
    case ColorButton::TurbopropEndColor:
        initialColor = d->exportSettings.turbopropEndColor;
        break;
    case ColorButton::PistonStartColor:
        initialColor = d->exportSettings.pistonStartColor;
        break;
    case ColorButton::PistonEndColor:
        initialColor = d->exportSettings.pistonEndColor;
        break;
    case ColorButton::AllStartColor:
        initialColor = d->exportSettings.allStartColor;
        break;
    case ColorButton::AllEndColor:
        initialColor = d->exportSettings.allEndColor;
        break;
    default:
        break;
    }

    QColor color = QColorDialog::getColor(initialColor, this);
    if (color.isValid()) {
        switch (static_cast<ColorButton>(id)) {
        case ColorButton::JetStartColor:
            d->exportSettings.jetStartColor = color;
            d->exportSettings.jetEndColor = color.darker();
            break;
        case ColorButton::JetEndColor:
            d->exportSettings.jetEndColor = color;
            break;
        case ColorButton::TurbopropStartColor:
            d->exportSettings.turbopropStartColor = color;
            d->exportSettings.turbopropEndColor = color.darker();
            break;
        case ColorButton::TurbopropEndColor:
            d->exportSettings.turbopropEndColor = color;
            break;
        case ColorButton::PistonStartColor:
            d->exportSettings.pistonStartColor = color;
            d->exportSettings.pistonEndColor = color.darker();
            break;
        case ColorButton::PistonEndColor:
            d->exportSettings.pistonEndColor = color;
            break;
        case ColorButton::AllStartColor:
            d->exportSettings.allStartColor = color;
            d->exportSettings.allEndColor = color.darker();
            break;
        case ColorButton::AllEndColor:
            d->exportSettings.allEndColor = color;
            break;
        default:
            break;
        }
        updateUi();
    }
}

void KMLExportOptionWidget::onColorStyleChanged([[maybe_unused]] int index) noexcept
{
    d->exportSettings.colorStyle = static_cast<KMLExportSettings::ColorStyle>(ui->colorStyleComboBox->currentData().toInt());
    if (d->exportSettings.colorStyle == KMLExportSettings::ColorStyle::ColorRamp || d->exportSettings.colorStyle == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
        d->exportSettings.nofColorsPerRamp = KMLExportSettings::DefaultNofColorsPerRamp;
    } else {
        d->exportSettings.nofColorsPerRamp = 1;
    }

    updateUi();
}