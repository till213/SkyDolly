/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QString>
#include <QStringView>
#include <QRegularExpression>
#include <QCoreApplication>

#include <Kernel/Enum.h>
#include <Kernel/Version.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Kernel/File.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include "Export.h"

// PUBLIC

QString Export::suggestFlightFilePath(const Flight &flight, QStringView extension) noexcept
{
    // https://www.codeproject.com/tips/758861/removing-characters-which-are-not-allowed-in-windo
    static const QRegularExpression illegalInFileName = QRegularExpression(R"([\\/:*?""<>|])");
    QString suggestedFileName;
    const auto &settings = Settings::getInstance();

    const QString &title = flight.getTitle();
    if (title.isNull()) {
        if (flight.count() > 0) {
            const auto &aircraft = flight.getUserAircraft();
            suggestedFileName = aircraft.getAircraftInfo().aircraftType.type;
        } else {
            suggestedFileName = Version::getApplicationName();
        }
    } else {
        suggestedFileName = title;
    }

    suggestedFileName = suggestedFileName.replace(illegalInFileName, "_");
    return settings.getExportPath() + "/" + File::ensureExtension(suggestedFileName, extension);
}

QString Export::suggestLocationFilePath(QStringView extension) noexcept
{
    QString suggestedFileName {QCoreApplication::translate("Export", "Locations")};

    const auto &settings = Settings::getInstance();
    return settings.getExportPath() + "/" + File::ensureExtension(suggestedFileName, extension);
}

std::vector<PositionData> Export::resamplePositionDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<PositionData> interpolatedData;
    // Position data
    Position &position = aircraft.getPosition();
    if (position.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = position.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = position.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(position.count());
            std::copy(position.begin(), position.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}

std::vector<EngineData> Export::resampleEngineDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<EngineData> interpolatedData;
    // Position data
    auto &engine = aircraft.getEngine();
    if (engine.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = engine.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = engine.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(engine.count());
            std::copy(engine.begin(), engine.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}

std::vector<PrimaryFlightControlData> Export::resamplePrimaryFlightControlDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<PrimaryFlightControlData> interpolatedData;
    // Position data
    auto &primaryFlightControl = aircraft.getPrimaryFlightControl();
    if (primaryFlightControl.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = primaryFlightControl.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = primaryFlightControl.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(primaryFlightControl.count());
            std::copy(primaryFlightControl.begin(), primaryFlightControl.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}

std::vector<SecondaryFlightControlData> Export::resampleSecondaryFlightControlDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<SecondaryFlightControlData> interpolatedData;
    // Position data
    auto &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    if (secondaryFlightControl.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = secondaryFlightControl.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = secondaryFlightControl.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(secondaryFlightControl.count());
            std::copy(secondaryFlightControl.begin(), secondaryFlightControl.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}

std::vector<AircraftHandleData> Export::resampleAircraftHandleDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<AircraftHandleData> interpolatedData;
    // Position data
    auto &aircraftHandle = aircraft.getAircraftHandle();
    if (aircraftHandle.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = aircraftHandle.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = aircraftHandle.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(aircraftHandle.count());
            std::copy(aircraftHandle.begin(), aircraftHandle.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}

std::vector<LightData> Export::resampleLightDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<LightData> interpolatedData;
    // Position data
    auto &light = aircraft.getLight();
    if (light.count() > 0) {
        if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
            const auto duration = light.getLast().timestamp;
            const auto deltaTime = Enum::underly(resamplingPeriod);
            std::int64_t timestamp {0};
            while (timestamp <= duration) {
                const auto data = light.interpolate(timestamp, TimeVariableData::Access::NoTimeOffset);
                if (!data.isNull()) {
                    interpolatedData.push_back(data);
                }
                timestamp += deltaTime;
            }
        } else {
            // Original data requested
            interpolatedData.reserve(light.count());
            std::copy(light.begin(), light.end(), std::back_inserter(interpolatedData));
        }
    }
    return interpolatedData;
}
