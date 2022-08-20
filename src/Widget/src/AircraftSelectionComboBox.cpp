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
#include <memory>
#include <vector>
#include <iterator>

#include <QWidget>
#include <QComboBox>
#include <QCompleter>

#include <Model/AircraftType.h>
#include <Persistence/Service/AircraftTypeService.h>
#include "AircraftSelectionComboBox.h"

struct AircraftSelectionComboBoxPrivate
{
    AircraftSelectionComboBoxPrivate() noexcept
        : aircraftTypeService(std::make_unique<AircraftTypeService>())
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService;
};

// PUBLIC

AircraftSelectionComboBox::AircraftSelectionComboBox(QWidget *parent) noexcept
    : QComboBox(parent),
      d(std::make_unique<AircraftSelectionComboBoxPrivate>())
{
    initialise();
}

AircraftSelectionComboBox::~AircraftSelectionComboBox() noexcept
{}

// PRIVATE

void AircraftSelectionComboBox::initialise() noexcept
{
    std::vector<AircraftType> aircraftTypes;
    const bool ok = d->aircraftTypeService->getAll(std::back_inserter(aircraftTypes));
    if (ok) {
        for (AircraftType &aircraftType : aircraftTypes) {
            this->addItem(aircraftType.type);
        }
    }

    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    QCompleter *autoCompleter = completer();
    autoCompleter->setCompletionMode(QCompleter::PopupCompletion);
    autoCompleter->setFilterMode(Qt::MatchContains);
}
