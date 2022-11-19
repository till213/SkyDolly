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
#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <memory>
#include <vector>

#include <QString>
#include <QChar>

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

    explicit CsvParser(QChar separatorChar = ',', QChar escapeChar = '"', bool trimValues = true);
    CsvParser(const CsvParser &rhs) = delete;
    CsvParser(CsvParser &&rhs);
    CsvParser &operator=(const CsvParser &rhs) = delete;
    CsvParser &operator=(CsvParser &&rhs);
    ~CsvParser();

    /*!
     * Parses the \c textStream as comma-separated values (CSV). The first row is ignored
     * if it begins with either \c header or \c alternateHeader.
     *
     * \param textStream
     *        the CSV to be parsed
     * \param header
     *        the header of the values (case-insensitive)
     * \param alternateHeader
     *        the alternate header of the values (case-insensitive); useful if the header
     *        may contain different escape characters
     * \return the CSV rows
     */
    Rows parse(QTextStream &textStream, const QString &header = QString(), const QString &alternateHeader = QString()) noexcept;

private:
    std::unique_ptr<CsvParserPrivate> d;

    inline void parseLine(const QString &line) noexcept;
    inline void parseQuote() noexcept;
    inline void parseSeparator() noexcept;
    inline void reset() noexcept;
};

#endif // CSVPARSER_H
