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
#ifndef NAME_H
#define NAME_H

#include <QString>
#include <QStringLiteral>
#include <QRegularExpression>

/*!
 * Conversion between various naming schemes: camelCase, snake_case.
 *
 * https://wiki.qt.io/Converting_Strings_from_and_to_Camel_Case
 */
namespace Name
{
    /*!
     * Converts the string \c camelCase from camelCase to snake_case.
     *
     * \param camelCase
     *        the string to be converted to snake_case
     * \return the string \c s in snake_case
     */
    QString fromCamelCase(const QString &camelCase)
    {
        static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
        static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

        QString snake_case = camelCase;
        snake_case.replace(regExp1, "\\1_\\2");
        snake_case.replace(regExp2, "\\1_\\2");

        return snake_case.toLower();
    }
}

#endif // NAME_H
