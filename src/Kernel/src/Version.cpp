/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include <QString>
#include <QRegExp>

#include <VersionConfig.h>
#include "Version.h"

class VersionPrivate
{
public:
    VersionPrivate() noexcept
        : major(VersionConfig::Major), minor(VersionConfig::Minor), patch(VersionConfig::Patch)
    {}

    VersionPrivate(int theMajor, int theMinor, int thePatch) noexcept
        : major(theMajor), minor(theMinor), patch(thePatch)
    {}

    int major;
    int minor;
    int patch;

    static const QString CodeName;
    static const QString UserVersion;
    static const QString OrganisationName;
    static const QString ApplicationName;
};

// Application version
const QString VersionPrivate::CodeName = QString("Anarchic Airbus");
const QString VersionPrivate::UserVersion = QString("21.05");
const QString VersionPrivate::OrganisationName = QString(VersionConfig::OrganisationName);
const QString VersionPrivate::ApplicationName = QString(VersionConfig::ApplicationName);

// PUBLIC

Version::Version() noexcept
    : d(std::make_unique<VersionPrivate>())
{}

Version::Version(int majorNo, int minorNo, int patch) noexcept
    : d(new VersionPrivate(majorNo, minorNo, patch))
{}

Version::Version(const QString &version) noexcept
    : d(std::make_unique<VersionPrivate>())
{
    fromString(version);
}

void Version::fromString(const QString &version) noexcept
{
    QRegExp versionRegExp("^(\\d+)\\.(\\d+)\\.(\\d+)$");
    if (versionRegExp.indexIn(version) != -1) {
        d->major = versionRegExp.cap(1).toInt();
        d->minor = versionRegExp.cap(2).toInt();
        d->patch = versionRegExp.cap(3).toInt();
    }
}

Version::~Version() noexcept
{}

int Version::getMajor() noexcept
{
    return d->major;
}

int Version::getMinor() noexcept
{
    return d->minor;
}

int Version::getPatch() noexcept
{
    return d->patch;
}

QString Version::toString() const noexcept
{
    return QString("%1.%2.%3").arg(d->major).arg(d->minor).arg(d->patch);
}

bool Version::operator==(const Version &other) noexcept
{
    bool result;
    result = d->major == other.d->major && d->minor == other.d->minor && d->patch == other.d->patch;
    return result;
}

bool Version::operator>=(const Version &other) noexcept
{
    bool result;
    if (d->major > other.d->major) {
        result = true;
    } else if (d->major < other.d->major) {
        result = false;
    } else {
        if (d->minor > other.d->minor) {
            result = true;
        } else if (d->minor < other.d->minor) {
            result = false;
        } else {
            if (d->patch >= other.d->patch) {
                result = true;
            } else {
                result = false;
            }
        }
    }
    return result;
}

bool Version::operator<(const Version &other) noexcept
{
    return !(*this >= other);
}

const QString Version::getCodeName() noexcept
{
    return VersionPrivate::CodeName;
}

const QString Version::getUserVersion() noexcept
{
    return VersionPrivate::UserVersion;
}

const QString Version::getApplicationVersion() noexcept
{
    Version version;
    return version.toString();
}

const QString Version::getOrganisationName() noexcept
{
    return VersionPrivate::OrganisationName;
}

const QString Version::getApplicationName() noexcept
{
    return VersionPrivate::ApplicationName;
}
