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
#ifndef SIMTYPES_H
#define SIMTYPES_H

#include <QString>

namespace SimTypes {

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

    inline QString surfaceTypeToString(SurfaceType surfaceType) {
        QString unknown = QT_TR_NOOP("Unknown");
        switch (surfaceType) {
        case SurfaceType::Unknown:
            return unknown;
            break;
        case SurfaceType::Concrete:
            return QT_TR_NOOP("Concrete");
            break;
        case SurfaceType::Grass:
            return QT_TR_NOOP("Grass");
            break;
        case SurfaceType::Water:
            return QT_TR_NOOP("Water");
            break;
        case SurfaceType::BumpyGrass:
            return QT_TR_NOOP("Bumpy grass");
            break;
        case SurfaceType::Asphalt:
            return QT_TR_NOOP("Asphalt");
            break;
        case SurfaceType::ShortGrass:
            return QT_TR_NOOP("Short grass");
            break;
        case SurfaceType::LongGrass:
            return QT_TR_NOOP("Long grass");
            break;
        case SurfaceType::HardTurf:
            return QT_TR_NOOP("Hard turf");
            break;
        case SurfaceType::Snow:
            return QT_TR_NOOP("Snow");
            break;
        case SurfaceType::Ice:
            return QT_TR_NOOP("Ice");
            break;
        case SurfaceType::Urban:
            return QT_TR_NOOP("Urban");
            break;
        case SurfaceType::Forest:
            return QT_TR_NOOP("Forest");
            break;
        case SurfaceType::Dirt:
            return QT_TR_NOOP("Dirt");
            break;
        case SurfaceType::Coral:
            return QT_TR_NOOP("Coral");
            break;
        case SurfaceType::Gravel:
            return QT_TR_NOOP("Gravel");
            break;
        case SurfaceType::OilTreated:
            return QT_TR_NOOP("Oil treated");
            break;
        case SurfaceType::SteelMats:
            return QT_TR_NOOP("Steel mats");
            break;
        case SurfaceType::Bituminus:
            return QT_TR_NOOP("Bituminus");
            break;
        case SurfaceType::Brick:
            return QT_TR_NOOP("Brick");
            break;
        case SurfaceType::Macadam:
            return QT_TR_NOOP("Macadam");
            break;
        case SurfaceType::Planks:
            return QT_TR_NOOP("Planks");
            break;
        case SurfaceType::Sand:
            return QT_TR_NOOP("Sand");
            break;
        case SurfaceType::Shale:
            return QT_TR_NOOP("Shale");
            break;
        case SurfaceType::Tarmac:
            return QT_TR_NOOP("Tarmac");
            break;
        case SurfaceType::WrightFlyerTrack:
            return QT_TR_NOOP("Wright flyer track");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString engineTypeToString(EngineType engineType) {
        QString unknown = QT_TR_NOOP("Unknown");
        switch (engineType) {
        case EngineType::Unknown:
            return unknown;
            break;
        case EngineType::Piston:
            return QT_TR_NOOP("Piston");
            break;
        case EngineType::Jet:
            return QT_TR_NOOP("Jet");
            break;
        case EngineType::None:
            return QT_TR_NOOP("None");
            break;
        case EngineType::HeloBellTurbine:
            return QT_TR_NOOP("Helo (Bell) turbine");
            break;
        case EngineType::Unsupported:
            return QT_TR_NOOP("Unsupported");
            break;
        case EngineType::Turboprop:
            return QT_TR_NOOP("Turboprop");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString precipitationStateToString(PrecipitationState precipitationState) {
        QString unknown = QT_TR_NOOP("Unknown");
        switch (precipitationState) {
        case PrecipitationState::Unknown:
            return unknown;
            break;
        case PrecipitationState::None:
            return QT_TR_NOOP("None");
            break;
        case PrecipitationState::Rain:
            return QT_TR_NOOP("Rain");
            break;
        case PrecipitationState::Snow:
            return QT_TR_NOOP("Snow");
            break;
        default:
            return unknown;
            break;
        }
    }
}

#endif // SIMTYPES_H
