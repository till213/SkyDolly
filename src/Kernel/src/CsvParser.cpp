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

#include <memory>

#include <QTextStream>
#include <QTextCodec>

#include "CsvParser.h"

struct CsvParserPrivate
{
    CsvParserPrivate(QChar separatorChar, QChar escapeChar, bool trimValue)
        : separatorChar(separatorChar),
          quoteChar(escapeChar),
          trimValue(trimValue)
    {}

    QChar separatorChar;
    QChar quoteChar;
    bool trimValue {false};

    CsvParser::Row row;
    QString currentValue;

    bool inQuotation {false};
    bool currentValueQuoted {false};

    QChar lastChar {'\0'};
    QChar currentChar {'\0'};
};

// PUBLIC

CsvParser::CsvParser(QChar separatorChar, QChar quoteChar, bool trimValue)
    : d(std::make_unique<CsvParserPrivate>(separatorChar, quoteChar, trimValue))
{}

CsvParser::CsvParser(CsvParser &&rhs) = default;
CsvParser &CsvParser::operator=(CsvParser &&rhs) = default;
CsvParser::~CsvParser() = default;

CsvParser::Rows CsvParser::parse(QTextStream &textStream, const QString &header) noexcept
{
    Rows rows;

    int row {0};
    while (!textStream.atEnd())
    {
        const QString line = textStream.readLine();

        if (row == 0 && !header.isNull()) {
            // Compare header (case-insensitive)
            if (line.startsWith(header, Qt::CaseInsensitive)) {
                ++row;
                // Skip header
                continue;
            }
        }

        // Skip empty lines but add them if within a quoted field
        if (line.isEmpty() && !d->inQuotation) {
            continue;
        }

        parseLine(line);
        if (d->inQuotation) {
            // Still in a quoted line, so continue to read unchanged until a quotation character shows the end of the field
            continue;
        }

        rows.push_back(d->row);
        ++row;
    }

    return rows;
}

inline void CsvParser::parseLine(const QString &line) noexcept
{
    if (!d->inQuotation) {
        // Reading a full new line
        reset();
    } else {
        // In a quotation: add a new line
        d->currentValue += "\n";
    }

    for (int i = 0; i < line.size(); ++i) {
        d->currentChar = line.at(i);

        if (d->currentChar == d->quoteChar) {
            parseQuote();
        } else if (d->currentChar == d->separatorChar && !d->inQuotation) {
            // Separator in unquoted text - start new value
            parseSeparator();
        } else {
            // Regular character
            d->currentValue.append(d->currentChar);
            d->lastChar = d->currentChar;
        }
    }

    if (!d->inQuotation) {
        // Finish line
        d->row.push_back((d->trimValue && !d->currentValueQuoted) ? d->currentValue.trimmed() : d->currentValue);
        d->currentValue.clear();
        d->currentValueQuoted = false;
    }
}

inline void CsvParser::parseQuote() noexcept
{
    // Remember if this value is quoted to suppress trimming
    if (!d->currentValueQuoted) {
        // However do trim any characters (expected: whitespace after the previous separator) that we have read so far
        d->currentValue.clear();
        d->currentValueQuoted = true;
    }

    if (d->inQuotation) {
        d->inQuotation = false;
    } else {
        if (d->lastChar == d->quoteChar) {
            // Double quotation character ("escaped") - add single quotation to value and keep quoted state
            d->currentValue.append(d->currentChar);
        }
        d->inQuotation = true;
    }
    d->lastChar = d->currentChar;
}

inline void CsvParser::parseSeparator() noexcept
{
    d->row.push_back((d->trimValue && !d->currentValueQuoted) ? d->currentValue.trimmed() : d->currentValue);
    d->currentValue.clear();
    d->currentValueQuoted = false;
    d->lastChar = d->currentChar;
}

inline void CsvParser::reset() noexcept
{
    d->row.clear();
    d->inQuotation = false;
    d->currentValue.clear();
    d->lastChar = '\0';
    d->currentChar = '\0';
    d->currentValueQuoted = false;
}
