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
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <QString>
#include <QChar>
#include <QHash>

class QTextStream;

#include "KernelLib.h"

struct CsvParserPrivate;

/*
 * Provides functionality to read CSV files including escaped lines with linefeeds and more.
 */
class KERNEL_API CsvParser final
{
public:
    using Row = std::vector<QString>;
    using Rows = std::vector<Row>;
    using Headers = std::unordered_map<QString, std::size_t>;

    explicit CsvParser(QChar separatorChar = ',', QChar quoteChar = '"', bool trimValues = true);
    CsvParser(const CsvParser &rhs) = delete;
    CsvParser(CsvParser &&rhs) = default;
    CsvParser &operator=(const CsvParser &rhs) = delete;
    CsvParser &operator=(CsvParser &&rhs) = default;
    ~CsvParser() = default;

    /*!
     * Parses the \p textStream as comma-separated values (CSV). The first row is ignored
     * if it begins with either \p header or \p alternateHeader.
     *
     * \param textStream
     *        the CSV to be parsed
     * \param header
     *        the header of the values (case-insensitive)
     * \param alternateHeader
     *        the alternate header of the values (case-insensitive); useful if the header
     *        may contain different escape characters
     * \return the CSV rows, without the header row (if present)
     */
    Rows parse(QTextStream &textStream, const QString &header = QString(), const QString &alternateHeader = QString()) noexcept;

    /*!
     * Returns the headers and their column indices from a previous parsing. Column
     * index numbering starts at 0.
     *
     * Also refer to #parse.
     *
     * \return the parsed heades (names) and their column indices, starting at 0; an
     *         empty collection if no headers were present or no parsing has been done yet
     */
    const Headers &getHeaders() const noexcept;

    /*!
     * Validates that there exists at least one Row in \p rows and that each Row has
     * at least the \p expectedColumnCount.
     *
     * \param rows
     *        the previously parsed CSV rows
     * \param expectedMinimumColumnCount
     *        the expected minimum column count of each row
     * \return \c true if each row in Rows has \p expectedColumnCount items; \c false else
     */
    static inline bool validate(const Rows &rows, std::size_t expectedMinimumColumnCount) noexcept
    {
        bool ok = rows.size() > 0;
        if (ok) {
            auto condition = [expectedMinimumColumnCount](const Row &row) {return row.size() < expectedMinimumColumnCount;};
            ok = std::find_if(rows.begin(), rows.end(), condition) == rows.end();
        }
        return ok;
    };

private:
    Row m_currentRow;
    Headers m_headers;
    QString m_currentValue;
    QChar m_separatorChar;
    QChar m_quoteChar;
    QChar m_lastChar {'\0'};

    bool m_trimValue {false};
    bool m_inQuotation {false};
    bool m_currentValueQuoted {false};

    inline void parseHeader(const QString &line) noexcept;
    inline void parseHeaderSeparator(QChar currentChar) noexcept;

    inline void parseLine(const QString &line) noexcept;
    inline void parseLineSeparator(QChar currentChar) noexcept;

    inline void parseQuote(QChar currentChar) noexcept;
    inline QString getCurrentValue() const noexcept;
    inline void reset() noexcept;
};

#endif // CSVPARSER_H
