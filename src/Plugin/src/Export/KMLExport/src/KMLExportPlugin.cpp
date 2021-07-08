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
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "KMLExportPlugin.h"

// PUBLIC

KMLExportPlugin::KMLExportPlugin()
{
    Q_INIT_RESOURCE(KMLExport);
#ifdef DEBUG
    qDebug("KMLExportPlugin::KMLExportPlugin: PLUGIN LOADED");
#endif
}

KMLExportPlugin::~KMLExportPlugin()
{
#ifdef DEBUG
    qDebug("KMLExportPlugin::~KMLExportPlugin: PLUGIN UNLOADED");
#endif
}

bool KMLExportPlugin::exportData() const noexcept
{
    bool ok;
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getSaveFileName(getParentWidget(), tr("Export KML"), exportPath, QString("*.kml"));
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            file.setTextModeEnabled(true);

            QFile kmlFile(":/kml/Header.kml");
            kmlFile.open(QIODevice::ReadOnly);
            kmlFile.setTextModeEnabled(true);
            ok = file.write(kmlFile.readAll());
            kmlFile.close();
        }
        if (ok) {
            ok = exportFlightData(flight, file);
        }
        if (ok) {
            QFile kmlFile(":/kml/Footer.kml");
            kmlFile.open(QIODevice::ReadOnly);
            kmlFile.setTextModeEnabled(true);
            ok = file.write(kmlFile.readAll());
            kmlFile.close();
        }
        file.close();

        if (ok) {
            exportPath = QFileInfo(filePath).absolutePath();
            Settings::getInstance().setExportPath(exportPath);
        } else {
            QMessageBox::critical(getParentWidget(), tr("Export error"), tr("The KML file %1 could not be written.").arg(filePath));
        }
    } else {
        ok = true;
    }
    return ok;
}

// PRIVATE

bool KMLExportPlugin::exportFlightData(const Flight &flight, QFile &file) const noexcept
{
    bool ok = true;
    for (const auto &aircraft : flight) {
        ok = exportAircraftData(*aircraft, file);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool KMLExportPlugin::exportAircraftData(const Aircraft &aircraft, QFile &file) const noexcept
{
    const PositionData positionData;
    const QString placemarkBegin = QString(
"        <Placemark>\n"
"            <name>Sky Dolly</name>\n"
"             <description>Flight path</description>\n"
"            <styleUrl>#m_flight</styleUrl>\n"
"            <LineString>\n"
"                <extrude>1</extrude>\n"
"                <tessellate>1</tessellate>\n"
"                <altitudeMode>absolute</altitudeMode>\n"
"                <coordinates>\n");
    bool ok = file.write(placemarkBegin.toUtf8());
    if (ok) {
        // Position data
        const Position &position = aircraft.getPositionConst();
        const qint64 duration = position.getLast().timestamp;
        qint64 time = 0;

        while (time <= duration && ok) {
            const PositionData &positionData = position.interpolate(time, TimeVariableData::Access::Linear);
            if (!positionData.isNull()) {
                ok = file.write((QString::number(positionData.longitude, 'g', 12) % "," %
                                 QString::number(positionData.latitude, 'g', 12) % "," %
                                 QString::number(Convert::feetToMeters(positionData.altitude), 'g', 6)).toUtf8() % " ");
            }
            // Advance in 1 second (1000 ms) steps (resampling @1Hz)
            time += 1000;
        }
    }
    if (ok) {
        const QString placemarkEnd = QString(
"\n"
"                </coordinates>\n"
"            </LineString>\n"
"        </Placemark>\n");
        ok = file.write(placemarkEnd.toUtf8());
    }
    return ok;
}
