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
#include <QCoreApplication>
#include <QString>
#include <QStringView>
#include <QStringBuilder>
#include <QStringBuilder>
#include <QFileInfo>
#include <QDir>

#include "File.h"

namespace
{
#if defined(Q_OS_MAC)
    constexpr const char *PluginDirectoryName {"PlugIns"};
#else
    constexpr const char *PluginDirectoryName {"Plugins"};
#endif
}

// PUBLIC

QString File::ensureExtension(QStringView filePath, QStringView extension) noexcept
{
    QString filePathWithExtension;
    if (!filePath.endsWith(extension, Qt::CaseInsensitive)) {
        filePathWithExtension = filePath % "." % extension;
    } else {
        filePathWithExtension = filePath.toString();
    }
    return filePathWithExtension;
}

QString File::getSequenceFilePath(const QString &filePath, int n) noexcept
{
    const QFileInfo fileInfo {filePath};
    const QString baseName = fileInfo.baseName();
    const QString absolutePath = fileInfo.absolutePath();
    const QString extension = fileInfo.completeSuffix();

    return absolutePath % "/" % baseName % "-" % QString::number(n) % "." % extension;
}

 QStringList File::getFilePaths(const QString &directoryPath, QStringView extension) noexcept
 {
     QStringList filePaths;
     const QString nameFilter = "*." % extension;
     QDir dir {directoryPath, nameFilter};

     const QStringList fileNames = dir.entryList(QDir::Files, QDir::SortFlag::Time);
     filePaths.reserve(fileNames.count());
     for (const QString &fileName : fileNames) {
         filePaths.append(dir.absoluteFilePath(fileName));
     }

     return filePaths;
 }

 QString File::getPluginDirectoryPath() noexcept
 {
     QDir pluginsDirectory;

     pluginsDirectory.setPath(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
     if (pluginsDirectory.dirName() == "MacOS") {
         // Navigate up the app bundle structure, into the Contents folder
         pluginsDirectory.cdUp();
     }
#endif
     pluginsDirectory.cd(QString::fromLatin1(::PluginDirectoryName));
     return pluginsDirectory.absolutePath();
 }
