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

#include <memory>
#include <unordered_map>

#include <QTextStream>

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
    bool firstLine {true};
    while (!textStream.atEnd())
    {
        const QString line = textStream.readLine();

        if (firstLine) {
            firstLine = false;
            // Compare header (case-insensitive)
            if (!header.isNull() && line.startsWith(header, Qt::CaseInsensitive) ||
                !alternateHeader.isNull() && line.startsWith(alternateHeader, Qt::CaseInsensitive))
            {
                // First line is a header line
                parseHeader(line);
                continue;
            } else if (!header.isNull()) {
                // First line contains (presumably) values, so parse the expected (default) header
                parseHeader(header);
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
    }

    return rows;
}

const CsvParser::Headers &CsvParser::getHeaders() const noexcept
{
    return m_headers;
}

inline void CsvParser::parseHeader(const QString &line) noexcept
{
    m_headers.clear();
    for (const QChar currentChar : line) {
        if (currentChar == m_quoteChar) {
            parseQuote(currentChar);
        } else if (currentChar == m_separatorChar && !m_inQuotation) {
            // Separator in unquoted text - start new value
            parseHeaderSeparator(currentChar);
        } else {
            // Regular character
            m_currentValue.append(currentChar);
            m_lastChar = currentChar;
        }
    }

    // Finish header
    m_headers.insert({getCurrentValue(), m_headers.size()});
    m_currentValue.clear();
    m_currentValueQuoted = false;
}

inline void CsvParser::parseHeaderSeparator(QChar currentChar) noexcept
{
    m_headers.insert({getCurrentValue(), m_headers.size()});
    m_currentValue.clear();
    m_currentValueQuoted = false;
    m_lastChar = currentChar;
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

    for (const QChar currentChar : line) {

        if (currentChar == m_quoteChar) {
            parseQuote(currentChar);
        } else if (currentChar == m_separatorChar && !m_inQuotation) {
            // Separator in unquoted text - start new value
            parseLineSeparator(currentChar);
        } else {
            // Regular character
            m_currentValue.append(currentChar);
            m_lastChar = currentChar;
        }
    }

    if (!m_inQuotation) {
        // Finish line
        m_currentRow.push_back(getCurrentValue());
        m_currentValue.clear();
        m_currentValueQuoted = false;
    }
}

inline void CsvParser::parseLineSeparator(QChar currentChar) noexcept
{
    m_currentRow.push_back(getCurrentValue());
    m_currentValue.clear();
    m_currentValueQuoted = false;
    m_lastChar = currentChar;
}

inline void CsvParser::parseQuote(QChar currentChar) noexcept
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
            m_currentValue.append(currentChar);
        }
        m_inQuotation = true;
    }
    m_lastChar = currentChar;
}

QString CsvParser::getCurrentValue() const noexcept
{
    return (m_trimValue && !m_currentValueQuoted) ? m_currentValue.trimmed() : m_currentValue;
}

inline void CsvParser::reset() noexcept
{
    m_currentRow.clear();
    m_inQuotation = false;
    m_currentValue.clear();
    m_lastChar = '\0';
    m_currentValueQuoted = false;
}
