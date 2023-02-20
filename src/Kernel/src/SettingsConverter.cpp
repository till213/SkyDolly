/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <QSettings>
#include <QByteArray>
#include <QUuid>

#include "Version.h"
#include "Const.h"
#include "SettingsConverter.h"

namespace
{
    void convertToV0dot13([[maybe_unused]] const Version &settingsVersion, QSettings &settings) noexcept
    {
        settings.beginGroup("Window");
        {
            settings.setValue("LocationTableState", QVariant());
        }
        settings.endGroup();
    }

    void convertPluginsV0dot16(QSettings &settings) noexcept
    {
        static constexpr const char *AddToFlightEnabledKey = "AddToFlightEnabled";
        static constexpr const char *ImportDirectoryKey = "ImportDirectoryEnabled";
        static constexpr const char *AircraftImportModeKey = "AircraftImportMode";
        bool addToFlight {false};
        bool importDirectory {false};

        // CSV import
        settings.beginGroup(QString("Plugins/") + QUuid(Const::CsvImportPluginUuid).toByteArray());
        {
            addToFlight = settings.value(AddToFlightEnabledKey).toBool();
            importDirectory = settings.value(ImportDirectoryKey).toBool();
            if (addToFlight) {
                // Add to current flight
                settings.setValue(AircraftImportModeKey, 0);
            } else {
                // Add to separate flights / new flight
                settings.setValue(AircraftImportModeKey, importDirectory ? 2 : 1);
            }
            // Remove obsolete setting
            settings.remove(AddToFlightEnabledKey);
        }
        settings.endGroup();

        // GPX import
        settings.beginGroup(QString("Plugins/") + QUuid(Const::GpxImportPluginUuid).toByteArray());
        {
            addToFlight = settings.value(AddToFlightEnabledKey).toBool();
            importDirectory = settings.value(ImportDirectoryKey).toBool();
            if (addToFlight) {
                settings.setValue(AircraftImportModeKey, 0);
            } else {
                settings.setValue(AircraftImportModeKey, importDirectory ? 2 : 1);
            }
            settings.remove(AddToFlightEnabledKey);
        }
        settings.endGroup();

        // IGC import
        settings.beginGroup(QString("Plugins/") + QUuid(Const::IgcImportPluginUuid).toByteArray());
        {
            addToFlight = settings.value(AddToFlightEnabledKey).toBool();
            importDirectory = settings.value(ImportDirectoryKey).toBool();
            if (addToFlight) {
                settings.setValue(AircraftImportModeKey, 0);
            } else {
                settings.setValue(AircraftImportModeKey, importDirectory ? 2 : 1);
            }
            settings.remove(AddToFlightEnabledKey);
        }
        settings.endGroup();

        // KML import
        settings.beginGroup(QString("Plugins/") + QUuid(Const::KmlImportPluginUuid).toByteArray());
        {
            addToFlight = settings.value(AddToFlightEnabledKey).toBool();
            importDirectory = settings.value(ImportDirectoryKey).toBool();
            if (addToFlight) {
                settings.setValue(AircraftImportModeKey, 0);
            } else {
                settings.setValue(AircraftImportModeKey, importDirectory ? 2 : 1);
            }
            settings.remove(AddToFlightEnabledKey);
        }
        settings.endGroup();
    }

    void convertModulesV0dot16(QSettings &settings) noexcept
    {
        QByteArray logbookTableState;
        QByteArray formationAircraftTableState;
        QByteArray locationTableState;
        bool relativePositionPlacement {false};

        settings.beginGroup("Window");
        {
            logbookTableState = settings.value("LogbookState").toByteArray();
            formationAircraftTableState = settings.value("FormationAircraftTableState").toByteArray();
            locationTableState = settings.value("LocationTableState").toByteArray();
            settings.remove("LogbookState");
            settings.remove("FormationAircraftTableState");
            settings.remove("LocationTableState");
        }
        settings.endGroup();

        settings.beginGroup("Plugins/Modules/Formation");
        {
            relativePositionPlacement = settings.value("RelativePositionPlacement").toBool();
            settings.remove("RelativePositionPlacement");
        }
        settings.endGroup();

        // Logbook
        settings.beginGroup(QString("Plugins/Modules/") + QUuid(Const::LogbookModuleUuid).toByteArray());
        {
            settings.setValue("LogbookTableState", logbookTableState);
        }
        settings.endGroup();

        // Formation
        settings.beginGroup(QString("Plugins/Modules/") + QUuid(Const::FormationModuleUuid).toByteArray());
        {
            settings.setValue("RelativePositionPlacement", relativePositionPlacement);
            settings.setValue("FormationAircraftTableState", formationAircraftTableState);
        }
        settings.endGroup();

        // Location
        settings.beginGroup(QString("Plugins/Modules/") + QUuid(Const::LocationModuleUuid).toByteArray());
        {
            settings.setValue("LocationTableState", locationTableState);
        }
        settings.endGroup();
    }

    void convertToV0dot16(const Version &settingsVersion, QSettings &settings) noexcept
    {
        if (settingsVersion < Version(QString("0.13.0"))) {
            convertToV0dot13(settingsVersion, settings);
        }
        convertPluginsV0dot16(settings);
        convertModulesV0dot16(settings);
    }
}

// PUBLIC

void SettingsConverter::convertToCurrent(const Version &settingsVersion, QSettings &settings) noexcept
{
    const Version currentVersion;
    if (settingsVersion < currentVersion) {
        ::convertToV0dot16(settingsVersion, settings);
    }
}
