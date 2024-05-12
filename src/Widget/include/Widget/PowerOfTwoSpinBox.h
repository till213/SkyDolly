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
#ifndef POWEROFTWOSPINBOX_H
#define POWEROFTWOSPINBOX_H

#include <cstdint>

#include <QSpinBox>

class QWidget;

#include "WidgetLib.h"

/*!
 * Steps to the next higher (or lower) value that is a power of two (1, 2, 4, 8, 16, 32, ...).
 */
class WIDGET_API PowerOfTwoSpinBox : public QSpinBox
{
public:
    PowerOfTwoSpinBox(QWidget *parent = nullptr) noexcept;

protected:
    void stepBy(int steps) override;

private:
    static std::uint32_t nextPowerOfTwo(std::uint32_t n) noexcept;
    static std::uint32_t nextLowerPowerOfTwo(std::uint32_t n) noexcept;
};

#endif // POWEROFTWOSPINBOX_H
