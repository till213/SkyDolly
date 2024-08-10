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
#include <mutex>
#include <memory>

#include <QObject>
#include <QFileInfo>
#include <QStringList>
#include <QList>
#include <QSettings>
#ifdef DEBUG
#include <QDebug>
#endif

#include "SecurityToken.h"
#include "RecentFile.h"

namespace
{
    constexpr int InvalidIndex {-1};
}

struct RecentFilePrivate {
    static const int DefaultMaxRecentFiles;
    static const int MaxRecentFiles;

    QSettings settings;
    QStringList recentFiles;             // the items in recentFiles must always be in sync
    QList<QByteArray> securityTokenData; // with the items in securityTokenData
    int maxRecentFiles {0};

    static inline std::once_flag onceFlag;
    static inline std::unique_ptr<RecentFile> instance;
};

const int RecentFilePrivate::DefaultMaxRecentFiles = 8;
const int RecentFilePrivate::MaxRecentFiles = 10; // There are 10 action shortcuts keys: 0...9

// PUBLIC

RecentFile &RecentFile::getInstance()
{
    std::call_once(RecentFilePrivate::onceFlag, []() {
        RecentFilePrivate::instance = std::unique_ptr<RecentFile>(new RecentFile());
    });
    return *RecentFilePrivate::instance;
}

void RecentFile::destroyInstance()
{
    if (RecentFilePrivate::instance != nullptr) {
        RecentFilePrivate::instance.reset();
    }
}

void RecentFile::addRecentFile(const QString &filePath)
{
    bool added = prependToRecentFiles(filePath);
    if (added) {
        QByteArray securityTokenData = SecurityToken::createSecurityTokenData(filePath);
        d->securityTokenData.prepend(securityTokenData);
        if (d->recentFiles.count() > d->maxRecentFiles) {
            d->recentFiles.removeLast();
            d->securityTokenData.removeLast();
        }
        emit recentFilesChanged();
    }
}

void RecentFile::removeRecentFile(const QString &filePath)
{
    int index = removeFromRecentFiles(filePath);
    if (index != ::InvalidIndex) {
        d->securityTokenData.removeAt(index);
        emit recentFilesChanged();
    }
}

void RecentFile::moveToFront(const QString &filePath)
{
    int index = removeFromRecentFiles(filePath);
    QByteArray securityTokenData;
    if (index != ::InvalidIndex) {
        // helper functions do not emit signals...
        prependToRecentFiles(filePath);
        // ... nor modify the security token data...
        securityTokenData = d->securityTokenData.at(index);

        // ... so we have to move the token data separately
        d->securityTokenData.removeAt(index);
        d->securityTokenData.prepend(securityTokenData);
    }
}

void RecentFile::clear()
{
    if (d->recentFiles.count() > 0) {
        d->recentFiles.clear();
        d->securityTokenData.clear();
        emit recentFilesChanged();
    }
}

const QStringList &RecentFile::getRecentFiles() const
{
    return d->recentFiles;
}

int RecentFile::getMaxRecentFiles() const
{
    return d->maxRecentFiles;
}

void RecentFile::setMaxRecentFiles(int maxRecentFiles)
{
    if (d->maxRecentFiles != maxRecentFiles && maxRecentFiles >= 1 && maxRecentFiles <= RecentFilePrivate::MaxRecentFiles) {
        if (d->maxRecentFiles > maxRecentFiles) {
            while (d->recentFiles.count() > maxRecentFiles) {
                d->recentFiles.removeLast();
                d->securityTokenData.removeLast();
            }
        }
        d->maxRecentFiles = maxRecentFiles;
        emit maxRecentFilesChanged(d->maxRecentFiles);
    }
}

void RecentFile::selectRecentFile(const QString &filePath)
{
    QByteArray securityTokenData;
    moveToFront(filePath);
    securityTokenData = d->securityTokenData.first();
    SecurityToken *securityToken = SecurityToken::create(securityTokenData);
    emit recentFileSelected(filePath, securityToken);
    securityToken->release();
    securityToken = nullptr;
}

// PRIVATE

RecentFile::RecentFile()
    : d(std::make_unique<RecentFilePrivate>())
{
    restore();
}

RecentFile::~RecentFile()
{
#ifdef DEBUG
    qDebug() << "RecentFile::~RecentFile: DELETED";
#endif
    store();
}

void RecentFile::store()
{
    int count;
    d->settings.beginGroup("Logbook");
    {
        d->settings.beginGroup("Recent");
        {
            d->settings.setValue("MaxRecentFiles", d->maxRecentFiles);
            d->settings.setValue("RecentFiles", d->recentFiles);
            count = d->securityTokenData.count();
            d->settings.beginWriteArray("Security", count);
            {
                for (int i = 0; i < count; ++i) {
                    d->settings.setArrayIndex(i);
                    d->settings.setValue("Data", d->securityTokenData.at(i));
                }
            }
            d->settings.endArray();
        }
        d->settings.endGroup();
    }
    d->settings.endGroup();
}

void RecentFile::restore()
{
    int count;
    QByteArray data;

    d->settings.beginGroup("Logbook");
    {
        d->settings.beginGroup("Recent");
        {
            d->maxRecentFiles = d->settings.value("MaxRecentFiles", RecentFilePrivate::DefaultMaxRecentFiles).toInt();
            d->recentFiles = d->settings.value("RecentFiles", QStringList()).toStringList();
            count = d->settings.beginReadArray("Security");
            {
                for (int i = 0; i < count; ++i) {
                    d->settings.setArrayIndex(i);
                    data = d->settings.value(("Data")).toByteArray();
                    d->securityTokenData.append(data);
                }
            }
            d->settings.endArray();
        }
        d->settings.endGroup();
    }
    d->settings.endGroup();

    reconcileData();
}

bool RecentFile::prependToRecentFiles(const QString &filePath)
{
    bool result {false};
    if (!d->recentFiles.contains(filePath)) {
        d->recentFiles.prepend(filePath);
        if (d->recentFiles.count() > d->maxRecentFiles) {
            d->recentFiles.removeLast();
            d->securityTokenData.removeLast();
        }
        result = true;
    }
    return result;
}

int RecentFile::removeFromRecentFiles(const QString &filePath)
{
    int index = d->recentFiles.indexOf(filePath);
    if (index != ::InvalidIndex) {
        d->recentFiles.removeAt(index);
    }
    return index;
}

void RecentFile::reconcileData()
{
    int nofSecurityTokenData;
    int n;

    n = d->recentFiles.count();
    nofSecurityTokenData = d->securityTokenData.count();
    while (n > nofSecurityTokenData) {
        d->recentFiles.removeLast();
        --n;
    }
}

