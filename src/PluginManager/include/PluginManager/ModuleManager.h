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
#ifndef MODULESWITCHER_H
#define MODULESWITCHER_H

#include <memory>
#include <vector>
#include <utility>
#include <optional>
#include <functional>

#include <QObject>
#include <QUuid>
#include <QString>
#include <QAction>

class QLayout;

#include <tsl/ordered_map.h>

#include <Kernel/QUuidHasher.h>
#include "PluginManagerLib.h"

class ModuleIntf;
struct ModuleManagerPrivate;

class PLUGINMANAGER_API ModuleManager : public QObject
{
    Q_OBJECT
public:
    explicit ModuleManager(QLayout &layout, QObject *parent = nullptr) noexcept;
    ~ModuleManager() noexcept override;

    using ActionRegistry = tsl::ordered_map<QUuid, QAction *, QUuidHasher>;

    const ActionRegistry &getActionRegistry() const noexcept;

    /*!
     * Returns the active module, or \c nullptr in case no module exists.
     *
     * \return the active module; may be \c nullptr (no module plugin loaded)
     */
    std::optional<std::reference_wrapper<ModuleIntf>> getActiveModule() const noexcept;
    void activateModule(QUuid uuid) noexcept;

    void setRecording(bool enable) noexcept;
    void setPlaying(bool enable) noexcept;
    void setPaused(bool enable) noexcept;

signals:
    void activated(QString title, QUuid moduleUuid);

private:
    std::unique_ptr<ModuleManagerPrivate> d;

    void enumerateModules() noexcept;
    void frenchConnection() noexcept;

private slots:
    void handleModuleSelected(QAction *action) noexcept;
};

#endif // MODULESWITCHER_H
