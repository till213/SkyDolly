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
    static const QString ApplicationTitle;
};

// Application version
const QString VersionPrivate::CodeName = QString("Anarchic Amoeba");
const QString VersionPrivate::UserVersion = QString("21.02");
// note: no translation here (i18n)
const QString VersionPrivate::ApplicationTitle = QString("Sky Dolly");

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
    : d(new VersionPrivate())
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
    delete d;
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

QString Version::toString()
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

QString Version::getApplicationName()
{
    return VersionPrivate::ApplicationTitle;
}
