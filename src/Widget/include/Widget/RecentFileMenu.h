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
#ifndef RECENTFILEMENU_H
#define RECENTFILEMENU_H

#include <QObject>
#include <QActionGroup>

#include "WidgetLib.h"

struct RecentFileMenuPrivate;

/*!
 * \brief Provides QAction menu entries for a recent files menu.
 *
 * The QActions are updated by connecting to the corresponding changes
 * in the RecentFile instance.
 */
class WIDGET_API RecentFileMenu : public QObject
{
    Q_OBJECT
public:
    explicit RecentFileMenu(QObject *parent = 0);
    virtual ~RecentFileMenu();

    QActionGroup &getRecentFileActionGroup() const;

signals:
    /*!
     * Emitted whenever the QActionGroup has changed, that is when QAction items
     * have been added or removed.
     */
    void actionGroupChanged();

private:
    Q_DISABLE_COPY(RecentFileMenu)

    RecentFileMenuPrivate *d;

    void initialise();
    void frenchConnections();

private slots:
    void updateRecentFileActions();
    void updateNofRecentFileActions(int maxRecentFiles);
    void onRecentFileAction();
    void clearRecentFileMenu();
};

#endif // RECENTFILEMENU_H
