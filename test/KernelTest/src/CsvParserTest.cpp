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
#include <utility>

#include <QtTest>
#include <QBuffer>
#include <QTextStream>
#include <QString>
#include <QStringConverter>
#include <QStringBuilder>

#include <Kernel/CsvParser.h>
#include "CsvParserTest.h"

// PRIVATE

std::pair<QString, QString> CsvParserTest::createCsv(const CsvParser::Row &headers, const CsvParser::Rows &rows, bool quotedValues) noexcept
{
    QString header;

    // Header
    for (int i = 0; i < headers.size(); ++i) {
        QString value = headers.at(i);
        value = value.replace("\"", "\"\"");
        const QString escapedValue = (quotedValues ? "\"" : "") % value % (quotedValues ? "\"" : "");
        header.append(escapedValue);
        if (i < headers.size() - 1) {
            header.append(", ");
        }
    }
    // CSV
    QString csvData;
    for (const auto &row : rows) {
        for (int i = 0; i < row.size(); ++i) {
            QString value = row.at(i);
            value = value.replace("\"", "\"\"");
            const QString escapedValue = (quotedValues ? "\"" : "") % value % (quotedValues ? "\"" : "");
            csvData.append(escapedValue);
            if (i < row.size() - 1) {
                csvData.append(", ");
            } else {
                csvData.append("\n");
            }
        }
    }
    return {header, header % '\n' % csvData};
}

// PRIVATE SLOTS

void CsvParserTest::initTestCase() noexcept
{}

void CsvParserTest::cleanupTestCase() noexcept
{}

void CsvParserTest::parseCsv_data() noexcept
{
    QTest::addColumn<QString>("header");
    QTest::addColumn<QString>("csv");
    QTest::addColumn<CsvParser::Row>("expectedHeaders");
    QTest::addColumn<CsvParser::Rows>("expectedRows");

    // Single row

    CsvParser::Row expectedHeader;
    CsvParser::Row expectedRow;

    expectedRow.push_back("a");
    expectedRow.push_back("b");
    expectedRow.push_back("c");
    CsvParser::Rows expectedRows;
    expectedRows.push_back(expectedRow);

    expectedHeader = {};
    std::pair<QString, QString> csv = createCsv(expectedHeader, expectedRows, false);
    QTest::newRow("Single row") << csv.first
                                << csv.second
                                << expectedHeader
                                << expectedRows;

    // Two rows
    expectedRows.clear();
    expectedRow.clear();
    expectedRow.push_back("a");
    expectedRow.push_back("b");
    expectedRow.push_back("c");
    expectedRows.push_back(expectedRow);
    expectedRow.clear();
    expectedRow.push_back("d");
    expectedRow.push_back("e");
    expectedRow.push_back("f");
    expectedRows.push_back(expectedRow);

    expectedHeader = {};
    csv = createCsv(expectedHeader, expectedRows, false);
    QTest::newRow("Two rows") << csv.first
                              << csv.second
                              << expectedHeader
                              << expectedRows;

    // With header, unquoted
    expectedRows.clear();
    expectedRow.clear();
    expectedRow.push_back("11");
    expectedRow.push_back("12");
    expectedRow.push_back("13");
    expectedRows.push_back(expectedRow);
    expectedRow.clear();
    expectedRow.push_back("21");
    expectedRow.push_back("22");
    expectedRow.push_back("23");
    expectedRows.push_back(expectedRow);

    expectedHeader = {"Header 1", "Header 2", "Header 3"};
    csv = createCsv(expectedHeader, expectedRows, false);
    QTest::newRow("With header #1") << csv.first
                                    << csv.second
                                    << expectedHeader
                                    << expectedRows;

    // With header, quoted
    expectedRows.clear();
    expectedRow.clear();
    expectedRow.push_back("11");
    expectedRow.push_back("12");
    expectedRow.push_back("13");
    expectedRows.push_back(expectedRow);
    expectedRow.clear();
    expectedRow.push_back("21");
    expectedRow.push_back("22");
    expectedRow.push_back("23");
    expectedRows.push_back(expectedRow);

    expectedHeader = {"Header 1", "Header 2", "Header 3"};
    csv = createCsv(expectedHeader, expectedRows, true);
    QTest::newRow("With header #2") << csv.first
                                    << csv.second
                                    << expectedHeader
                                    << expectedRows;

    // Multiline, quoted
    expectedRows.clear();
    expectedRow.clear();
    expectedRow.push_back(R"(keyword 1, keyword 2, keyword 3)");
    expectedRow.push_back(R"("Quoted keyword 4")");
    expectedRows.push_back(expectedRow);
    expectedRow.clear();
    expectedRow.push_back(R"(Multiline
keyword 5)");
    expectedRow.push_back(R"(   Multiline,
non-trimmed, "quoted" and comma-separated keyword 6    )");
    expectedRows.push_back(expectedRow);

    expectedHeader = {};
    csv = createCsv(expectedHeader, expectedRows, true);
    QTest::newRow("Comma-separated, multiline, quoted") << csv.first
                                                        << csv.second
                                                        << expectedHeader
                                                        << expectedRows;

    // UTF-8
    expectedRow.clear();
    expectedRow.push_back("祝你好运");
    expectedRow.push_back("飞行");
    expectedRow.push_back("到月球");
    expectedRows.clear();
    expectedRows.push_back(expectedRow);

    expectedHeader = {};
    csv = createCsv(expectedHeader, expectedRows, true);
    QTest::newRow("UTF-8") << csv.first
                           << csv.second
                           << expectedHeader
                           << expectedRows;
}

void CsvParserTest::parseCsv() noexcept
{
    // Setup
    QFETCH(QString, header);
    QFETCH(QString, csv);    
    QFETCH(CsvParser::Row, expectedHeaders);
    QFETCH(CsvParser::Rows, expectedRows);

    CsvParser csvParser;
    QByteArray data = csv.toUtf8();
    QTextStream textStream {data};
    textStream.setEncoding(QStringConverter::Utf8);

    // Exercise
    const CsvParser::Rows rows = csvParser.parse(textStream, header);
    const CsvParser::Headers headers = csvParser.getHeaders();

    // Verify

    // Headers
    QCOMPARE(headers.size(), expectedHeaders.size());

    int columnIndex = 0;
    for (const QString &expectedHeader : expectedHeaders) {
        const int index = headers.at(expectedHeader);
        QCOMPARE(index, columnIndex);
        ++columnIndex;
    }

    // CSV
    QCOMPARE(rows.size(), expectedRows.size());
    int rowIndex = 0;
    for (const auto &row : rows) {
        const CsvParser::Row &expectedRow = expectedRows.at(rowIndex);
        QCOMPARE(row.size(), expectedRow.size());
        int columnIndex = 0;
        for (const auto &value : row) {
            const QString &expectedValue = expectedRow.at(columnIndex);
            QCOMPARE(value, expectedValue);
            ++columnIndex;
        }
        ++rowIndex;
    }
}

QTEST_MAIN(CsvParserTest)
