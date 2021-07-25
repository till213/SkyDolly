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
#include "KMLStyleExport.h"
#include "ui_KMLExportDialog.h"

namespace {
    // in AARRGGBB format
    // https://designs.ai/colors/color-wheel
    // http://khroma.co/generator/
    // http://colormind.io/
    const QRgb Opaque = 0xff000000;

    // "Tetraedic" colors
    constexpr QRgb DefaultJetStartColor = Opaque | 0xde7b51;
    constexpr QRgb DefaultJetEndColor = Opaque | 0x6f3d28;
    constexpr QRgb DefaultTurbopropStartColor = Opaque | 0x6ade4b;
    constexpr QRgb DefaultTurbopropEndColor = Opaque | 0x356f25;
    constexpr QRgb DefaultPistonStartColor = Opaque | 0x4bb3de;
    constexpr QRgb DefaultPistonEndColor = Opaque | 0x255a6f;
    constexpr QRgb DefaultAllStartColor = Opaque | 0xc561de;
    constexpr QRgb DefaultAllEndColor = Opaque | 0x63316f;

    constexpr KMLStyleExport::ColorStyle DefaultColorStyle = KMLStyleExport::ColorStyle::ColorRampPerEngineType;

    constexpr int DefaultNofColorsPerRamp = 8;
    constexpr float DefaultLineWidth = 3.0f;

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
}

class KMLExportDialogPrivate
{
public:
    KMLExportDialogPrivate() noexcept
        : exportButton(nullptr),
          colorButtonGroup(std::make_unique<QButtonGroup>()),
          jetStartColor(DefaultJetStartColor),
          jetEndColor(DefaultJetEndColor),
          turbopropStartColor(DefaultTurbopropStartColor),
          turbopropEndColor(DefaultTurbopropEndColor),
          pistonStartColor(DefaultPistonStartColor),
          pistonEndColor(DefaultPistonEndColor),
          allStartColor(DefaultAllStartColor),
          allEndColor(DefaultAllEndColor)
    {}

    QPushButton *exportButton;
    std::unique_ptr<QButtonGroup> colorButtonGroup;
    Unit unit;

    QColor jetStartColor;
    QColor jetEndColor;
    QColor turbopropStartColor;
    QColor turbopropEndColor;
    QColor pistonStartColor;
    QColor pistonEndColor;
    QColor allStartColor;
    QColor allEndColor;
};

// PUBLIC

KMLExportDialog::KMLExportDialog(QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::KMLExportDialog),
      d(std::make_unique<KMLExportDialogPrivate>())
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KMLExportDialog::~KMLExportDialog() noexcept
{
    delete ui;
}

QString KMLExportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

KMLExportDialog::ResamplingPeriod KMLExportDialog::getSelectedResamplingPeriod() const noexcept
{
    return static_cast<KMLExportDialog::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
}

bool KMLExportDialog::doOpenExportedFile() const noexcept
{
    return ui->openExportCheckBox->isChecked();
}

KMLStyleExport::StyleParameter KMLExportDialog::getStyleParameters() const noexcept
{
    KMLStyleExport::StyleParameter p;

    p.colorStyle = static_cast<KMLStyleExport::ColorStyle>(ui->colorStyleComboBox->currentIndex());
    if (p.colorStyle == KMLStyleExport::ColorStyle::ColorRampPerEngineType || p.colorStyle == KMLStyleExport::ColorStyle::OneColorPerEngineType) {
        p.categoryColors[SimType::EngineType::Jet].first = d->jetStartColor.rgba();
        p.categoryColors[SimType::EngineType::Jet].second = d->jetEndColor.rgba();
        p.categoryColors[SimType::EngineType::Turboprop].first = d->turbopropStartColor.rgba();
        p.categoryColors[SimType::EngineType::Turboprop].second = d->turbopropEndColor.rgba();
        p.categoryColors[SimType::EngineType::Piston].first = d->pistonStartColor.rgba();
        p.categoryColors[SimType::EngineType::Piston].second = d->pistonEndColor.rgba();
    }
    p.categoryColors[SimType::EngineType::All].first = d->allStartColor.rgba();
    p.categoryColors[SimType::EngineType::All].second = d->allEndColor.rgba();

    if (p.colorStyle == KMLStyleExport::ColorStyle::ColorRamp || p.colorStyle == KMLStyleExport::ColorStyle::ColorRampPerEngineType) {
        p.nofColorsPerRamp = DefaultNofColorsPerRamp;
    } else {
        p.nofColorsPerRamp = 1;
    }
    p.lineWidth = DefaultLineWidth;

    return p;
}

// PRIVATE

void KMLExportDialog::initUi() noexcept
{
    d->exportButton = ui->buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // File path
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(Export::suggestFilePath(FileSuffix)));

    // Resampling
    ui->resamplingComboBox->addItem(QString("1/10 Hz") % " (" % tr("less data, less accuracy") % ")", Enum::toUnderlyingType(ResamplingPeriod::ATenthHz));
    ui->resamplingComboBox->addItem("1/5 Hz", Enum::toUnderlyingType(ResamplingPeriod::AFifthHz));
    ui->resamplingComboBox->addItem(QString("1 Hz") % " (" % tr("good accuracy") % ")", Enum::toUnderlyingType(ResamplingPeriod::OneHz));
    ui->resamplingComboBox->addItem("2 Hz", Enum::toUnderlyingType(ResamplingPeriod::TwoHz));
    ui->resamplingComboBox->addItem("5 Hz", Enum::toUnderlyingType(ResamplingPeriod::FiveHz));
    ui->resamplingComboBox->addItem("10 Hz", Enum::toUnderlyingType(ResamplingPeriod::TenHz));
    ui->resamplingComboBox->addItem(tr("Original data (performance critical)"), Enum::toUnderlyingType(ResamplingPeriod::Original));

    ui->resamplingComboBox->setCurrentIndex(2);

    initColorUi();
}

void KMLExportDialog::initColorUi() noexcept
{
    ui->colorStyleComboBox->insertItem(Enum::toUnderlyingType(KMLStyleExport::ColorStyle::OneColor), tr("One color"));
    ui->colorStyleComboBox->insertItem(Enum::toUnderlyingType(KMLStyleExport::ColorStyle::OneColorPerEngineType), tr("One color per engine type"));
    ui->colorStyleComboBox->insertItem(Enum::toUnderlyingType(KMLStyleExport::ColorStyle::ColorRamp), tr("Color ramp"));
    ui->colorStyleComboBox->insertItem(Enum::toUnderlyingType(KMLStyleExport::ColorStyle::ColorRampPerEngineType), tr("Color ramp per engine type"));
    ui->colorStyleComboBox->setCurrentIndex(Enum::toUnderlyingType(DefaultColorStyle));

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
    ResamplingPeriod resamplingPeriod = static_cast<ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    qint64 samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != ResamplingPeriod::Original) {
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
    switch (static_cast<KMLStyleExport::ColorStyle>(ui->colorStyleComboBox->currentIndex())) {
    case KMLStyleExport::ColorStyle::OneColor:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLStyleExport::ColorStyle::OneColorPerEngineType:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(false);
        ui->jetStartColorToolButton->setEnabled(true);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(true);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(true);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLStyleExport::ColorStyle::ColorRamp:
        ui->allStartColorToolButton->setEnabled(true);
        ui->allEndColorToolButton->setEnabled(true);
        ui->jetStartColorToolButton->setEnabled(false);
        ui->jetEndColorToolButton->setEnabled(false);
        ui->turbopropStartColorToolButton->setEnabled(false);
        ui->turbopropEndColorToolButton->setEnabled(false);
        ui->pistonStartColorToolButton->setEnabled(false);
        ui->pistonEndColorToolButton->setEnabled(false);
        break;
    case KMLStyleExport::ColorStyle::ColorRampPerEngineType:
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
        css = "background-color: " % d->allStartColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->allStartColorToolButton->setStyleSheet(css);
    if (ui->allEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->allEndColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->allEndColorToolButton->setStyleSheet(css);

    if (ui->jetStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->jetStartColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->jetStartColorToolButton->setStyleSheet(css);
    if (ui->jetEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->jetEndColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->jetEndColorToolButton->setStyleSheet(css);

    if (ui->turbopropStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->turbopropStartColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->turbopropStartColorToolButton->setStyleSheet(css);
    if (ui->turbopropEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->turbopropEndColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->turbopropEndColorToolButton->setStyleSheet(css);

    if (ui->pistonStartColorToolButton->isEnabled()) {
        css = "background-color: " % d->pistonStartColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->pistonStartColorToolButton->setStyleSheet(css);
    if (ui->pistonEndColorToolButton->isEnabled()) {
        css = "background-color: " % d->pistonEndColor.name() % ";";
    } else {
        css = "background-color: #aaa;";
    }
    ui->pistonEndColorToolButton->setStyleSheet(css);
}

void KMLExportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &KMLExportDialog::updateUi);
    connect(d->colorButtonGroup.get(), &QButtonGroup::idClicked,
            this, &KMLExportDialog::selectColor);
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

    updateInfoUi();
    updateColorUi();
}

void KMLExportDialog::selectColor(int id) noexcept
{
    QColor initialColor;
    switch (static_cast<ColorButton>(id)) {
    case ColorButton::JetStartColor:
        initialColor = d->jetStartColor;
        break;
    case ColorButton::JetEndColor:
        initialColor = d->jetEndColor;
        break;
    case ColorButton::TurbopropStartColor:
        initialColor = d->turbopropStartColor;
        break;
    case ColorButton::TurbopropEndColor:
        initialColor = d->turbopropEndColor;
        break;
    case ColorButton::PistonStartColor:
        initialColor = d->pistonStartColor;
        break;
    case ColorButton::PistonEndColor:
        initialColor = d->pistonEndColor;
        break;
    case ColorButton::AllStartColor:
        initialColor = d->allStartColor;
        break;
    case ColorButton::AllEndColor:
        initialColor = d->allEndColor;
        break;
    default:
        break;
    }

    QColor color = QColorDialog::getColor(initialColor, this);
    if (color.isValid()) {
        switch (static_cast<ColorButton>(id)) {
        case ColorButton::JetStartColor:
            d->jetStartColor = color;
            d->jetEndColor = color.darker();
            break;
        case ColorButton::JetEndColor:
            d->jetEndColor = color;
            break;
        case ColorButton::TurbopropStartColor:
            d->turbopropStartColor = color;
            d->turbopropEndColor = color.darker();
            break;
        case ColorButton::TurbopropEndColor:
            d->turbopropEndColor = color;
            break;
        case ColorButton::PistonStartColor:
            d->pistonStartColor = color;
            d->pistonEndColor = color.darker();
            break;
        case ColorButton::PistonEndColor:
            d->pistonEndColor = color;
            break;
        case ColorButton::AllStartColor:
            d->allStartColor = color;
            d->allEndColor = color.darker();
            break;
        case ColorButton::AllEndColor:
            d->allEndColor= color;
            break;
        default:
            break;
        }
        updateColorUi();
    }
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
    updateInfoUi();
}

void KMLExportDialog::on_colorStyleComboBox_activated(int index) noexcept
{
    updateColorUi();
}
