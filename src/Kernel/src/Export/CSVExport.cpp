#include <QIODevice>
#include <QStringBuilder>

#include "../Aircraft.h"
#include "CSVExport.h"

namespace {
    constexpr char Separator = '\t';
    constexpr char EndLine = '\n';
}

class CSVExportPrivate
{
public:
    CSVExportPrivate()
    {}

    ~CSVExportPrivate()
    {}
};

// PUBLIC

CSVExport::CSVExport()
    : d(new CSVExportPrivate())
{

}

CSVExport::~CSVExport()
{
    delete d;
}

bool CSVExport::exportData(const Aircraft &aircraft, QIODevice &io) const
{
    bool ok;
    ok = io.open(QIODevice::WriteOnly);
    if (ok) {

        // Header
        QString csv = QString("Latitude") % Separator % QString("Longitude") % Separator % QString("Altitude") % Separator %
                      QString("Pitch") % Separator % QString("Bank") % Separator % QString("Heading") % Separator %
                      QString("YokeXPosition") % Separator % QString("YokeYPosition") % Separator % QString("RudderPosition") % Separator % QString("ElevatorPosition") % Separator % QString("AileronPosition") % EndLine;
        if (!io.write(csv.toLocal8Bit())) {
            ok = false;
        }

        if (ok) {
            // CSV data
            for (const AircraftData &data : aircraft.getAllAircraftData()) {
                QString csv = QString::number(data.latitude) % Separator % QString::number(data.longitude) % Separator % QString::number(data.altitude) % Separator %
                              QString::number(data.pitch) % Separator % QString::number(data.bank) % Separator % QString::number(data.heading) % Separator %
                              QString::number(data.yokeXPosition) % Separator % QString::number(data.yokeYPosition) % Separator % QString::number(data.rudderPosition) % Separator % QString::number(data.elevatorPosition) % Separator % QString::number(data.aileronPosition) % EndLine;
                if (!io.write(csv.toLocal8Bit())) {
                    ok = false;
                    break;
                }
            }
        }
    } else {
        ok = false;
    }
    return ok;
}
