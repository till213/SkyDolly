/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef FLIGHTIMPORTINTF_H
#define FLIGHTIMPORTINTF_H

#include <vector>

#include <QtPlugin>

class QIODevice;

#include "../PluginIntf.h"
#include "../DialogPluginIntf.h"

class Flight;
struct FlightData;
class FlightService;

class FlightImportIntf : public DialogPluginIntf, public PluginIntf
{
public:
    /*!
     * Presents the user with a file selection dialog and imports all selected
     * files, optionally adding the imported aircraft to the \p currentFlight
     * as well.
     *
     * \param currentFlight
     *        the current flight in memory
     * \return \c true upon success; \c false else (parsing error, no data
     *         imported)
     */
    virtual bool importFlights(Flight &currentFlight) noexcept = 0;

    /*!
     * Imports the flight data from the given \p io data source and returns the list
     * of imported FlightData. Note that \p ok is also set to \c false in case
     * no flight data was imported at all (despite the existing file not having
     * any syntax errors).
     *
     * The data source \p io must have been properly opened for reading already.
     *
     * \param io
     *        the IO device to read from; already opened for reading
     * \param ok
     *        is set to \c true in case of success; \c false else (a parse/read error occured
     *        or otherwise no data imported)
     * \return the list of imported flight data
     */
    virtual std::vector<FlightData> importFlightData(QIODevice &io, bool &ok) noexcept = 0;
};

#define FLIGHT_IMPORT_INTERFACE_IID "com.github.till213.SkyDolly.FlightImportInterface/1.0"
Q_DECLARE_INTERFACE(FlightImportIntf, FLIGHT_IMPORT_INTERFACE_IID)

#endif // FLIGHTIMPORTINTF_H
