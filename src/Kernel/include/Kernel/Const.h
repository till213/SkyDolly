#ifndef CONST_H
#define CONST_H

#include <cstdint>

#include <QString>

/*!
 * Generally useful constants.
 */
namespace Const {

    /*!
     * An \e invalid ID indicates that the object has been newly created and
     * not yet been (successfully) persisted.
     */
    constexpr std::int64_t InvalidId {-1};

    /*!
     * A \e recording ID indicates that the object is being recorded, but
     * has not yet been persisted.
     */
    constexpr std::int64_t RecordingId {-2};

    /*!
     * An invalid aircraft index indicates that no aircraft has been specified as user aircraft.
     * A valid index starts at 0.
     */
    constexpr int InvalidIndex {-1};

    /*!
     * The plain logbook file extension (without leading dot).
     */
    const inline QString LogbookExtension {"sdlog"};

    /*!
     * The logbook file extension, with a leading dot.
     */
    const inline QString DotLogbookExtension {".sdlog"};

    /*!
     * The logbook connection name that is used for the application.
     */
    const inline QString DefaultConnectionName {"SkyDolly"};

    /*!
     * The logbook connection name that is used for import.
     */
    const inline QString ImportConnectionName {"Import"};

    /*!
     * The logbook connection name that is used for export.
     */
    const inline QString ExportConnectionName {"Export"};

    /*!
     * Suffix indicating zulu time including whitespace, ready to be appended to a formated date/time string.
     */
    const inline QString ZuluTimeSuffix {" Z"};

    // Altitude [feet] and speed [knots] minima and maxima

    constexpr int DefaultAltitude {5000};
    // Dead Sea Depression - The lowest point on Earth: -413 meters
    // https://geology.com/below-sea-level/
    constexpr int MinimumAltitude {-1500};
    // https://www.reddit.com/r/flightsim/comments/ibstui/turns_out_the_maximum_altitude_in_fs2020_275000/
    constexpr int MaximumAltitude {275000};
    constexpr int DefaultIndicatedAirspeed {120};
    constexpr int MinimumIndicatedAirspeed {0};
    // NASA X-43 (Mach 9.6)
    // https://internationalaviationhq.com/2020/06/27/17-fastest-aircraft/
    constexpr int MaximumIndicatedAirspeed {6350};

    // Known plugin UUIDs

    // Import
    const inline QString CsvImportPluginUuid {"077448de-4909-4c5e-8957-2347afee6708"};
    const inline QString GpxImportPluginUuid {"13f44df3-1df6-4458-ad29-71f7b185bf3e"};
    const inline QString IgcImportPluginUuid {"a1902cf0-82a8-47ed-86ee-5a15152697c7"};
    const inline QString KmlImportPluginUuid {"5a72c866-310d-4d84-8bd6-1baa720bc64e"};
    const inline QString SdLogImportPluginUuid {"a0b3c866-3000-4026-aeb5-f53ae5451609"};
    const inline QString CsvExportPluginUuid {"e718d4d2-da18-422b-921d-625315b75ca8"};

    // Known module UUIDs

    const inline QString LogbookModuleUuid {"441c2289-1aab-4724-9651-c74d4961270f"};
    const inline QString FormationModuleUuid {"d1fb9d27-67de-4a03-8333-202f599a0454"};
    const inline QString LocationModuleUuid {"a9fd31c9-0559-4ac3-abeb-a06e29ac7357"};
}

#endif // CONST_H
