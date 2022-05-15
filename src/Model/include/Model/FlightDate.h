#ifndef FLIGHTDATE_H
#define FLIGHTDATE_H

#include "ModelLib.h"

/*!
 * Represents a date on which flights have been recorded, including
 * the number of recorded flights on that date.
 */
struct MODEL_API FlightDate
{
    FlightDate(int year, int month, int dayOfMonth, int nofFlights) noexcept;
    ~FlightDate() = default;
    FlightDate(const FlightDate &other) = default;
    FlightDate(FlightDate &&other) = default;

    /*!
     * The year of the flight: [1 - 9999]
     */
    int year;

    /*!
     * The month of the flight: [1 - 12]
     */
    int month;

    /*!
     * The day of the month: [1 - 31]
     */
    int dayOfMonth;

    /*!
     * The number of flights on that date.
     */
    int nofFlights;
};

#endif // FLIGHTDATE_H
