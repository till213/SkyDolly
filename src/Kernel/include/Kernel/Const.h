#ifndef CONST_H
#define CONST_H

#include <cstdint>

namespace Const {

    /*!
     * An invalid ID indicates that the object has not yet been (successfully) persisted.
     */
    constexpr std::int64_t InvalidId {-1};

    constexpr const char *LogbookExtension {".sdlog"};

    constexpr const char *BackupNeverSymId = "NEVER";
    constexpr const char *BackupNowSymId = "NOW";
    constexpr const char *BackupMonthlySymId = "MONTH";
    constexpr const char *BackupWeeklySymId = "WEEK";
    constexpr const char *BackupDailySymId = "DAY";
    constexpr const char *BackupAlwaysSymId = "ALWAYS";

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
}

#endif // CONST_H
