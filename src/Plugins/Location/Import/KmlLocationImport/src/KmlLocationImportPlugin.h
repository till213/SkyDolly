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
#ifndef KMLLOCATIONIMPORTPLUGIN_H
#define KMLLOCATIONIMPORTPLUGIN_H

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
struct KmlLocationImportPluginPrivate;

class KmlLocationImportPlugin : public LocationImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LOCATION_IMPORT_INTERFACE_IID FILE "KmlLocationImportPlugin.json")
    Q_INTERFACES(LocationImportIntf)
public:
    KmlLocationImportPlugin() noexcept;
    KmlLocationImportPlugin(const KmlLocationImportPlugin &rhs) = delete;
    KmlLocationImportPlugin(KmlLocationImportPlugin &&rhs) = delete;
    KmlLocationImportPlugin &operator=(const KmlLocationImportPlugin &rhs) = delete;
    KmlLocationImportPlugin &operator=(KmlLocationImportPlugin &&rhs) = delete;
    ~KmlLocationImportPlugin() override;

protected:
    LocationImportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    std::vector<Location> importLocations(QIODevice &io, bool &ok) noexcept override;

private:
    const std::unique_ptr<KmlLocationImportPluginPrivate> d;

    std::vector<Location> parseKML() noexcept;
};

#endif // KMLLOCATIONIMPORTPLUGIN_H
