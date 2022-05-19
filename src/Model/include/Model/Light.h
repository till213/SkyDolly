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
#ifndef LIGHT_H
#define LIGHT_H

#include "LightData.h"
#include "AircraftInfo.h"
#include "AbstractComponent.h"
#include "ModelLib.h"

class MODEL_API Light : public AbstractComponent<LightData>
{
public:
    explicit Light(const AircraftInfo &aircraftInfo) noexcept;
    Light(Light &aircraftHandle) = default;
    Light(Light &&aircraftHandle) = default;
    ~Light() noexcept override;
    Light &operator=(const Light &rhs) = default;
    Light &operator=(Light &&rhs) = default;

    const LightData &interpolate(std::int64_t timestamp, TimeVariableData::Access access) noexcept override;

private:
    LightData m_currentLightData;
};

#endif // LIGHT_H
