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
#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QObject>

class QString;
class QActionGroup;

class SecurityToken;
struct RecentFilePrivate;

#include "KernelLib.h"

/*!
 * \brief The recently opened files.
 *
 * Keeps track of recently opened files.
 */
class KERNEL_API RecentFile : public QObject
{
    Q_OBJECT
public:
    static RecentFile &getInstance();
    static void destroyInstance();

    /*!
     * \sa #recentFilesChanged()
     */
    void addRecentFile(const QString &filePath);

    /*!
     * \sa #recentFilesChanged()
     */
    void removeRecentFile(const QString &filePath);

    /*!
     * Moves the \em existing \p filePath to the most recent file position.
     * The \p filePath is not added in case it does not already exist in the
     * recent files list.
     *
     * \sa #recentFilesChanged()
     */
    void moveToFront(const QString &filePath);

    const QStringList &getRecentFiles() const;

    /*!
     * Clears the list of recent files.
     *
     * \sa #recentFilesChanged()
     */
    void clear();

    int getMaxRecentFiles() const;

    /*!
     * Sets the maximum menu entries of recent files which must be in [1, 10].
     *
     * \sa #maxRecentFilesChanged(int)
     */
    void setMaxRecentFiles(int maxRecentFile);

    /*!
     * This will select \p filePath for opening. The \p filePath is moved to front
     * of the recent file list and the signal #recentFileSelected is emitted.
     *
     * \param filePath
     *        the file path to be opened
     * \sa #recentFileSelected(const QString &, SecurityToken *)
     */
    void selectRecentFile(const QString &filePath);

signals:
    /*!
     * Emitted whenever a file from the recent file list has
     * been selected. The \p securityToken gives access to the \p filePath for
     * as long as the \p SecurityToken exists.
     *
     * If the \p filePath is to be read only within the scope of the corresponding
     * slot and no further processing with that file is planned then no further ado
     * is necessary.
     *
     * If on the other hand the file is to be saved later on under the same
     * \p filePath then SecurityToken#retain must be called
     * and the reference to the \p securityToken must be stored somewhere. As soon
     * as the file is closed the \p securityToken must then be released with
     * SecurityToken#release. Otherwise not only memory but also system kernel
     * resources will be leaked.
     *
     * \param filePath
     *        the file path selected from the recent file times
     * \param securityToken
     *        the security token which gives access to the \p filePath for as long
     *        as the security token exists; call SecurityToken#retain
     *        for further file access and and SecurityToken#release once no more
     *        file access is needed
     */
    void recentFileSelected(const QString &filePath, SecurityToken *securityToken);

    /*!
     * Emitted whenever the recent file list has changed. Existing QActionGroup
     * entries must be set in-/visible.
     */
    void recentFilesChanged();

    /*!
     * Emitted whenever the maximum number of recent files has changed. In this
     * cases new QAction entries must be either created or existing ones must
     * be deleted. In any case the application menu must be updated accordingly.
     *
     * \param maxRecentFiles
     *        the new number of maximum recent files
     */
    void maxRecentFilesChanged(int maxRecentFiles);

private:
    Q_DISABLE_COPY(RecentFile)
    std::unique_ptr<RecentFilePrivate> d;

    RecentFile();
    friend std::unique_ptr<RecentFile>::deleter_type;
    ~RecentFile() override;

    void initialise();
    void frenchConnection();

    void store();
    void restore();

    // helper function which only modifies the 'recentFiles' list without
    // affecting the security tokens; returns true if 'filePath' was not
    // already contained in the list, in which case a corresponding security
    // token needs to be created and prepended to the security token list
    bool prependToRecentFiles(const QString &filePath);

    // helper function which only modifies the 'recentFiles' list without
    // affecting the security tokens; they need to be adjusted according
    // to the returned original index (the original location of 'filePath');
    // the returned index is -1 if 'filePath' was not contained in the list
    int removeFromRecentFiles(const QString &filePath);

    // make sure the number of 'recentFiles' and 'securityTokenData' is the
    // same, in case the application settings have been tampered with
    void reconcileData();
};

#endif // RECENTFILE_H
