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
    VersionPrivate()
        : major(VersionConfig::Major), minor(VersionConfig::Minor), patch(VersionConfig::Patch)
    {}

    VersionPrivate(int theMajor, int theMinor, int thePatch)
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
const QString VersionPrivate::CodeName = QString("Anarchic Amoeba");
const QString VersionPrivate::UserVersion = QString("21.03");
const QString VersionPrivate::OrganisationName = QString(VersionConfig::OrganisationName);
const QString VersionPrivate::ApplicationName = QString(VersionConfig::ApplicationName);

// public

Version::Version()
    : d(new VersionPrivate())
{
}

Version::Version(int majorNo, int minorNo, int patch)
    : d(new VersionPrivate(majorNo, minorNo, patch))
{
}

Version::Version(const QString &version)
    : d(std::make_unique<VersionPrivate>())
{
    QRegExp versionRegExp("^(\\d+)\\.(\\d+)\\.(\\d+)$");
    if (versionRegExp.indexIn(version) != -1) {
        d->major = versionRegExp.cap(1).toInt();
        d->minor = versionRegExp.cap(2).toInt();
        d->patch = versionRegExp.cap(3).toInt();
    }
}

Version::~Version()
{
}

int Version::getMajor()
{
    return d->major;
}

int Version::getMinor()
{
    return d->minor;
}

int Version::getPatch()
{
    return d->patch;
}

QString Version::toString() const
{
    return QString("%1.%2.%3").arg(d->major).arg(d->minor).arg(d->patch);
}

bool Version::operator==(const Version &other)
{
    bool result;
    result = d->major == other.d->major && d->minor == other.d->minor && d->patch == other.d->patch;
    return result;
}

bool Version::operator>=(const Version &other)
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

bool Version::operator<(const Version &other)
{
    return !(*this >= other);
}

QString Version::getCodeName()
{
    return VersionPrivate::CodeName;
}

QString Version::getUserVersion()
{
    return VersionPrivate::UserVersion;
}

QString Version::getApplicationVersion()
{
    Version version;
    return version.toString();
}

QString Version::getOrganisationName()
{
    return VersionPrivate::OrganisationName;
}

QString Version::getApplicationName()
{
    return VersionPrivate::ApplicationName;
}
