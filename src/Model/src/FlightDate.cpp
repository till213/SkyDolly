#include "FlightDate.h"

// PUBLIC

FlightDate::FlightDate(int theYear, int theMonth, int theDayOfMonth, int theNofFlights) noexcept
    : year(theYear),
      month(theMonth),
      dayOfMonth(theDayOfMonth),
      nofFlights(theNofFlights)
{}
