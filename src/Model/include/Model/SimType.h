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
#ifndef SIMTYPE_H
#define SIMTYPE_H

#include <QCoreApplication>
#include <QString>

/*!
 * Simulation variable types and enumerations.
 */
namespace SimType {

    /*!
     * The state of the aircraft lights.
     *
     * Implemenation note: those values act as actual flag values which get persisted in the database.
     */
    enum struct LightState {
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
    Q_DECLARE_FLAGS(LightStates, LightState)

    /*!
     * The surface type describes the surface on (over) which the flight has started.
     *
     * Implemenation note: those values act as actual IDs which get persisted in the database.
     */
    enum struct SurfaceType {
        Unknown = 0,
        Concrete = 1,
        Grass = 2,
        Water = 3,
        BumpyGrass = 4,
        Asphalt = 5,
        ShortGrass = 6,
        LongGrass = 7,
        HardTurf = 8,
        Snow = 9,
        Ice = 10,
        Urban = 11,
        Forest = 12,
        Dirt = 13,
        Coral = 14,
        Gravel = 15,
        OilTreated = 16,
        SteelMats = 17,
        Bituminus = 18,
        Brick = 19,
        Macadam = 20,
        Planks = 21,
        Sand = 22,
        Shale = 23,
        Tarmac = 24,
        WrightFlyerTrack  = 25
    };

    /*!
     * The engine type of the aircraft.
     *
     * Implemenation note: those values act as actual IDs which get persisted in the database.
     */
    enum struct EngineType {
        Unknown = 0,
        Piston = 1,
        Jet = 2,
        None = 3,
        HeloBellTurbine = 4,
        Unsupported = 5,
        Turboprop = 6,
        // Only used for selection (never assigned/persisted)
        All
    };

    /*!
     * The precipitation state at the beginning of the flight.
     *
     * Implemenation note: those values act as actual IDs which get persisted in the database.
     */
    enum struct PrecipitationState {
        Unknown = 0,
        None = 1,
        Rain = 2,
        Snow = 3
    };

    inline QString surfaceTypeToString(SurfaceType surfaceType) noexcept {
        QString unknown = QCoreApplication::translate("SimType", "Unknown");
        switch (surfaceType) {
        case SurfaceType::Unknown:
            return unknown;
            break;
        case SurfaceType::Concrete:
            return QCoreApplication::translate("SimType", "Concrete");
            break;
        case SurfaceType::Grass:
            return QCoreApplication::translate("SimType", "Grass");
            break;
        case SurfaceType::Water:
            return QCoreApplication::translate("SimType", "Water");
            break;
        case SurfaceType::BumpyGrass:
            return QCoreApplication::translate("SimType", "Bumpy grass");
            break;
        case SurfaceType::Asphalt:
            return QCoreApplication::translate("SimType", "Asphalt");
            break;
        case SurfaceType::ShortGrass:
            return QCoreApplication::translate("SimType", "Short grass");
            break;
        case SurfaceType::LongGrass:
            return QCoreApplication::translate("SimType", "Long grass");
            break;
        case SurfaceType::HardTurf:
            return QCoreApplication::translate("SimType", "Hard turf");
            break;
        case SurfaceType::Snow:
            return QCoreApplication::translate("SimType", "Snow");
            break;
        case SurfaceType::Ice:
            return QCoreApplication::translate("SimType", "Ice");
            break;
        case SurfaceType::Urban:
            return QCoreApplication::translate("SimType", "Urban");
            break;
        case SurfaceType::Forest:
            return QCoreApplication::translate("SimType", "Forest");
            break;
        case SurfaceType::Dirt:
            return QCoreApplication::translate("SimType", "Dirt");
            break;
        case SurfaceType::Coral:
            return QCoreApplication::translate("SimType", "Coral");
            break;
        case SurfaceType::Gravel:
            return QCoreApplication::translate("SimType", "Gravel");
            break;
        case SurfaceType::OilTreated:
            return QCoreApplication::translate("SimType", "Oil treated");
            break;
        case SurfaceType::SteelMats:
            return QCoreApplication::translate("SimType", "Steel mats");
            break;
        case SurfaceType::Bituminus:
            return QCoreApplication::translate("SimType", "Bituminus");
            break;
        case SurfaceType::Brick:
            return QCoreApplication::translate("SimType", "Brick");
            break;
        case SurfaceType::Macadam:
            return QCoreApplication::translate("SimType", "Macadam");
            break;
        case SurfaceType::Planks:
            return QCoreApplication::translate("SimType", "Planks");
            break;
        case SurfaceType::Sand:
            return QCoreApplication::translate("SimType", "Sand");
            break;
        case SurfaceType::Shale:
            return QCoreApplication::translate("SimType", "Shale");
            break;
        case SurfaceType::Tarmac:
            return QCoreApplication::translate("SimType", "Tarmac");
            break;
        case SurfaceType::WrightFlyerTrack:
            return QCoreApplication::translate("SimType", "Wright flyer track");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString engineTypeToString(EngineType engineType) noexcept {
        QString unknown = QCoreApplication::translate("SimType", "Unknown");
        switch (engineType) {
        case EngineType::Unknown:
            return unknown;
            break;
        case EngineType::Piston:
            return QCoreApplication::translate("SimType", "Piston");
            break;
        case EngineType::Jet:
            return QCoreApplication::translate("SimType", "Jet");
            break;
        case EngineType::None:
            return QCoreApplication::translate("SimType", "No engine");
            break;
        case EngineType::HeloBellTurbine:
            return QCoreApplication::translate("SimType", "Helo (Bell) turbine");
            break;
        case EngineType::Unsupported:
            return QCoreApplication::translate("SimType", "Unsupported");
            break;
        case EngineType::Turboprop:
            return QCoreApplication::translate("SimType", "Turboprop");
            break;
        case EngineType::All:
            return QCoreApplication::translate("SimType", "All");
            break;
        default:
            return unknown;
            break;
        }
    }

    inline QString precipitationStateToString(PrecipitationState precipitationState) noexcept {
        QString unknown = QCoreApplication::translate("SimType", "Unknown");
        switch (precipitationState) {
        case PrecipitationState::Unknown:
            return unknown;
            break;
        case PrecipitationState::None:
            return QCoreApplication::translate("SimType", "None");
            break;
        case PrecipitationState::Rain:
            return QCoreApplication::translate("SimType", "Rain");
            break;
        case PrecipitationState::Snow:
            return QCoreApplication::translate("SimType", "Snow");
            break;
        default:
            return unknown;
            break;
        }
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS(SimType::LightStates)

#endif // SIMTYPE_H
