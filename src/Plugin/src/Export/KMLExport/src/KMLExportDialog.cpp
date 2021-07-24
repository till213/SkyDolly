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
#include <limits>

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
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

    // Cyan
    constexpr QRgb DefaultJetStartColor = Opaque | 0x00d9ff;
    constexpr QRgb DefaultJetEndColor = Opaque | 0x001aff;
    // Red
    constexpr QRgb DefaultTurbopropStartColor = Opaque | 0xe60000;
    constexpr QRgb DefaultTurbopropEndColor = Opaque | 0x66008c;
    // Green
    constexpr QRgb DefaultPistonStartColor = Opaque | 0x00ff20;
    constexpr QRgb DefaultPistonEndColor = Opaque | 0x007020;
    // Orange
    constexpr QRgb DefaultAllStartColor = Opaque | 0xff8f00;
    constexpr QRgb DefaultAllEndColor = Opaque | 0xa06417;

    constexpr int DefaultNofColorsPerRamp = 8;
    constexpr float DefaultLineWidth = 3.0f;
}

class KMLExportDialogPrivate
{
public:
    KMLExportDialogPrivate() noexcept
    {}

    QPushButton *exportButton;
    Unit unit;
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

    p.categoryColors[SimType::EngineType::Jet].first = DefaultJetStartColor;
    p.categoryColors[SimType::EngineType::Jet].second = DefaultJetEndColor;
    p.categoryColors[SimType::EngineType::Turboprop].first = DefaultTurbopropStartColor;
    p.categoryColors[SimType::EngineType::Turboprop].second = DefaultTurbopropStartColor;
    p.categoryColors[SimType::EngineType::Piston].first = DefaultPistonStartColor;
    p.categoryColors[SimType::EngineType::Piston].second = DefaultPistonEndColor;
    p.categoryColors[SimType::EngineType::All].first = DefaultAllStartColor;
    p.categoryColors[SimType::EngineType::All].second = DefaultAllEndColor;
    p.nofColorsPerRamp = DefaultNofColorsPerRamp;
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

void KMLExportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &KMLExportDialog::updateUi);
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
