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
#ifndef CSVCONST_H
#define CSVCONST_H

namespace CsvConst {

    /*! Separator character for Sky Dolly CSV import & export */
    constexpr char TabSep = '\t';
    /*! Comma separator character */
    constexpr char CommaSep = ',';
    /*! Double-quote character */
    constexpr char DoubleQuote = '\"';
    /*! Newline character */
    constexpr char Ln = '\n';

    // Format and precision for double
    constexpr char Format = 'g';
    constexpr int Precision = 9;

    enum struct DataType : char {
        Aircraft = 'a',
        Engine = 'e',
        PrimaryFlightControl = 'p',
        SecondaryFlightControl = 's',
        AircraftHandle = 'h',
        Light = 'l'
    };

    constexpr char TypeColumnName[] = "Type";
}

#endif // CSVCONST_H