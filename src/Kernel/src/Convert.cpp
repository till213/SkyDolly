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
#include <exception>

#include <QCoreApplication>
#include <QFileInfo>
#ifdef DEBUG
#include <QDebug>
#endif

#include <GeographicLib/Geoid.hpp>

#include "Settings.h"
#include "Convert.h"

// PUBLIC

Convert::Convert() noexcept
{
    const QFileInfo earthGravityModelFileInfo = Settings::getInstance().getEarthGravityModelFileInfo();
    if (earthGravityModelFileInfo.exists()) {
        try {
            const std::string egmName = earthGravityModelFileInfo.baseName().toStdString();
            const std::string egmDirectory = earthGravityModelFileInfo.absolutePath().toStdString();
            m_egm = std::make_unique<GeographicLib::Geoid>(egmName, egmDirectory);
        } catch (const std::exception &ex) {
            m_egm = nullptr;
#ifdef DEBUG
            qDebug() << "Convert::Convert: caught exception: " << ex.what();
#endif
        }
    } else {
        m_egm = nullptr;
    }
}

Convert::~Convert() noexcept
{}
