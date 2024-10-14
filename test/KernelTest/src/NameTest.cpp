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
#include <QTest>
#include <QString>

#include <Kernel/Name.h>
#include "NameTest.h"

// PRIVATE SLOTS

void NameTest::initTestCase()
{}

void NameTest::cleanupTestCase()
{}

void NameTest::nameTest_data()
{
    QTest::addColumn<QString>("camelCase");
    QTest::addColumn<QString>("expected");

    QTest::newRow("camelCase") << "camelCase" << "camel_case";
    QTest::newRow("longerCamelCase") << "longerCamelCase" << "longer_camel_case";
    QTest::newRow("LongerTitleCase") << "LongerTitleCase" << "longer_title_case";
    QTest::newRow("simple") << "simple" << "simple";
    QTest::newRow("Simple") << "Simple" << "simple";
    QTest::newRow("snake_case_remains") << "snake_case_remains" << "snake_case_remains";
    QTest::newRow("A") << "A" << "a";
    QTest::newRow("a") << "a" << "a";
    QTest::newRow("empty") << QString() << QString();
}

void NameTest::nameTest()
{
    // Setup
    QFETCH(QString, camelCase);
    QFETCH(QString, expected);

    // Exercise
    QString result = Name::fromCamelCase(camelCase);

    // Verify
    QCOMPARE(result, expected);
}

QTEST_MAIN(NameTest)
