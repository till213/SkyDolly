/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QActionGroup>
#include <QAction>
#include <QKeySequence>
#include <QFileInfo>
#include <QVector>

#include <Kernel/RecentFile.h>
#include "RecentFileMenu.h"

struct RecentFileMenuPrivate
{
    RecentFileMenuPrivate(QObject *parent)
        : recentFileActionGroup(new QActionGroup {parent})
    {}

    QActionGroup *recentFileActionGroup;
    QAction *clearRecentFileAction {nullptr};
};

// PUBLIC

RecentFileMenu::RecentFileMenu(QObject *parent) :
    QObject {parent}
{
    d = new RecentFileMenuPrivate(parent);
    initialise();
    frenchConnections();
}

RecentFileMenu::~RecentFileMenu()
{
    delete d;
}

QActionGroup &RecentFileMenu::getRecentFileActionGroup() const
{
    return *d->recentFileActionGroup;
}

// PRIVATE

void RecentFileMenu::initialise()
{
    const int maxRecentFiles = RecentFile::getInstance().getMaxRecentFiles();
    updateNofRecentFileActions(maxRecentFiles);
    updateRecentFileActions();

    auto *separator = new QAction(d->recentFileActionGroup);
    separator->setSeparator(true);
    d->clearRecentFileAction = new QAction(tr("Clear &Menu"), d->recentFileActionGroup);
}

void RecentFileMenu::frenchConnections()
{
    RecentFile &recentFile = RecentFile::getInstance();
    connect(&recentFile, &RecentFile::recentFilesChanged,
            this, &RecentFileMenu::updateRecentFileActions);
    connect(&recentFile, &RecentFile::maxRecentFilesChanged,
            this, &RecentFileMenu::updateNofRecentFileActions);
    connect(d->clearRecentFileAction, &QAction::triggered,
            this, &RecentFileMenu::clearRecentFileMenu);
}

// PRIVATE SLOTS

void RecentFileMenu::updateRecentFileActions()
{
    RecentFile &recentFile = RecentFile::getInstance();
    const QStringList &recentFiles = recentFile.getRecentFiles();
    const QList<QAction *> recentFileActions = d->recentFileActionGroup->actions();

    const int nofRecentFiles = recentFiles.count();
    for (int i = 0; i < nofRecentFiles; ++i) {
        const QFileInfo fileInfo(recentFiles[i]);
        const QString text = fileInfo.fileName();

        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(recentFiles[i]);
        recentFileActions[i]->setVisible(true);
    }

    // Set all not yet used actions to invisible
    const int maxRecentFiles = recentFile.getMaxRecentFiles();
    for (int i = nofRecentFiles; i < maxRecentFiles; ++i) {
        recentFileActions[i]->setVisible(false);
    }
}

void RecentFileMenu::updateNofRecentFileActions(int maxRecentFiles)
{
    int nofRecentFilesActions;
    QList<QAction *> recentFileActions {d->recentFileActionGroup->actions()};
    QAction *action {nullptr};
    bool changed {false};

    nofRecentFilesActions = recentFileActions.count() - 2; // two extra actions: separator and "clear" entry

    // Add menu entries
    for (int i = nofRecentFilesActions; i < maxRecentFiles; ++i) {
        action = new QAction(d->recentFileActionGroup);
        action->setVisible(false);
        connect(action, &QAction::triggered,
                this, &RecentFileMenu::onRecentFileAction);
        changed = true;
    }

    // Remove menu entries - index starts at 0 (-> -1)
    for (int i = nofRecentFilesActions - 1; i >= maxRecentFiles; --i) {
        delete recentFileActions.at(i);
        recentFileActions.removeAt(i);
        changed = true;
    }

    if (changed) {
        emit actionGroupChanged();
    }
}

void RecentFileMenu::onRecentFileAction()
{
    RecentFile &recentFile = RecentFile::getInstance();

    if (const QAction *action = qobject_cast<const QAction *>(sender())) {
        QString filePath = action->data().toString();
        recentFile.selectRecentFile(filePath);
    }
}

void RecentFileMenu::clearRecentFileMenu()
{
    RecentFile::getInstance().clear();
}
