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

#include <tsl/ordered_map.h>

#include <QAction>
#include <QActionGroup>
#include <QStackedWidget>

#include <Kernel/Enum.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/FlightService.h>
#include "Module.h"
#include "ModuleIntf.h"
#include "ModuleManager.h"

namespace
{
    constexpr Module::Module DefaultModule = Module::Module::Logbook;
}

class ModuleManagerPrivate
{
public:
    ModuleManagerPrivate(QStackedWidget &theModuleStackWidget, DatabaseService &theDatabaseService) noexcept
        : moduleStackWidget(theModuleStackWidget),
          databaseService(theDatabaseService),
          activeModuleId(Module::Module::None),
          moduleActionGroup(nullptr)
    {}

    ~ModuleManagerPrivate() noexcept
    {}

    QStackedWidget &moduleStackWidget;
    DatabaseService &databaseService;
    Module::Module activeModuleId;
    tsl::ordered_map<Module::Module, ModuleIntf *> moduleMap;
    QActionGroup *moduleActionGroup;
};

// PUBLIC

ModuleManager::ModuleManager(QStackedWidget &moduleStackWidget, DatabaseService &databaseService, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ModuleManagerPrivate>(moduleStackWidget, databaseService))
{
    initModules();
    activateModule(DefaultModule);
    frenchConnection();
}

ModuleManager::~ModuleManager() noexcept
{
#ifdef DEBUG
    qDebug("ModuleManager::~ModuleManager: DELETED.");
#endif
}

std::vector<ModuleIntf *> ModuleManager::getModules() const
{
    std::vector<ModuleIntf *> modules;
    for (const auto &module : d->moduleMap) {
        modules.push_back(module.second);
    }
    return modules;
}

ModuleIntf &ModuleManager::getModule(Module::Module moduleId) const noexcept
{
    return *d->moduleMap[moduleId];
}

ModuleIntf &ModuleManager::getActiveModule() const
{
    return *d->moduleMap[d->activeModuleId];
}

void ModuleManager::activateModule(Module::Module moduleId) noexcept
{
    if (d->activeModuleId != moduleId) {
        if (d->activeModuleId != Module::Module::None) {
            ModuleIntf *previousModule = d->moduleMap[d->activeModuleId];
            previousModule->setActive(false);
        }
        d->activeModuleId = moduleId;
        ModuleIntf *module = d->moduleMap[d->activeModuleId];
        d->moduleStackWidget.setCurrentWidget(&module->getWidget());
        module->setActive(true);

        emit activated(module->getModuleName(), moduleId);
    }
}

// PRIVATE

void ModuleManager::initModules() noexcept
{
    d->moduleActionGroup = new QActionGroup(this);

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
    Module::Module moduleId = static_cast<Module::Module>(action->data().toInt());
    activateModule(moduleId);
}
