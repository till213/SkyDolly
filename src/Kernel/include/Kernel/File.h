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
#ifndef FILE_H
#define FILE_H

#include <QString>
#include <QStringView>
#include <QStringList>

class QString;

#include "KernelLib.h"

class KERNEL_API File final
{
public:

    /*!
     * Ensures that the given \c filePath has the suffix \c suffix, which is case-insensitive.
     *
     * \param filePath
     *        the file path to be checked for suffix \c suffix
     * \param suffix
     *        the case-insensitive suffix
     * \return the \c filePath with guaranteed suffix \c suffix
     */
    static QString ensureSuffix(QStringView filePath, QStringView suffix) noexcept;

    /*!
     * Returns the \c filePath with a sequence number \c n appended to it (before the suffix).
     * When \c n is equal 1
     *
     * Examples:
     *
     * filePath               | n | result
     * -----------------------|---|-------------------------
     * /some/path/example.csv | 1 | /some/path/example-1.csv
     * -----------------------|---|-------------------------
     * /some/path/example.csv | 2 | /some/path/example-2.csv
     * -----------------------|---|-------------------------
     * /some/path/example.csv | n | /some/path/example-n.csv
     *
     * \param filePath
     *        the file path including file suffix
     * \param n
     *        the sequence number; while also negative values are acceptable meaningful sequence
     *        numbers should start with 1 and increase
     * \return the \c filePath with a sequence number
     */
    static QString getSequenceFilePath(const QString &filePath, int n) noexcept;

    static QStringList getFilePaths(const QString &directoryPath, QStringView suffix) noexcept;
};

#endif // FILE_H
