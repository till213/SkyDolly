/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SAMPLERATE_H
#define SAMPLERATE_H

#include <cstdint>

#include <QtGlobal>

/*!
 * Recording sample rates.
 */
namespace SampleRate
{
    /*!
     * Setting the recording sample rate to \e auto will enable
     * "event-based sampling" ("as fast as data arrives").
     */
    constexpr int AutoValue = 999.0;
    constexpr double DefaultAutoSampleRate = 60.0;

    /*!
     * The SampleRate defines various recording sample rates.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct SampleRate: std::uint8_t {
        First = 0,
        Auto = First,
        Hz1,
        Hz2,
        Hz5,
        Hz10,
        Hz15,
        Hz20,
        Hz24,
        Hz25,
        Hz30,
        Hz45,
        Hz50,
        Hz60,
        Last = Hz60
    };

    /*!
     * Resampling period [millisecons], useful for resampling during data export.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct ResamplingPeriod: std::uint16_t {
        First = 0,
        Original = First,
        TenHz = 100,
        FiveHz = 200,
        TwoHz = 500,
        OneHz = 1000,
        AFifthHz = 5000,
        ATenthHz = 10000,
        Last = ATenthHz
    };

    static constexpr ResamplingPeriod DefaultResamplingPeriod = ResamplingPeriod::OneHz;

    /*!
     * Converts the given \p sampleRate enumeration value to the corresponding value in Hz.
     *
     * \param sampleRate
     *        the sample rate enumeration to convert
     * \return the sample rate in Hz
     */
    constexpr double toValue(SampleRate sampleRate) noexcept {
        switch (sampleRate) {
        case SampleRate::Auto:
            return AutoValue;
        case SampleRate::Hz1:
            return 1.0;
        case SampleRate::Hz2:
            return 2.0;
        case SampleRate::Hz5:
            return 5.0;
        case SampleRate::Hz10:
            return 10.0;
        case SampleRate::Hz15:
            return 15.0;
        case SampleRate::Hz20:
            return 20.0;
        case SampleRate::Hz24:
            return 24.0;
        case SampleRate::Hz25:
            return 25.0;
        case SampleRate::Hz30:
            return 30.0;
        case SampleRate::Hz45:
            return 45.0;
        case SampleRate::Hz50:
            return 50.0;
        case SampleRate::Hz60:
            return 60.0;
        }
        return AutoValue;
    }

    /*!
     * Converts the given \p sampleRate in Hz to the corresponding enumeration value.
     *
     * \param sampleRate
     *        the sample rate in Hz to convert
     * \return the sample rate enumeration value; default: \p Auto
     */
    constexpr SampleRate fromValue(double sampleRate) noexcept {
        if (sampleRate <= 1.0) {
            return SampleRate::Hz1;
        } else if (sampleRate <= 2.0) {
            return SampleRate::Hz2;
        } else if (sampleRate <= 5.0) {
            return SampleRate::Hz5;
        } else if (sampleRate <= 10.0) {
            return SampleRate::Hz10;
        } else if (sampleRate <= 15.0) {
            return SampleRate::Hz15;
        } else if (sampleRate <= 20.0) {
            return SampleRate::Hz20;
        } else if (sampleRate <= 24.0) {
            return SampleRate::Hz24;
        } else if (sampleRate <= 25.0) {
            return SampleRate::Hz25;
        } else if (sampleRate <= 30.0) {
            return SampleRate::Hz30;
        } else if (sampleRate <= 45.0) {
            return SampleRate::Hz45;
        } else if (sampleRate <= 50.0) {
            return SampleRate::Hz50;
        } else if (sampleRate <= 60.0) {
            return SampleRate::Hz60;
        } else {
            return SampleRate::Auto;
        }
    }

    /*!
     * Returns the interval [msec], suitable for a QTimer, corresponding to the given \p sampleRateValue [Hz].
     *
     * \param sampleRateValue
     *        the sample rate value to convert to the corresponding interval
     * \return the interval in milliseconds
     */
    inline int toIntervalMSec(double sampleRateValue) noexcept {
        if (sampleRateValue != AutoValue) {
            return static_cast<int>(1000.0 / sampleRateValue);
        } else {
            return static_cast<int>(1000.0 / DefaultAutoSampleRate);
        }
    }

    /*!
     * Returns the interval [msec], suitable for a QTimer, corresponding to the given \p sampleRate [Hz].
     * \param sampleRate
     *        the sample rate to convert to the corresponding interval
     * \return the interval in milliseconds
     */
    inline int toIntervalMSec(SampleRate sampleRate) noexcept {
        if (sampleRate != SampleRate::Auto) {
            return static_cast<int>(1000.0 / toValue(sampleRate));
        } else {
            return DefaultAutoSampleRate;
        }
    }
}

#endif // SAMPLERATE_H
