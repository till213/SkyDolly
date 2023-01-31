#ifndef CONST_H
#define CONST_H

#include <cstdint>

/*!
 * Generally useful constants.
 */
namespace Const {

    /*!
     * An invalid ID indicates that the object has not yet been (successfully) persisted.
     */
    constexpr std::int64_t InvalidId {-1};

    /*!
     * The logbook file extension.
     */
    constexpr const char *LogbookExtension {".sdlog"};

    /*!
     * The default logbook connection name that is used for the application.
     */
    constexpr const char *DefaultConnectionName {"Default"};

    /*!
     * Suffix indicating zulu time including whitespace, ready to be appended to a
     * formated date/time string.
     */
    constexpr const char *ZuluTimeSuffix {" Z"};

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
