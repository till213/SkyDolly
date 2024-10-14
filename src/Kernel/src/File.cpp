/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

    constexpr const char *ResourceDirectoryName {"Resources"};
    // This happens to be the same directory name as when unzipping the downloaded EGM data
    // from https://geographiclib.sourceforge.io/html/geoid.html#geoidinst
    constexpr const char *EgmDirectoryName {"geoids"};
    constexpr const char *DefaultEgmFileName {"egm2008-5.pgm"};

    constexpr const char *SimConnectConfigurationFileName {"SimConnect.cfg"};
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
     for (const auto &fileName : fileNames) {
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
     pluginsDirectory.cd(::PluginDirectoryName);
     return pluginsDirectory.absolutePath();
 }

 QFileInfo File::getEarthGravityModelFileInfo() noexcept
 {
     QFileInfo egmFileInfo;
     QDir egmDirectory = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
     if (egmDirectory.dirName() == "MacOS") {
         // Navigate up the app bundle structure, into the Contents folder
         egmDirectory.cdUp();
     }
#endif
     if (egmDirectory.cd(::ResourceDirectoryName)) {
         if (egmDirectory.cd(::EgmDirectoryName)) {
             if (egmDirectory.exists(::DefaultEgmFileName)) {
                 egmFileInfo = QFileInfo(
                     egmDirectory.absoluteFilePath(::DefaultEgmFileName));
             }
         }
     }
     return egmFileInfo;
 }

 bool File::hasEarthGravityModel() noexcept
 {
     return getEarthGravityModelFileInfo().exists();
 }

 bool File::hasSimConnectConfiguration() noexcept
 {
     QDir applicationDirectory = QDir(QCoreApplication::applicationDirPath());
     return applicationDirectory.exists(::SimConnectConfigurationFileName);
 }
