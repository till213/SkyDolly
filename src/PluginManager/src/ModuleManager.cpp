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
#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>

#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QLayout>
#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#ifdef DEBUG
#include <QDebug>
#endif

#include <tsl/ordered_map.h>

#include <Kernel/Enum.h>
#include <Kernel/QUuidHasher.h>
#include "ModuleIntf.h"
#include "ModuleManager.h"

namespace
{
    constexpr char ModuleDirectoryName[] = "Module";
#if defined(Q_OS_MAC)
    constexpr char PluginDirectoryName[] = "PlugIns";
#else
    constexpr char PluginDirectoryName[] = "Plugins";
#endif
    constexpr char PluginUuidKey[] = "uuid";
    constexpr char PluginNameKey[] = "name";
}

struct ModuleManagerPrivate
{
    ModuleManagerPrivate(QLayout &theLayout) noexcept
        : layout(theLayout),
          moduleActionGroup(new QActionGroup(&layout))
    {
        pluginsDirectoryPath = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (pluginsDirectoryPath.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            pluginsDirectoryPath.cdUp();
        }
#endif
        pluginsDirectoryPath.cd(PluginDirectoryName);
    }

    std::unique_ptr<QPluginLoader> pluginLoader {std::make_unique<QPluginLoader>()};
    QDir pluginsDirectoryPath;
    QLayout &layout;
    QActionGroup * moduleActionGroup;
    ModuleIntf *activeModule {nullptr};
    QUuid activeModuleUuid;
    // Key: uuid - value: plugin path
    std::unordered_map<QUuid, QString, QUuidHasher> moduleRegistry;
    ModuleManager::ActionRegistry actionRegistry;
    std::vector<QKeySequence> actionShortcuts {Qt::Key_F1, Qt::Key_F2, Qt::Key_F3, Qt::Key_F4, Qt::Key_F5, Qt::Key_F6, Qt::Key_F7, Qt::Key_F8, Qt::Key_F9,
                                               Qt::Key_F10, Qt::Key_F11, Qt::Key_F12, Qt::Key_F13, Qt::Key_F14, Qt::Key_F15, Qt::Key_F16, Qt::Key_F17,
                                               Qt::Key_F18, Qt::Key_F19, Qt::Key_F20};
};

// PUBLIC

ModuleManager::ModuleManager(QLayout &layout, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ModuleManagerPrivate>(layout))
{
    enumerateModules();
    if (d->moduleRegistry.size() > 0) {
        activateModule(d->moduleRegistry.begin()->first);
    }
    frenchConnection();
#ifdef DEBUG
    qDebug() << "ModuleManager::ModuleManager: CREATED.";
#endif
}

ModuleManager::~ModuleManager() noexcept
{
    d->pluginLoader->unload();
#ifdef DEBUG
    qDebug() << "ModuleManager::~ModuleManager: DELETED.";
#endif
}

const ModuleManager::ActionRegistry &ModuleManager::getActionRegistry() const noexcept
{
    return d->actionRegistry;
}

std::optional<std::reference_wrapper<ModuleIntf>> ModuleManager::getActiveModule() const noexcept
{
    QObject *plugin = d->pluginLoader->instance();
    if (plugin != nullptr) {
        return std::optional<std::reference_wrapper<ModuleIntf>>{*(dynamic_cast<ModuleIntf *>(plugin))};
    } else {
        return {};
    }
}

void ModuleManager::activateModule(QUuid uuid) noexcept
{   
    if (d->activeModuleUuid != uuid) {
        if (d->pluginLoader->isLoaded()) {
            // Unload the previous module
            d->pluginLoader->unload();
            d->activeModule = nullptr;
            d->activeModuleUuid = QUuid();
        }
        QString modulePath = d->moduleRegistry[uuid];
        d->pluginLoader->setFileName(modulePath);
        const QObject *plugin = d->pluginLoader->instance();
        d->activeModule = qobject_cast<ModuleIntf *>(plugin);
        if (d->activeModule != nullptr) {
            d->activeModuleUuid = uuid;
            d->layout.addWidget(&d->activeModule->getWidget());
            d->actionRegistry[uuid]->setChecked(true);
            emit activated(d->activeModule->getModuleName(), uuid);
        }
    }
}

void ModuleManager::setRecording(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setRecording(enable);
    }
}

void ModuleManager::setPlaying(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setPlaying(enable);
    }
}

void ModuleManager::setPaused(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setPaused(enable);
    }
}

// PRIVATE

void ModuleManager::enumerateModules() noexcept
{
    d->moduleRegistry.clear();
    if (d->pluginsDirectoryPath.exists(::ModuleDirectoryName)) {
        d->pluginsDirectoryPath.cd(::ModuleDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        int count {0};
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectoryPath.absoluteFilePath(fileName);
            d->pluginLoader->setFileName(pluginPath);
            const QJsonObject metaData = d->pluginLoader->metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata = metaData.value("MetaData").toObject();
                const QUuid uuid = pluginMetadata.value(::PluginUuidKey).toString();
                const QString pluginName = pluginMetadata.value(::PluginNameKey).toString();
                QAction *action = d->moduleActionGroup->addAction(pluginName);
                if (count < d->actionShortcuts.size()) {
                    action->setShortcut(d->actionShortcuts[count]);
                }
                action->setData(uuid);
                action->setCheckable(true);
                d->actionRegistry[uuid] = action;
                d->moduleRegistry[uuid] = pluginPath;
                ++count;
            }
        }
        d->pluginsDirectoryPath.cdUp();
    }
}

void ModuleManager::frenchConnection() noexcept
{
    connect(d->moduleActionGroup, &QActionGroup::triggered,
            this, &ModuleManager::handleModuleSelected);
}

// PRIVATE SLOTS

void ModuleManager::handleModuleSelected(QAction *action) noexcept
{
    QUuid uuid = action->data().toUuid();
    activateModule(uuid);
}
