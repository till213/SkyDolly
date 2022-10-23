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

#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>

#include "CsvParser.h"

struct CsvParserPrivate
{
    CsvParserPrivate(QChar separatorChar, QChar escapeChar, bool trimValue)
        : separatorChar(separatorChar),
          escapeChar(escapeChar),
          trimValue(trimValue)
    {}


    QChar separatorChar;
    QChar escapeChar;
    bool trimValue {false};

    CsvParser::Columns columns;
    QString currentValue;

    bool inEscape {false};
    bool currentValueEscaped {false};

    QChar lastChar {'\0'};
    QChar currentChar {'\0'};
};

// PUBLIC

CsvParser::CsvParser(QChar separatorChar, QChar escapeChar, bool trimValue)
    : d(std::make_unique<CsvParserPrivate>(separatorChar, escapeChar, trimValue))
{}

CsvParser::Rows CsvParser::parse(QIODevice &io, const QString &header) noexcept
{
    Rows rows;

    QTextStream stream(&io);
    stream.setCodec("UTF-8");

    int rowNum {0};
    while (!stream.atEnd())
    {
        const QString line = stream.readLine();

        if (rowNum == 0 && !header.isNull()) {
            if (header.startsWith(header)) {
                ++rowNum;
                // Skip header
                continue;
            }
        }

        // Skip empty lines but add them if within an escaped field
        if(line.isEmpty() && !d->inEscape) {
            continue;
        }

        parseLine(line);
        if (d->inEscape) {
            // Still in an escaped line so continue to read unchanged until " shows the end of the field
            continue;
        }

        rows.push_back(d->columns);
        ++rowNum;

    }

    return rows;
}

void CsvParser::parseLine(const QString &line) noexcept
{
    if (!d->inEscape) {
        // Reading a full new line
        reset();
    } else {
        // In escape - add a new line
        d->currentValue += "\n";
    }

    for (int i = 0; i < line.size(); ++i) {
        d->currentChar = line.at(i);

        if (d->currentChar == d->escapeChar)
        {
            // Remember if this value is escaped to suppress trimming
            d->currentValueEscaped = true;

            // Found escape character "
            if (d->inEscape) {
                // End of escaped text
                d->inEscape = false;
            } else {
                if (d->lastChar == d->escapeChar)
                    // Escape char itself doubled "" - add single escape " to value and keep escaped state
                    d->currentValue.append(d->currentChar);
                d->inEscape = true;
            }
            d->lastChar = d->currentChar;

            // Do not store value
            continue;
        }

        if (d->currentChar == d->separatorChar && !d->inEscape) {
            // Separator in unescaped text - start new value
            d->columns.push_back((d->trimValue && !d->currentValueEscaped) ? d->currentValue.trimmed() : d->currentValue);
            d->currentValue.clear();
            d->currentValueEscaped = false;
            d->lastChar = d->currentChar;
            continue;
        }

        // Regular character
        d->currentValue.append(d->currentChar);
        d->lastChar = d->currentChar;
    }

    if (!d->inEscape) {
        // Finishe line
        d->columns.push_back((d->trimValue && !d->currentValueEscaped) ? d->currentValue.trimmed() : d->currentValue);
        d->currentValue.clear();
        d->currentValueEscaped = false;
    }
}

void CsvParser::reset() noexcept
{
    d->columns.clear();
    d->inEscape = false;
    d->currentValue.clear();
    d->lastChar = '\0';
    d->currentChar = '\0';
    d->currentValueEscaped = false;
}
