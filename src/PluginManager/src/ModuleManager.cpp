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
#include <QIcon>
#include <QActionGroup>
#include <QKeySequence>
#include <QLayout>
#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QJsonArray>
#include <QUuid>
#ifdef DEBUG
#include <QDebug>
#endif

#include <tsl/ordered_map.h>

#include <Kernel/Enum.h>
#include <Kernel/QUuidHasher.h>
#include <Kernel/Sort.h>
#include "ModuleIntf.h"
#include "DefaultModuleImpl.h"
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
    constexpr char PluginAfter[] = "after";
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
        if (recordIcon.isNull()) {
            recordIcon.addFile(":/img/icons/record-normal.png", QSize(), QIcon::Normal, QIcon::Off);
            recordIcon.addFile(":/img/icons/record-normal-on.png", QSize(), QIcon::Normal, QIcon::On);
            recordIcon.addFile(":/img/icons/record-active.png", QSize(), QIcon::Active);

            recordAddIcon.addFile(":/img/icons/record-add-normal.png", QSize(), QIcon::Normal, QIcon::Off);
            recordAddIcon.addFile(":/img/icons/record-add-normal-on.png", QSize(), QIcon::Normal, QIcon::On);
            recordAddIcon.addFile(":/img/icons/record-add-active.png", QSize(), QIcon::Active);
        }
    }

    std::unique_ptr<DefaultModuleImpl> defaultModuleImpl {nullptr};
    std::unique_ptr<QPluginLoader> pluginLoader {std::make_unique<QPluginLoader>()};
    QDir pluginsDirectoryPath;
    QLayout &layout;
    QActionGroup *moduleActionGroup;
    ModuleIntf *activeModule {nullptr};
    QUuid activeModuleUuid;
    // Key: uuid - value: plugin path
    tsl::ordered_map<QUuid, QString, QUuidHasher> moduleRegistry;
    ModuleManager::ActionRegistry actionRegistry;
    static const std::vector<QKeySequence> actionShortcuts;
    static QIcon recordIcon;
    static QIcon recordAddIcon;
};

const std::vector<QKeySequence> ModuleManagerPrivate::actionShortcuts {
    Qt::Key_F1, Qt::Key_F2, Qt::Key_F3, Qt::Key_F4, Qt::Key_F5, Qt::Key_F6, Qt::Key_F7, Qt::Key_F8, Qt::Key_F9,
    Qt::Key_F10, Qt::Key_F11, Qt::Key_F12, Qt::Key_F13, Qt::Key_F14, Qt::Key_F15, Qt::Key_F16, Qt::Key_F17,
    Qt::Key_F18, Qt::Key_F19, Qt::Key_F20
};

QIcon ModuleManagerPrivate::recordIcon;
QIcon ModuleManagerPrivate::recordAddIcon;

// PUBLIC

ModuleManager::ModuleManager(QLayout &layout, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ModuleManagerPrivate>(layout))
{
    Q_INIT_RESOURCE(PluginManager);
    initModules();
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
            d->layout.addWidget(d->activeModule->getWidget());
            d->actionRegistry[uuid]->setChecked(true);
            emit activated(d->activeModule->getModuleName(), uuid);
        }
    }
}

const QIcon &ModuleManager::getRecordIcon() const noexcept
{
    ModuleIntf::RecordIconId recordIconId {ModuleIntf::RecordIconId::Normal};
    if (d->activeModule != nullptr) {
        recordIconId = d->activeModule->getRecordIconId();
    }
    switch (recordIconId) {
    case ModuleIntf::RecordIconId::Normal:
        return d->recordIcon;
        break;
    case ModuleIntf::RecordIconId::Add:
        return d->recordAddIcon;
        break;
    }
    return d->recordIcon;
}

void ModuleManager::setRecording(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setRecording(enable);
    } else {
        d->defaultModuleImpl->setRecording(enable);
    }
}

void ModuleManager::setPlaying(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setPlaying(enable);
    } else {
        d->defaultModuleImpl->setPlaying(enable);
    }
}

void ModuleManager::setPaused(bool enable) noexcept
{
    if (d->activeModule != nullptr) {
        d->activeModule->setPaused(enable);
    } else {
        d->defaultModuleImpl->setPaused(enable);
    }
}

// PRIVATE

void ModuleManager::initModules() noexcept
{
    Graph graph;
    std::unordered_map<QUuid, ModuleInfo, QUuidHasher> moduleInfos;
    d->moduleRegistry.clear();
    if (d->pluginsDirectoryPath.exists(::ModuleDirectoryName)) {
        d->pluginsDirectoryPath.cd(::ModuleDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        for (const QString &fileName : entryList) {
            initModule(fileName, moduleInfos, graph);
        }
        initModuleActions(moduleInfos, graph);
        d->pluginsDirectoryPath.cdUp();
    }
    if (d->moduleRegistry.size() == 0) {
        // No module plugins found, use the default implementation
        d->defaultModuleImpl = std::make_unique<DefaultModuleImpl>();
    }
}

void ModuleManager::frenchConnection() noexcept
{
    connect(d->moduleActionGroup, &QActionGroup::triggered,
            this, &ModuleManager::handleModuleSelected);
}

void ModuleManager::initModule(const QString fileName, std::unordered_map<QUuid, ModuleInfo, QUuidHasher> &moduleInfos, Graph &graph) noexcept
{
    const QString pluginPath = d->pluginsDirectoryPath.absoluteFilePath(fileName);
    d->pluginLoader->setFileName(pluginPath);
    const QJsonObject metaData = d->pluginLoader->metaData();
    if (!metaData.isEmpty()) {
        const QJsonObject pluginMetadata = metaData.value("MetaData").toObject();
        const QUuid uuid = pluginMetadata.value(::PluginUuidKey).toString();
        const QString name = pluginMetadata.value(::PluginNameKey).toString();
        moduleInfos[uuid] = std::make_pair(name, pluginPath);

        const QJsonArray afterArray = pluginMetadata.value(::PluginAfter).toArray();
        std::shared_ptr<Vertex> vertex ;
        const auto it = graph.find(uuid);
        // Vertex already in graph?
        if (it == graph.end()) {
            vertex = std::make_shared<Vertex>(uuid);
            graph[uuid] = vertex;
        } else {
            vertex = it->second;
        }
        for (const auto &json : afterArray) {
            const QString uuidString = json.toString();
            const QUuid uuid {uuidString};
            // This 'vertex' (module) comes after the 'afterVertex'
            std::shared_ptr<Vertex> afterVertex {nullptr};
            const auto it = graph.find(uuid);
            // Neighbouring vertex already in graph?
            if (it == graph.end()) {
                afterVertex = std::make_shared<Vertex>(uuid);
                graph[uuid] = afterVertex;
            } else {
                afterVertex = it->second;
            }
            vertex->edges.push_back(afterVertex);
        }
    }
}

void ModuleManager::initModuleActions(const std::unordered_map<QUuid, ModuleInfo, QUuidHasher> &moduleInfos, Graph &graph) noexcept
{
    std::deque<std::shared_ptr<Vertex>> sortedModules;
    // Reverse sorting, because the "after" edge (directed from A to B: A --- after ---> B) really means that
    // "first B, then A" (= reversed topological sorting)
    sortedModules = Sort::topologicalSort(graph, Sort::Sorting::Reverse);
    int count {0};
    for (const auto &sortedModule : sortedModules) {
        const QUuid uuid {sortedModule->id};
        const ModuleInfo moduleInfo = moduleInfos.at(uuid);
        QAction *action = d->moduleActionGroup->addAction(moduleInfo.first);
        if (count < d->actionShortcuts.size()) {
            action->setShortcut(d->actionShortcuts[count]);
        }
        action->setData(sortedModule->id);
        action->setCheckable(true);
        d->actionRegistry[uuid] = action;
        d->moduleRegistry[uuid] = moduleInfo.second;
        ++count;
    }
}

// PRIVATE SLOTS

void ModuleManager::handleModuleSelected(QAction *action) noexcept
{
    QUuid uuid = action->data().toUuid();
    activateModule(uuid);
}
