#ifndef FLIGHTDATE_H
#define FLIGHTDATE_H

#include <cstring>

#include "ModelLib.h"

/*!
 * Represents a date on which flights have been recorded, including
 * the number of recorded flights on that date.
 */
struct MODEL_API FlightDate final
{
    FlightDate(int year, int month, int dayOfMonth, int nofFlights) noexcept;

    /*!
     * The year of the flight: [1 - 9999]
     */
    int year {0};

    /*!
     * The month of the flight: [1 - 12]
     */
    int month {0};

    /*!
     * The day of the month: [1 - 31]
     */
    int dayOfMonth {0};

    /*!
     * The number of flights on that date.
     */
    int nofFlights {0};
};

inline bool operator==(const FlightDate &lhs, const FlightDate &rhs) 
{ 
    return std::memcmp(&lhs, &rhs, sizeof(FlightDate)) == 0;
}

inline bool operator<(const FlightDate& lhs, const FlightDate& rhs)
{
    return lhs.year < rhs.year || lhs.month < rhs.month || rhs.dayOfMonth < lhs.dayOfMonth || lhs.nofFlights < rhs.nofFlights;
}

#endif // FLIGHTDATE_H
