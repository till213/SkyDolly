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
#include <QRegularExpression>
#include <QStringList>
#include <QString>
#include <QStringView>
#ifdef DEBUG
#include <QDebug>
#endif

#include <GeographicLib/DMS.hpp>

#include "PositionParser.h"

namespace
{
    QStringList parseComponents(QStringView value) noexcept
    {
        static QRegularExpression numberRexExp(R"(^([+-]?[0-9]*[.]?[0-9]+)[,]?[\s]*([+-]?[0-9]*[.]?[0-9]+)$)");
        static QRegularExpression dmsRegExp(R"(^([\d\W]+[N|S|E|W])[,]?([\d\W]+[E|W|N|S])$)");
        QStringList values;

        QStringView trimmedValue = value.trimmed();
        // First try to match (possibly comma-separated) floating point numbers
        // (e.g. 46.94697890467696, 7.444134280004356)
        QRegularExpressionMatch match = numberRexExp.match(trimmedValue);
        if (match.hasMatch() && match.lastCapturedIndex() == 2) {
            values << match.captured(1).trimmed();
            values << match.captured(2).trimmed();
        } else {
            // Try parsing latitude/longitude DMS values
            // (e.g. 46° 56' 52.519" N 7° 26' 40.589" E or 7° 26' 40.589" E, 46° 56' 52.519" N)
            match = dmsRegExp.match(trimmedValue);
            if (match.hasMatch() && match.lastCapturedIndex() == 2) {
                // GeographicLib DMS does not like whitespace in DMS strings
                values << match.captured(1).replace(" ", "");
                values << match.captured(2).replace(" ", "");
            }
        }
        return values;
    }
}

// PUBLIC

PositionParser::Coordinate PositionParser::parse(QStringView position, bool *ok) noexcept
{
    Coordinate coordinate {0.0, 0.0};
    const QStringList values = parseComponents(position);
    if (values.count() == 2) {
        try {
            const std::string first = values.first().toStdString();
            const std::string second = values.last().trimmed().toStdString();
            GeographicLib::DMS::flag flag {GeographicLib::DMS::NONE};
            double value = GeographicLib::DMS::Decode(first, flag);
            if (flag == GeographicLib::DMS::LATITUDE || flag == GeographicLib::DMS::NONE) {
                coordinate.first = value;
            } else {
                coordinate.second = value;
            }
            value = GeographicLib::DMS::Decode(second, flag);
            if (flag == GeographicLib::DMS::LONGITUDE || flag == GeographicLib::DMS::NONE) {
                coordinate.second = value;
            } else {
                coordinate.first = value;
            }
            if (ok != nullptr) {
                *ok = true;
            }
        } catch (GeographicLib::GeographicErr err) {
            if (ok != nullptr) {
                *ok = false;
            }
#ifdef DEBUG
        qDebug() << "PositionParser::parse: Not a coordinate:" << err.what();
#endif
        }
    } else if (ok != nullptr) {
        *ok = false;
    }
    return coordinate;
}
