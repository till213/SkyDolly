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
#ifndef KMLEXPORTSETTINGS_H
#define KMLEXPORTSETTINGS_H

#include <memory>

#include <QObject>
#include <QColor>

#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportPluginBaseSettings.h>

struct KmlExportSettingsPrivate;

class KmlExportSettings : public FlightExportPluginBaseSettings
{
    Q_OBJECT
public:
    /*!
     * The colour styles of the lines.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct ColorStyle {
        OneColor = 0,
        OneColorPerEngineType = 1,
        ColorRamp = 2,
        ColorRampPerEngineType = 3
    };

    KmlExportSettings() noexcept;
    KmlExportSettings(const KmlExportSettings &rhs) = delete;
    KmlExportSettings(KmlExportSettings &&rhs) = delete;
    KmlExportSettings &operator=(const KmlExportSettings &rhs) = delete;
    KmlExportSettings &operator=(KmlExportSettings &&rhs) = delete;
    ~KmlExportSettings() override;

    ColorStyle getColorStyle() const noexcept;
    void setColorStyle(ColorStyle colorStyle) noexcept;

    int getNofColorsPerRamp() const noexcept;
    void setNofColorsPerRamp(int nofColors) noexcept;

    float getLineWidth() const noexcept;
    void setLineWidth(float lineWidth) noexcept;

    QColor getJetStartColor() const noexcept;
    void setJetStartColor(const QColor &color) noexcept;
    QColor getJetEndColor() const noexcept;
    void setJetEndColor(const QColor &color) noexcept;

    QColor getTurbopropStartColor() const noexcept;
    void setTurbopropStartColor(const QColor &color) noexcept;
    QColor getTurbopropEndColor() const noexcept;
    void setTurbopropEndColor(const QColor &color) noexcept;

    QColor getPistonStartColor() const noexcept;
    void setPistonStartColor(const QColor &color) noexcept;
    QColor getPistonEndColor() const noexcept;
    void setPistonEndColor(const QColor &color) noexcept;

    QColor getAllStartColor() const noexcept;
    void setAllStartColor(const QColor &color) noexcept;
    QColor getAllEndColor() const noexcept;
    void setAllEndColor(const QColor &color) noexcept;

    bool isResamplingSupported() const noexcept override;
    bool isFormationExportSupported(FormationExport formationExport) const noexcept override;

protected:
    void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    void restoreDefaultsExtn() noexcept override;

private:
   const std::unique_ptr<KmlExportSettingsPrivate> d;
};

#endif // KMLEXPORTSETTINGS_H
