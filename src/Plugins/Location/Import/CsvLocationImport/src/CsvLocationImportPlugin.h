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
#ifndef CSVLOCATIONIMPORTPLUGIN_H
#define CSVLOCATIONIMPORTPLUGIN_H

#include <memory>
#include <vector>

#include <QtPlugin>
#include <QString>

class QIODevice;
class QWidget;

#include <PluginManager/Location/LocationImportIntf.h>
#include <PluginManager/Location/LocationImportPluginBase.h>

struct Location;
class LocationImportPluginBaseSettings;
struct CsvLocationImportPluginPrivate;

class CsvLocationImportPlugin : public LocationImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LOCATION_IMPORT_INTERFACE_IID FILE "CsvLocationImportPlugin.json")
    Q_INTERFACES(LocationImportIntf)
public: 
    CsvLocationImportPlugin() noexcept;
    CsvLocationImportPlugin(const CsvLocationImportPlugin &rhs) = delete;
    CsvLocationImportPlugin(CsvLocationImportPlugin &&rhs) = delete;
    CsvLocationImportPlugin &operator=(const CsvLocationImportPlugin &rhs) = delete;
    CsvLocationImportPlugin &operator=(CsvLocationImportPlugin &&rhs) = delete;
    ~CsvLocationImportPlugin() override;

protected:
    LocationImportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    std::vector<Location> importLocations(QIODevice &io, bool &ok) noexcept override;

private:
    const std::unique_ptr<CsvLocationImportPluginPrivate> d;
};

#endif // CSVLOCATIONIMPORTPLUGIN_H
