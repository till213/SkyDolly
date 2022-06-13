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

#include <QAction>
#include <QActionGroup>
#include <QStackedWidget>
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
    ModuleManagerPrivate(QStackedWidget &theModuleStackWidget) noexcept
        : moduleStackWidget(theModuleStackWidget)
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
    QStackedWidget &moduleStackWidget;
    ModuleIntf *activeModule;
    QUuid activeModuleUuid;
    // Key: uuid - value: plugin path
    QMap<QUuid, QString> moduleRegistry;
    ModuleManager::Actions actionRegistry;
    QActionGroup *moduleActionGroup {nullptr};
};

// PUBLIC

ModuleManager::ModuleManager(QStackedWidget &moduleStackWidget, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ModuleManagerPrivate>(moduleStackWidget))
{
    enumerateModules();
    if (d->moduleRegistry.count() > 0) {
        activateModule(d->moduleRegistry.firstKey());
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

const ModuleManager::Actions &ModuleManager::getModules() const noexcept
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
            d->moduleStackWidget.addWidget(&d->activeModule->getWidget());

            // @todo IMPLEMENT ME Exchange/update widgets
          //  d->moduleStackWidget.setCurrentWidget(&module->getWidget());
          //  module->setActive(true);
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
    d->moduleActionGroup = new QActionGroup(this);

    d->moduleRegistry.clear();
    if (d->pluginsDirectoryPath.exists(::ModuleDirectoryName)) {
        d->pluginsDirectoryPath.cd(::ModuleDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectoryPath.absoluteFilePath(fileName);
            d->pluginLoader->setFileName(pluginPath);
            const QJsonObject metaData = d->pluginLoader->metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata = metaData.value("MetaData").toObject();
                const QUuid uuid = pluginMetadata.value(::PluginUuidKey).toString();
                const QString pluginName = pluginMetadata.value(::PluginNameKey).toString();
                QAction *action = d->moduleActionGroup->addAction(pluginName);
                action->setData(uuid);
                d->actionRegistry[uuid] = action;
                d->moduleRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectoryPath.cdUp();
    }


    // @todo IMPLEMENT ME dynamically load as plugins
//    LogbookWidget *logbookWidget = new LogbookWidget(d->databaseService, d->flightService, &d->moduleStackWidget);
//    d->moduleMap[logbookWidget->getModuleId()] = logbookWidget;
//    d->moduleStackWidget.addWidget(logbookWidget);
//    QAction &logbookAction = logbookWidget->getAction();
//    logbookAction.setData(Enum::toUnderlyingType(logbookWidget->getModuleId()));
//    logbookAction.setShortcut(tr("F1"));
//    logbookAction.setToolTip("Record single aircraft, load and manage existing flights.");
//    d->moduleActionGroup->addAction(&logbookAction);

//    FormationWidget *formationWidget = new FormationWidget(d->flightService, &d->moduleStackWidget);
//    d->moduleMap[formationWidget->getModuleId()] = formationWidget;
//    d->moduleStackWidget.addWidget(formationWidget);
//    QAction &formationAction = formationWidget->getAction();
//    formationAction.setData(Enum::toUnderlyingType(formationWidget->getModuleId()));
//    formationAction.setShortcut(tr("F2"));
//    formationAction.setToolTip("Record and manage formation flights.");
//    d->moduleActionGroup->addAction(&formationAction);
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
