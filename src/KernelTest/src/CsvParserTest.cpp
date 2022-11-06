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
#include <QtTest/QtTest>
#include <QBuffer>
#include <QTextStream>
#include <QString>
#include <QTextCodec>
#include <QStringBuilder>

#include <Kernel/CsvParser.h>
#include "CsvParserTest.h"

// PRIVATE

QString CsvParserTest::createCsv(const QString &header, const CsvParser::Rows &rows, bool quotedValues) noexcept
{
    QString csvData;
    if (!header.isEmpty()) {
        csvData = header + '\n';
    }
    for (const auto &columns : rows) {
        for (int i = 0; i < columns.size(); ++i) {
            QString value = columns.at(i);
            value = value.replace("\"", "\"\"");
            const QString escapedValue = (quotedValues ? "\"" : "") % value % (quotedValues ? "\"" : "");
            csvData.append(escapedValue);
            if (i < columns.size() - 1) {
                csvData.append(", ");
            } else {
                csvData.append("\n");
            }
        }
    }
    return csvData;
}

// PRIVATE SLOTS

void CsvParserTest::initTestCase() noexcept
{}

void CsvParserTest::cleanupTestCase() noexcept
{}

void CsvParserTest::parseCsv_data() noexcept
{
    QTest::addColumn<QString>("csv");
    QTest::addColumn<QString>("header");
    QTest::addColumn<CsvParser::Rows>("expectedRows");

    // Single row
    CsvParser::Columns expectedColumns;
    expectedColumns.push_back("a");
    expectedColumns.push_back("b");
    expectedColumns.push_back("c");
    CsvParser::Rows expectedRows;
    expectedRows.push_back(expectedColumns);

    QString header;
    QString csvData = createCsv(header, expectedRows, false);

    QTest::newRow("Single row") << csvData
                                << header
                                << expectedRows;

    // Two rows
    expectedRows.clear();
    expectedColumns.clear();
    expectedColumns.push_back("a");
    expectedColumns.push_back("b");
    expectedColumns.push_back("c");
    expectedRows.push_back(expectedColumns);
    expectedColumns.clear();
    expectedColumns.push_back("d");
    expectedColumns.push_back("e");
    expectedColumns.push_back("f");
    expectedRows.push_back(expectedColumns);

    header.clear();
    csvData = createCsv(header, expectedRows, false);
    QTest::newRow("Two rows") << csvData
                              << header
                              << expectedRows;

    // Comma-separated, multiline, quoted
    expectedRows.clear();
    expectedColumns.clear();
    expectedColumns.push_back("11");
    expectedColumns.push_back("12");
    expectedColumns.push_back("13");
    expectedRows.push_back(expectedColumns);
    expectedColumns.clear();
    expectedColumns.push_back("21");
    expectedColumns.push_back("22");
    expectedColumns.push_back("23");
    expectedRows.push_back(expectedColumns);

    header = "Header 1, Header 2, Header 3";
    csvData = createCsv(header, expectedRows, false);
    QTest::newRow("With header") << csvData
                                 << header
                                 << expectedRows;
    expectedRows.clear();
    expectedColumns.clear();
    expectedColumns.push_back(R"(keyword 1, keyword 2, keyword 3)");
    expectedColumns.push_back(R"("Quoted keyword 4")");
    expectedRows.push_back(expectedColumns);
    expectedColumns.clear();
    expectedColumns.push_back(R"(Multiline
keyword 5)");
    expectedColumns.push_back(R"(   Multiline,
non-trimmed, "quoted" and comma-separated keyword 6    )");

    expectedRows.push_back(expectedColumns);
    header.clear();
    csvData = createCsv(header, expectedRows, true);
    QTest::newRow("Comma-separated, multiline, quoted") << csvData
                                                        << header
                                                        << expectedRows;

    // UTF-8
    expectedColumns.clear();
    expectedColumns.push_back("祝你好运");
    expectedColumns.push_back("飞行");
    expectedColumns.push_back("到月球");
    expectedRows.clear();
    expectedRows.push_back(expectedColumns);
    header.clear();
    csvData = createCsv(header, expectedRows, true);

    QTest::newRow("UTF-8") << csvData
                           << header
                           << expectedRows;
}

void CsvParserTest::parseCsv() noexcept
{

    // Setup
    QFETCH(QString, csv);
    QFETCH(QString, header);
    QFETCH(CsvParser::Rows, expectedRows);

    CsvParser csvParser;
    QByteArray data = csv.toUtf8();
    QTextStream textStream {data};
    textStream.setCodec("UTF-8");

    // Exercise
    const CsvParser::Rows rows = csvParser.parse(textStream, header);

    // Verify
    QCOMPARE(rows.size(), expectedRows.size());
    int row = 0;
    for (const auto &columns : rows) {
        const CsvParser::Columns expectedColumns = expectedRows.at(row);
        QCOMPARE(columns.size(), expectedColumns.size());
        int column = 0;
        for (const auto &value : columns) {
            const QString expectedValue = expectedColumns.at(column);
            QCOMPARE(value, expectedValue);
            ++column;
        }
        ++row;
    }
}

QTEST_MAIN(CsvParserTest)
