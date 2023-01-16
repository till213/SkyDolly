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
#ifndef SKYMATHTEST_H
#define SKYMATHTEST_H

#include <QObject>

/*!
 * Test cases for the SkyMath module.
 */
class SkyMathTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void interpolateHermite180_data();
    void interpolateHermite180();

    void interpolateHermite360_data();
    void interpolateHermite360();

    void fromPosition_data();
    void fromPosition();

    void toPosition_data();
    void toPosition();

    void fromPercent_data();
    void fromPercent();

    void toPercent_data();
    void toPercent();

    void relativePosition_data();
    void relativePosition();

    void headingChange_data();
    void headingChange();

    void bankAngle_data();
    void bankAngle();
};

#endif // SKYMATHTEST_H
