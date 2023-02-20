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
#include <memory>

#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QChar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDateTime>

#include <GitInfo.h>
#include <VersionConfig.h>
#include "Version.h"

struct VersionPrivate
{
    VersionPrivate(int major = VersionConfig::Major, int minor = VersionConfig::Minor, int patch = VersionConfig::Patch) noexcept
        : major(major), minor(minor), patch(patch)
    {}

    int major {VersionConfig::Major};
    int minor {VersionConfig::Minor};
    int patch {VersionConfig::Patch};

    // https://grammar.yourdictionary.com/parts-of-speech/adjectives/Adjectives-That-Start-With-F.html
    // https://en.wikipedia.org/wiki/List_of_aircraft_(G)
    static inline const QString CodeName {QStringLiteral("Gregarious Gee Bee")};
};

// PUBLIC

Version::Version() noexcept
    : d(std::make_unique<VersionPrivate>())
{}

Version::Version(int majorNo, int minorNo, int patch) noexcept
    : d(std::make_unique<VersionPrivate>(majorNo, minorNo, patch))
{}

Version::Version(QStringView version) noexcept
    : d(std::make_unique<VersionPrivate>())
{
    fromString(version);
}

Version::Version(Version &&rhs) noexcept = default;
Version &Version::operator=(Version &&rhs) noexcept = default;
Version::~Version() = default;

void Version::fromString(QStringView version) noexcept
{
    static QRegularExpression versionRegExp(R"(^(\d+)\.(\d+)\.(\d+)$)");
    QRegularExpressionMatch match = versionRegExp.match(version);
    if (match.isValid()) {
        d->major = match.captured(1).toInt();
        d->minor = match.captured(2).toInt();
        d->patch = match.captured(3).toInt();
    }
}

int Version::getMajor() const noexcept
{
    return d->major;
}

int Version::getMinor() const noexcept
{
    return d->minor;
}

int Version::getPatch() const noexcept
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

bool Version::operator==(const Version &rhs) const noexcept
{
    return d->major == rhs.d->major && d->minor == rhs.d->minor && d->patch == rhs.d->patch;
}

bool Version::operator>=(const Version &rhs) const noexcept
{
    bool result {false};
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

bool Version::operator<(const Version &rhs) const noexcept
{
    return !(*this >= rhs);
}

QString Version::getCodeName() noexcept
{
    return VersionPrivate::CodeName;
}

QString Version::getUserVersion() noexcept
{
    QString userVersion;
    const QDate gitDate = getGitDate().date();
    const int year = gitDate.year();
    const int month = gitDate.month();
    userVersion = QString("%1").arg(year) % "." % QString("%1").arg(month, 2, 10, QChar('0'));
    return userVersion;
}

QString Version::getApplicationVersion() noexcept
{
    Version version;
    return version.toString();
}

QString Version::getOrganisationName() noexcept
{
    return VersionConfig::OrganisationName;
}

QString Version::getApplicationName() noexcept
{
    return VersionConfig::ApplicationName;
}

QLatin1String Version::getGitHash() noexcept
{
    return QLatin1String(GitInfo::GitHash);
}

QDateTime Version::getGitDate() noexcept
{
    return QDateTime::fromString(GitInfo::GitIsoDate, Qt::ISODate);
}
