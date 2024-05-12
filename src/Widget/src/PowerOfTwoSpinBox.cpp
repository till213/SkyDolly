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
#include <cstdint>
#include <algorithm>

#include <QWidget>
#include <QSpinBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include "PowerOfTwoSpinBox.h"

// PUBLIC

PowerOfTwoSpinBox::PowerOfTwoSpinBox(QWidget *parent) noexcept
    : QSpinBox(parent)
{}

// PROTECTED

void PowerOfTwoSpinBox::stepBy(int steps)
{
    int currentValue = value();
    int newValue {0};
    if (steps > 0) {
        currentValue += 1;
        if (steps > 1) {
            currentValue <<= 1;
        }
        newValue = static_cast<int>(nextPowerOfTwo(currentValue));
    } else if (steps < 0) {
        currentValue -= 1;
        if (steps < -1) {
            currentValue >>= 1;
        }
        newValue = static_cast<int>(nextLowerPowerOfTwo(currentValue));
    }
    newValue = std::clamp(newValue, minimum(), maximum());

#ifdef DEBUG
    qDebug() << "PowerOfTwoSpinBox: steps:" << steps << "current value:" << value() << "Adjusted value:" << currentValue << "Next power of two:" << newValue
             << "Minimum:" << minimum() << "Maximum:" << maximum();
#endif
    setValue(newValue);
}

// PRIVATE

std::uint32_t PowerOfTwoSpinBox::nextPowerOfTwo(std::uint32_t n) noexcept
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

std::uint32_t PowerOfTwoSpinBox::nextLowerPowerOfTwo(std::uint32_t n) noexcept
{
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    return n - (n >> 1);
}
