#ifndef FLIGHTDATE_H
#define FLIGHTDATE_H

#include "ModelLib.h"

struct MODEL_API FlightDate
{
    FlightDate(int year, int month, int dayOfMonth);

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
};

#endif // FLIGHTDATE_H
