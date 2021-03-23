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
#ifndef SIMTYPE_H
#define SIMTYPE_H

#include <QString>

/*!
 * Simulation variable types and enumerations.
 */
namespace SimTypes {

    enum class LightState {
        None = 0x0000,
        Navigation = 0x0001,
        Beacon = 0x0002,
        Landing = 0x0004,
        Taxi = 0x0008,
        Strobe = 0x0010,
        Panel = 0x0020,
        Recognition = 0x0040,
        Wing = 0x0080,
        Logo = 0x0100,
        Cabin = 0x0200
    };
    Q_DECLARE_FLAGS(LightStates, SimTypes::LightState)

    enum class SurfaceType {
        Unknown,
        Concrete,
        Grass,
        Water,
        BumpyGrass,
        Asphalt,
        ShortGrass,
        LongGrass,
        HardTurf,
        Snow,
        Ice,
        Urban,
        Forest,
        Dirt,
        Coral,
        Gravel,
        OilTreated,
        SteelMats,
        Bituminus,
        Brick,
        Macadam,
        Planks,
        Sand,
        Shale,
        Tarmac,
        WrightFlyerTrack
    };

    enum class EngineType {
        Unknown,
        Piston,
        Jet,
        None,
        HeloBellTurbine,
        Unsupported,
        Turboprop,
    };

    enum class PrecipitationState {
        Unknown,
        None,
        Rain,
        Snow
    };

    inline QString surfaceTypeToString(SurfaceType surfaceType) noexcept {
        QString unknown = QT_TRANSLATE_NOOP("SimTypes", "Unknown");
        switch (surfaceType) {
        case SurfaceType::Unknown:
            return unknown;
            break;
        case SurfaceType::Concrete:
            return QT_TRANSLATE_NOOP("SimTypes", "Concrete");
            break;
        case SurfaceType::Grass:
            return QT_TRANSLATE_NOOP("SimTypes", "Grass");
            break;
        case SurfaceType::Water:
            return QT_TRANSLATE_NOOP("SimTypes", "Water");
            break;
        case SurfaceType::BumpyGrass:
            return QT_TRANSLATE_NOOP("SimTypes", "Bumpy grass");
            break;
        case SurfaceType::Asphalt:
            return QT_TRANSLATE_NOOP("SimTypes", "Asphalt");
            break;
        case SurfaceType::ShortGrass:
            return QT_TRANSLATE_NOOP("SimTypes", "Short grass");
            break;
        case SurfaceType::LongGrass:
            return QT_TRANSLATE_NOOP("SimTypes", "Long grass");
            break;
        case SurfaceType::HardTurf:
            return QT_TRANSLATE_NOOP("SimTypes", "Hard turf");
            break;
        case SurfaceType::Snow:
            return QT_TRANSLATE_NOOP("SimTypes", "Snow");
            break;
        case SurfaceType::Ice:
            return QT_TRANSLATE_NOOP("SimTypes", "Ice");
            break;
        case SurfaceType::Urban:
            return QT_TRANSLATE_NOOP("SimTypes", "Urban");
            break;
        case SurfaceType::Forest:
            return QT_TRANSLATE_NOOP("SimTypes", "Forest");
            break;
        case SurfaceType::Dirt:
            return QT_TRANSLATE_NOOP("SimTypes", "Dirt");
            break;
        case SurfaceType::Coral:
            return QT_TRANSLATE_NOOP("SimTypes", "Coral");
            break;
        case SurfaceType::Gravel:
            return QT_TRANSLATE_NOOP("SimTypes", "Gravel");
            break;
        case SurfaceType::OilTreated:
            return QT_TRANSLATE_NOOP("SimTypes", "Oil treated");
            break;
        case SurfaceType::SteelMats:
            return QT_TRANSLATE_NOOP("SimTypes", "Steel mats");
            break;
        case SurfaceType::Bituminus:
            return QT_TRANSLATE_NOOP("SimTypes", "Bituminus");
            break;
        case SurfaceType::Brick:
            return QT_TRANSLATE_NOOP("SimTypes", "Brick");
            break;
        case SurfaceType::Macadam:
            return QT_TRANSLATE_NOOP("SimTypes", "Macadam");
            break;
        case SurfaceType::Planks:
            return QT_TRANSLATE_NOOP("SimTypes", "Planks");
            break;
        case SurfaceType::Sand:
            return QT_TRANSLATE_NOOP("SimTypes", "Sand");
            break;
        case SurfaceType::Shale:
            return QT_TRANSLATE_NOOP("SimTypes", "Shale");
            break;
        case SurfaceType::Tarmac:
            return QT_TRANSLATE_NOOP("SimTypes", "Tarmac");
            break;
        case SurfaceType::WrightFlyerTrack:
            return QT_TRANSLATE_NOOP("SimTypes", "Wright flyer track");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString engineTypeToString(EngineType engineType) noexcept {
        QString unknown = QT_TRANSLATE_NOOP("SimTypes", "Unknown");
        switch (engineType) {
        case EngineType::Unknown:
            return unknown;
            break;
        case EngineType::Piston:
            return QT_TRANSLATE_NOOP("SimTypes", "Piston");
            break;
        case EngineType::Jet:
            return QT_TRANSLATE_NOOP("SimTypes", "Jet");
            break;
        case EngineType::None:
            return QT_TRANSLATE_NOOP("SimTypes", "None");
            break;
        case EngineType::HeloBellTurbine:
            return QT_TRANSLATE_NOOP("SimTypes", "Helo (Bell) turbine");
            break;
        case EngineType::Unsupported:
            return QT_TRANSLATE_NOOP("SimTypes", "Unsupported");
            break;
        case EngineType::Turboprop:
            return QT_TRANSLATE_NOOP("SimTypes", "Turboprop");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString precipitationStateToString(PrecipitationState precipitationState) noexcept {
        QString unknown = QT_TRANSLATE_NOOP("SimTypes", "Unknown");
        switch (precipitationState) {
        case PrecipitationState::Unknown:
            return unknown;
            break;
        case PrecipitationState::None:
            return QT_TRANSLATE_NOOP("SimTypes", "None");
            break;
        case PrecipitationState::Rain:
            return QT_TRANSLATE_NOOP("SimTypes", "Rain");
            break;
        case PrecipitationState::Snow:
            return QT_TRANSLATE_NOOP("SimTypes", "Snow");
            break;
        default:
            return unknown;
            break;
        }
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS(SimTypes::LightStates)

#endif // SIMTYPE_H
