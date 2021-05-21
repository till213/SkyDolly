/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <unordered_map>

#include <QStackedWidget>

#include "../../Persistence/src/Service/DatabaseService.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "Logbook/LogbookWidget.h"
#include "Formation/FormationWidget.h"
#include "Module.h"
#include "ModuleIntf.h"
#include "ModuleSwitcher.h"

namespace
{
    constexpr Module::Module DefaultModule = Module::Module::Logbook;
}

class ModuleSwitcherPrivate
{
public:
    ModuleSwitcherPrivate(QStackedWidget &theModuleStackWidget, DatabaseService &theDatabaseService, FlightService &theFlightService) noexcept
        : moduleStackWidget(theModuleStackWidget),
          databaseService(theDatabaseService),
          flightService(theFlightService),
          activeModuleId(Module::Module::None)
    {}

    ~ModuleSwitcherPrivate() noexcept
    {}

    QStackedWidget &moduleStackWidget;
    DatabaseService &databaseService;
    FlightService &flightService;
    Module::Module activeModuleId;
    std::unordered_map<Module::Module, ModuleIntf *> moduleMap;
};

// PUBLIC

ModuleSwitcher::ModuleSwitcher(QStackedWidget &moduleStackWidget, DatabaseService &theDatabaseService, FlightService &theFlightService, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ModuleSwitcherPrivate>(moduleStackWidget, theDatabaseService, theFlightService))
{
    initModules();
    activateModule(DefaultModule);
}

ModuleSwitcher::~ModuleSwitcher() noexcept
{
#ifdef DEBUG
    qDebug("ModuleSwitcher::~ModuleSwitcher: DELETED.");
#endif
}

ModuleIntf &ModuleSwitcher::getActiveModule() const
{
    return *d->moduleMap[d->activeModuleId];
}

void ModuleSwitcher::activateModule(Module::Module moduleId) noexcept
{
    if (d->activeModuleId != moduleId) {
        d->activeModuleId = moduleId;
        ModuleIntf *module = d->moduleMap[d->activeModuleId];
        d->moduleStackWidget.setCurrentWidget(&module->getWidget());
        emit activated(module->getTitle(), moduleId);
    }
}

// PRIVATE

void ModuleSwitcher::initModules() noexcept
{
    LogbookWidget *logbookWidget = new LogbookWidget(d->databaseService, d->flightService, &d->moduleStackWidget);
    d->moduleMap[logbookWidget->getModuleId()] = logbookWidget;
    d->moduleStackWidget.addWidget(logbookWidget);

    FormationWidget *formationWidget = new FormationWidget(&d->moduleStackWidget);
    d->moduleMap[formationWidget->getModuleId()] = formationWidget;
    d->moduleStackWidget.addWidget(formationWidget);
}
