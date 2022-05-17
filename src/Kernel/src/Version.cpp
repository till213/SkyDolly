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

#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

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

    static inline const QString CodeName {QStringLiteral("Beasty Boeing")};
    static inline const QString UserVersion {QStringLiteral("22.02")};
};

// PUBLIC

Version::Version() noexcept
    : d(std::make_unique<VersionPrivate>())
{}

Version::Version(int majorNo, int minorNo, int patch) noexcept
    : d(new VersionPrivate(majorNo, minorNo, patch))
{}

Version::Version(QStringView version) noexcept
    : d(std::make_unique<VersionPrivate>())
{
    fromString(version);
}

void Version::fromString(QStringView version) noexcept
{
    QRegularExpression versionRegExp("^(\\d+)\\.(\\d+)\\.(\\d+)$");
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QRegularExpressionMatch match = versionRegExp.match(version.toString());
#else
    QRegularExpressionMatch match = versionRegExp.match(version);
#endif
    if (match.isValid()) {
        d->major = match.captured(1).toInt();
        d->minor = match.captured(2).toInt();
        d->patch = match.captured(3).toInt();
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

bool Version::isNull() const noexcept
{
    return d->major == 0 && d->minor == 0 && d->patch == 0;
}

void Version::operator = (const Version &rhs) noexcept
{
    d = std::make_unique<VersionPrivate>(rhs.d->major, rhs.d->minor, rhs.d->patch);
}

bool Version::operator == (const Version &rhs) noexcept
{
    bool result;
    result = d->major == rhs.d->major && d->minor == rhs.d->minor && d->patch == rhs.d->patch;
    return result;
}

bool Version::operator>=(const Version &rhs) noexcept
{
    bool result;
    if (d->major > rhs.d->major) {
        result = true;
    } else if (d->major < rhs.d->major) {
        result = false;
    } else {
        if (d->minor > rhs.d->minor) {
            result = true;
        } else if (d->minor < rhs.d->minor) {
            result = false;
        } else {
            if (d->patch >= rhs.d->patch) {
                result = true;
            } else {
                result = false;
            }
        }
    }
    return result;
}

bool Version::operator<(const Version &rhs) noexcept
{
    return !(*this >= rhs);
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
    return VersionConfig::OrganisationName;
}

const QString Version::getApplicationName() noexcept
{
    return VersionConfig::ApplicationName;
}
