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
#ifndef CSVLOCATIONEXPORTPLUGIN_H
/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation ios (the "Software"), to deal in the Software
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
#define CSVLOCATIONEXPORTPLUGIN_H

#include <memory>
#include <vector>

#include <QtPlugin>
#include <QString>

class QFile;
class QWidget;

#include <PluginManager/Location/LocationExportIntf.h>
#include <PluginManager/Location/LocationExportPluginBase.h>

struct Location;
class LocationExportPluginBaseSettings;
struct CsvLocationExportPluginPrivate;

class CsvLocationExportPlugin : public LocationExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LOCATION_EXPORT_INTERFACE_IID FILE "CsvLocationExportPlugin.json")
    Q_INTERFACES(LocationExportIntf)
public:
    CsvLocationExportPlugin() noexcept;
    CsvLocationExportPlugin(const CsvLocationExportPlugin &rhs) = delete;
    CsvLocationExportPlugin(CsvLocationExportPlugin &&rhs) = delete;
    CsvLocationExportPlugin &operator=(const CsvLocationExportPlugin &rhs) = delete;
    CsvLocationExportPlugin &operator=(CsvLocationExportPlugin &&rhs) = delete;
    ~CsvLocationExportPlugin() override;

protected:
    LocationExportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool exportLocations(const std::vector<Location> &locations, QIODevice &io) const noexcept override;

private:
    const std::unique_ptr<CsvLocationExportPluginPrivate> d;
};

#endif // CSVLOCATIONEXPORTPLUGIN_H
