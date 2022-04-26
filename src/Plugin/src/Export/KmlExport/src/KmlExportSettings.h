/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include "../../../../../Kernel/src/Settings.h"
#include "../../../ExportPluginBaseSettings.h"

class KmlExportSettingsPrivate;

class KmlExportSettings : public ExportPluginBaseSettings
{
    Q_OBJECT
public:
    enum struct ColorStyle {
        OneColor,
        OneColorPerEngineType,
        ColorRamp,
        ColorRampPerEngineType
    };

    KmlExportSettings() noexcept;
    virtual ~KmlExportSettings() noexcept;

    ColorStyle getColorStyle() const noexcept;
    void setColorStyle(ColorStyle colorStyle) noexcept;

    int getNofColorsPerRamp() const noexcept;
    void setNofColorsPerRamp(int nofColors) noexcept;

    float getLineWidth() const noexcept;
    void setLineWidth(float lineWidth) noexcept;

    QColor getJetStartColor() const noexcept;
    void setJetStartColor(QColor color) noexcept;
    QColor getJetEndColor() const noexcept;
    void setJetEndColor(QColor color) noexcept;

    QColor getTurbopropStartColor() const noexcept;
    void setTurbopropStartColor(QColor color) noexcept;
    QColor getTurbopropEndColor() const noexcept;
    void setTurbopropEndColor(QColor color) noexcept;

    QColor getPistonStartColor() const noexcept;
    void setPistonStartColor(QColor color) noexcept;
    QColor getPistonEndColor() const noexcept;
    void setPistonEndColor(QColor color) noexcept;

    QColor getAllStartColor() const noexcept;
    void setAllStartColor(QColor color) noexcept;
    QColor getAllEndColor() const noexcept;
    void setAllEndColor(QColor color) noexcept;

signals:
    /*!
     * Emitted whenever the extended settings have changed.
     */
    void extendedSettingsChanged();

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    virtual void restoreDefaultsExtn() noexcept override;

private:
    std::unique_ptr<KmlExportSettingsPrivate> d;
};

#endif // KMLEXPORTSETTINGS_H
