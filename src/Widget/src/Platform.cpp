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
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QString>
#include <QStringLiteral>

#include "Platform.h"
#include "QtCore/qstringliteral.h"

namespace CommonTheme {
    constexpr QRgb ActiveButtonBGColor {0x0077e5};
}

namespace DarkTheme {
    constexpr QRgb EditableTableCellBGColor {0xff3aa8ff};
    constexpr QRgb ButtonBGColor {0x6a6a6a};
}

namespace BrightTheme {
    constexpr QRgb EditableTableCellBGColor {0xfff6fdff};
    constexpr QRgb ButtonBGColor {0xe1e1e1};
}

// PUBLIC

inline bool Platform::isDarkModeEnabled() noexcept
{
    const QPalette palette = QApplication::palette();
    const QColor &windowColor = palette.color(QPalette::Window);
    const QColor &textColor = palette.color(QPalette::Text);
    return windowColor.value() < textColor.value();
}

QColor Platform::getEditableTableCellBGColor() noexcept
{
    return isDarkModeEnabled() ? DarkTheme::EditableTableCellBGColor : BrightTheme::EditableTableCellBGColor;
}

QColor Platform::getActiveButtonBGColor() noexcept
{
    return {CommonTheme::ActiveButtonBGColor};
}

QColor Platform::getButtonBGColor() noexcept
{
    return isDarkModeEnabled() ? DarkTheme::ButtonBGColor : BrightTheme::ButtonBGColor;
}
