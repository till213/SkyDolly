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

// PUBLIC

CsvParser::CsvParser(QChar separatorChar, QChar quoteChar, bool trimValue)
    : m_separatorChar(separatorChar),
      m_quoteChar(quoteChar),
      m_trimValue(trimValue)
{}

CsvParser::Rows CsvParser::parse(QTextStream &textStream, const QString &header, const QString &alternateHeader) noexcept
{
    Rows rows;

    int row {0};
    while (!textStream.atEnd())
    {
        const QString line = textStream.readLine();

        if (row == 0) {
            // Compare header (case-insensitive)
            if (!header.isNull() && line.startsWith(header, Qt::CaseInsensitive) ||
                !alternateHeader.isNull() && line.startsWith(alternateHeader, Qt::CaseInsensitive))
            {
                ++row;
                // Skip header
                continue;
            }
        }

        // Skip empty lines but add them if within a quoted field
        if (line.isEmpty() && !m_inQuotation) {
            continue;
        }

        parseLine(line);
        if (m_inQuotation) {
            // Still in a quoted line, so continue to read unchanged until a quotation character shows the end of the field
            continue;
        }

        rows.push_back(m_currentRow);
        ++row;
    }

    return rows;
}

inline void CsvParser::parseLine(const QString &line) noexcept
{
    if (!m_inQuotation) {
        // Reading a full new line
        reset();
    } else {
        // In a quotation: add a new line
        m_currentValue += "\n";
    }

    for (int i = 0; i < line.size(); ++i) {
        m_currentChar = line.at(i);

        if (m_currentChar == m_quoteChar) {
            parseQuote();
        } else if (m_currentChar == m_separatorChar && !m_inQuotation) {
            // Separator in unquoted text - start new value
            parseSeparator();
        } else {
            // Regular character
            m_currentValue.append(m_currentChar);
            m_lastChar = m_currentChar;
        }
    }

    if (!m_inQuotation) {
        // Finish line
        m_currentRow.push_back((m_trimValue && !m_currentValueQuoted) ? m_currentValue.trimmed() : m_currentValue);
        m_currentValue.clear();
        m_currentValueQuoted = false;
    }
}

inline void CsvParser::parseQuote() noexcept
{
    // Remember if this value is quoted to suppress trimming
    if (!m_currentValueQuoted) {
        // However do trim any characters (expected: whitespace after the previous separator) that we have read so far
        m_currentValue.clear();
        m_currentValueQuoted = true;
    }

    if (m_inQuotation) {
        m_inQuotation = false;
    } else {
        if (m_lastChar == m_quoteChar) {
            // Double quotation character ("escaped") - add single quotation to value and keep quoted state
            m_currentValue.append(m_currentChar);
        }
        m_inQuotation = true;
    }
    m_lastChar = m_currentChar;
}

inline void CsvParser::parseSeparator() noexcept
{
    m_currentRow.push_back((m_trimValue && !m_currentValueQuoted) ? m_currentValue.trimmed() : m_currentValue);
    m_currentValue.clear();
    m_currentValueQuoted = false;
    m_lastChar = m_currentChar;
}

inline void CsvParser::reset() noexcept
{
    m_currentRow.clear();
    m_inQuotation = false;
    m_currentValue.clear();
    m_lastChar = '\0';
    m_currentChar = '\0';
    m_currentValueQuoted = false;
}
