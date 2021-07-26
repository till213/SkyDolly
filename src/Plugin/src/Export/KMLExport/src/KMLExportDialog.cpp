/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <utility>
#include <limits>

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QButtonGroup>
#include <QColorDialog>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../src/Export.h"
#include "KMLExportDialog.h"
#include "KMLExportSettings.h"
#include "ui_KMLExportDialog.h"

namespace {

    enum class ColorButton {
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

class KMLExportDialogPrivate
{
public:
    KMLExportDialogPrivate(KMLExportSettings &theExportSettings) noexcept
        : exportSettings(theExportSettings),
          exportButton(nullptr),
          colorButtonGroup(std::make_unique<QButtonGroup>())
    {}

    KMLExportSettings &exportSettings;
    QPushButton *exportButton;
    std::unique_ptr<QButtonGroup> colorButtonGroup;
    Unit unit;
};

// PUBLIC

KMLExportDialog::KMLExportDialog(KMLExportSettings &exportSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::KMLExportDialog),
      d(std::make_unique<KMLExportDialogPrivate>(exportSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KMLExportDialog::~KMLExportDialog() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("KMLExportDialog::~KMLExportDialog: DELETED");
#endif
}

QString KMLExportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

bool KMLExportDialog::doOpenExportedFile() const noexcept
{
    return ui->openExportCheckBox->isChecked();
}

// PRIVATE

void KMLExportDialog::initUi() noexcept
{
    d->exportButton = ui->buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // File path
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(Export::suggestFilePath(FileSuffix)));

    // Resampling
    ui->resamplingComboBox->addItem(QString("1/10 Hz") % " (" % tr("less data, less accuracy") % ")", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::ATenthHz));
    ui->resamplingComboBox->addItem("1/5 Hz", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::AFifthHz));
    ui->resamplingComboBox->addItem(QString("1 Hz") % " (" % tr("good accuracy") % ")", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::OneHz));
    ui->resamplingComboBox->addItem("2 Hz", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::TwoHz));
    ui->resamplingComboBox->addItem("5 Hz", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::FiveHz));
    ui->resamplingComboBox->addItem("10 Hz", Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::TenHz));
    ui->resamplingComboBox->addItem(tr("Original data (performance critical)"), Enum::toUnderlyingType(KMLExportSettings::ResamplingPeriod::Original));

    initColorUi();
}

void KMLExportDialog::initColorUi() noexcept
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

void KMLExportDialog::updateInfoUi() noexcept
{
    QString infoText;
    KMLExportSettings::ResamplingPeriod resamplingPeriod = static_cast<KMLExportSettings::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    qint64 samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != KMLExportSettings::ResamplingPeriod::Original) {
        infoText = tr("The position data is resampled every %1 milliseconds, resulting in approximately %2 exported positions in total.")
                      .arg(d->unit.formatNumber(Enum::toUnderlyingType(resamplingPeriod), 0), d->unit.formatNumber(samplePoints, 0));
    } else {
        infoText = tr("WARNING: exporting the original position data may result in too large KML files. The KML viewer performance may "
                      "drastically slow down, or the exported data may not even be displayed at all.\n\nIn total %1 positions will be exported.")
                      .arg(d->unit.formatNumber(samplePoints, 0));
    }
    ui->infoLabel->setText(infoText);
}

void KMLExportDialog::updateColorUi() noexcept
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

void KMLExportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &KMLExportDialog::updateUi);
    connect(d->colorButtonGroup.get(), &QButtonGroup::idClicked,
            this, &KMLExportDialog::selectColor);

    QPushButton *resetButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, KMLExportDialog::restoreDefaults);

}

qint64 KMLExportDialog::estimateNofSamplePoints() noexcept
{
    Flight &flight = Logbook::getInstance().getInstance().getCurrentFlight();
    qint64 nofSamplePoints = 0;
    const qint64 period = ui->resamplingComboBox->currentData().toInt();
    if (period != 0) {
        for (const auto &aircraft : flight) {
            qint64 duration = aircraft->getDurationMSec();
            nofSamplePoints += qRound(static_cast<double>(duration) / static_cast<double>(period));
        }
    } else {
        // Count the actual position sample points
        for (const auto &aircraft : flight) {
            nofSamplePoints += aircraft->getPositionConst().count();
        }
    }
    return nofSamplePoints;
}

// PRIVATE SLOTS

void KMLExportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFileInfo fileInfo(filePath);
    QFile file(fileInfo.absolutePath());
    d->exportButton->setEnabled(file.exists());

    int currentIndex = 0;
    while (currentIndex < ui->resamplingComboBox->count() &&
           static_cast<KMLExportSettings::ResamplingPeriod>(ui->resamplingComboBox->itemData(currentIndex).toInt()) != d->exportSettings.resamplingPeriod) {
        ++currentIndex;
    }
    ui->resamplingComboBox->setCurrentIndex(currentIndex);

    updateInfoUi();
    updateColorUi();
}

void KMLExportDialog::selectColor(int id) noexcept
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
        updateColorUi();
    }
}

void KMLExportDialog::restoreDefaults() noexcept
{
    d->exportSettings.restoreDefaults();
    updateUi();
}

void KMLExportDialog::on_fileSelectionPushButton_clicked() noexcept
{
    const QString filePath = QFileDialog::getSaveFileName(this, QCoreApplication::translate("KMLExportDialog", "Export KML"), ui->filePathLineEdit->text(), QString("*.kml"));
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(filePath);
    }
    updateUi();
}

void KMLExportDialog::on_resamplingComboBox_activated(int index) noexcept
{
    Q_UNUSED(index)
    d->exportSettings.resamplingPeriod = static_cast<KMLExportSettings::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    updateInfoUi();
}

void KMLExportDialog::on_colorStyleComboBox_activated(int index) noexcept
{
    Q_UNUSED(index)
    d->exportSettings.colorStyle = static_cast<KMLExportSettings::ColorStyle>(ui->colorStyleComboBox->currentData().toInt());
    if (d->exportSettings.colorStyle == KMLExportSettings::ColorStyle::ColorRamp || d->exportSettings.colorStyle == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
        d->exportSettings.nofColorsPerRamp = KMLExportSettings::DefaultNofColorsPerRamp;
    } else {
        d->exportSettings.nofColorsPerRamp = 1;
    }

    updateColorUi();
}
