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
#ifndef LOCATIONEXPORTINTF_H
#define LOCATIONEXPORTINTF_H

#include <QtPlugin>

#include "../PluginIntf.h"
#include "../DialogPluginIntf.h"

struct Location;

class LocationExportIntf : public DialogPluginIntf, public PluginIntf
{
public:
    /*!
     * Exports all or the selected locations, according to the specific plugin location selection criteria.
     *
     * \return \c true when successful; \c false else
     */
    virtual bool exportLocations() const noexcept = 0;
};

#define LOCATION_EXPORT_INTERFACE_IID "com.github.till213.SkyDolly.LocationExportInterface/1.0"
Q_DECLARE_INTERFACE(LocationExportIntf, LOCATION_EXPORT_INTERFACE_IID)

#endif // LOCATIONEXPORTINTF_H
