/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <cstdint>

#include <QComboBox>
#include <QButtonGroup>
#include <QString>
#include <QStringBuilder>
#include <QColorDialog>

#include <Kernel/Enum.h>
#include "KmlExportOptionWidget.h"
#include "KmlExportSettings.h"
#include "ui_KmlExportOptionWidget.h"

namespace
{
    enum struct ColorButton: std::uint8_t {
        JetStartColor,
        JetEndColor,
        TurbopropStartColor,
        TurbopropEndColor,
        PistonStartColor,
        PistonEndColor,
        AllStartColor,
        AllEndColor
    };

    constexpr const char *DisabledColor {"#aaa"};
}

struct KmlExportOptionWidgetPrivate
{
    KmlExportOptionWidgetPrivate(KmlExportSettings &pluginSettings) noexcept
        : colorButtonGroup(std::make_unique<QButtonGroup>()),
          pluginSettings {pluginSettings}
    {}

    std::unique_ptr<QButtonGroup> colorButtonGroup;
    KmlExportSettings &pluginSettings;
};

// PUBLIC

KmlExportOptionWidget::KmlExportOptionWidget(KmlExportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::KmlExportOptionWidget>()},
      d {std::make_unique<KmlExportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KmlExportOptionWidget::~KmlExportOptionWidget() = default;

// PRIVATE

void KmlExportOptionWidget::frenchConnection() noexcept
{
    connect(&d->pluginSettings, &KmlExportSettings::changed,
            this, &KmlExportOptionWidget::updateUi);
    connect(d->colorButtonGroup.get(), &QButtonGroup::idClicked,
            this, &KmlExportOptionWidget::selectColor);
    connect(ui->colorStyleComboBox, &QComboBox::currentIndexChanged,
            this, &KmlExportOptionWidget::onColorStyleChanged);
}

void KmlExportOptionWidget::initUi() noexcept
{
    ui->colorStyleComboBox->addItem(tr("One color"), Enum::underly(KmlExportSettings::ColorStyle::OneColor));
    ui->colorStyleComboBox->addItem(tr("One color per engine type"), Enum::underly(KmlExportSettings::ColorStyle::OneColorPerEngineType));
    ui->colorStyleComboBox->addItem(tr("Color ramp"), Enum::underly(KmlExportSettings::ColorStyle::ColorRamp));
    ui->colorStyleComboBox->addItem(tr("Color ramp per engine type"), Enum::underly(KmlExportSettings::ColorStyle::ColorRampPerEngineType));

    d->colorButtonGroup->addButton(ui->allStartColorToolButton, Enum::underly(ColorButton::AllStartColor));
    d->colorButtonGroup->addButton(ui->allEndColorToolButton, Enum::underly(ColorButton::AllEndColor));
    d->colorButtonGroup->addButton(ui->jetStartColorToolButton, Enum::underly(ColorButton::JetStartColor));
    d->colorButtonGroup->addButton(ui->jetEndColorToolButton, Enum::underly(ColorButton::JetEndColor));
    d->colorButtonGroup->addButton(ui->turbopropStartColorToolButton, Enum::underly(ColorButton::TurbopropStartColor));
    d->colorButtonGroup->addButton(ui->turbopropEndColorToolButton, Enum::underly(ColorButton::TurbopropEndColor));
    d->colorButtonGroup->addButton(ui->pistonStartColorToolButton, Enum::underly(ColorButton::PistonStartColor));
    d->colorButtonGroup->addButton(ui->pistonEndColorToolButton, Enum::underly(ColorButton::PistonEndColor));
}

// PRIVATE SLOTS

void KmlExportOptionWidget::updateUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->colorStyleComboBox->count() &&
           static_cast<KmlExportSettings::ColorStyle>(ui->colorStyleComboBox->itemData(currentIndex).toInt()) != d->pluginSettings.getColorStyle()) {
        ++currentIndex;
    }
    ui->colorStyleComboBox->setCurrentIndex(currentIndex);

    switch (d->pluginSettings.getColorStyle()) {
    case KmlExportSettings::ColorStyle::OneColor:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KmlExportSettings::ColorStyle::OneColorPerEngineType:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(true);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(true);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(true);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KmlExportSettings::ColorStyle::ColorRamp:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(true);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KmlExportSettings::ColorStyle::ColorRampPerEngineType:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(true);
        ui->jetStartColorToolButton->setEnabled(true);
        ui->jetEndColorToolButton->setEnabled(true);
        ui->turbopropStartColorToolButton->setEnabled(true);
        ui->turbopropEndColorToolButton->setEnabled(true);
        ui->pistonStartColorToolButton->setEnabled(true);
        ui->pistonEndColorToolButton->setEnabled(true);
        break;
    }

    QString css;
    if (ui->allStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getAllStartColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->allStartColorToolButton->setStyleSheet(css);
    if (ui->allEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getAllEndColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->allEndColorToolButton->setStyleSheet(css);

    if (ui->jetStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getJetStartColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->jetStartColorToolButton->setStyleSheet(css);
    if (ui->jetEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getJetEndColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->jetEndColorToolButton->setStyleSheet(css);

    if (ui->turbopropStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getTurbopropStartColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->turbopropStartColorToolButton->setStyleSheet(css);
    if (ui->turbopropEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getTurbopropEndColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->turbopropEndColorToolButton->setStyleSheet(css);

    if (ui->pistonStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getPistonStartColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->pistonStartColorToolButton->setStyleSheet(css);
    if (ui->pistonEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->pluginSettings.getPistonEndColor().name() % ";";
    } else {
        css = QStringLiteral("background-color: ") % DisabledColor % ";";
    }
    ui->pistonEndColorToolButton->setStyleSheet(css);
}

void KmlExportOptionWidget::selectColor(int id) noexcept
{
    QColor initialColor;
    switch (static_cast<ColorButton>(id)) {
    case ColorButton::JetStartColor:
        initialColor = d->pluginSettings.getJetStartColor();
        break;
    case ColorButton::JetEndColor:
        initialColor = d->pluginSettings.getJetEndColor();
        break;
    case ColorButton::TurbopropStartColor:
        initialColor = d->pluginSettings.getTurbopropStartColor();
        break;
    case ColorButton::TurbopropEndColor:
        initialColor = d->pluginSettings.getTurbopropEndColor();
        break;
    case ColorButton::PistonStartColor:
        initialColor = d->pluginSettings.getPistonStartColor();
        break;
    case ColorButton::PistonEndColor:
        initialColor = d->pluginSettings.getPistonEndColor();
        break;
    case ColorButton::AllStartColor:
        initialColor = d->pluginSettings.getAllStartColor();
        break;
    case ColorButton::AllEndColor:
        initialColor = d->pluginSettings.getAllEndColor();
        break;
    }

    QColor color = QColorDialog::getColor(initialColor, this);
    if (color.isValid()) {
        switch (static_cast<ColorButton>(id)) {
        case ColorButton::JetStartColor:
            d->pluginSettings.setJetStartColor(color);
            d->pluginSettings.setJetEndColor(color.darker());
            break;
        case ColorButton::JetEndColor:
            d->pluginSettings.setJetEndColor(color);
            break;
        case ColorButton::TurbopropStartColor:
            d->pluginSettings.setTurbopropStartColor(color);
            d->pluginSettings.setTurbopropEndColor(color.darker());
            break;
        case ColorButton::TurbopropEndColor:
            d->pluginSettings.setTurbopropEndColor(color);
            break;
        case ColorButton::PistonStartColor:
            d->pluginSettings.setPistonStartColor(color);
            d->pluginSettings.setPistonEndColor(color.darker());
            break;
        case ColorButton::PistonEndColor:
            d->pluginSettings.setPistonEndColor(color);
            break;
        case ColorButton::AllStartColor:
            d->pluginSettings.setAllStartColor(color);
            d->pluginSettings.setAllEndColor(color.darker());
            break;
        case ColorButton::AllEndColor:
            d->pluginSettings.setAllEndColor(color);
            break;
        }
    }
}

void KmlExportOptionWidget::onColorStyleChanged() noexcept
{
    const KmlExportSettings::ColorStyle colorStyle = static_cast<KmlExportSettings::ColorStyle>(ui->colorStyleComboBox->currentData().toInt());
    d->pluginSettings.setColorStyle(colorStyle);
}
