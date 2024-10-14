/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <cstdint>

#include <QCoreApplication>
#include <QString>

/*!
 * Simulation variable types and enumerations.
 */
namespace SimType {

    /*!
     * The state of the aircraft lights.
     *
     * Implementation note: these values act as actual flag values that get persisted in the database.
     */
    enum struct LightState: std::uint16_t {
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
        Cabin = 0x0200,
        Unknown = 0xffff
    };
    Q_DECLARE_FLAGS(LightStates, LightState)

    /*!
     * The surface type describes the surface on (over) which the flight has started.
     *
     * Implementation note: these values act as actual IDs that get persisted in the database.
     */
    enum struct SurfaceType: std::uint8_t {
        First = 0,
        Unknown = First,
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
        WrightFlyerTrack  = 25,
        Last = WrightFlyerTrack
    };

    /*!
     * The surface condition describes the condition of the runway on which the flight has (potentially) started.
     *
     * Implementation note: these values act as actual IDs that get persisted in the database.
     */
    enum struct SurfaceCondition: std::uint8_t {
        First = 0,
        Unknown = First,
        Normal,
        Wet ,
        Icy,
        Snow,
        Last = Snow
    };

    /*!
     * The engine type of the aircraft.
     *
     * Implementation note: these values act as actual IDs that get persisted in the database.
     */
    enum struct EngineType: std::uint8_t {
        First = 0,
        Unknown = First,
        Piston,
        Jet,
        None,
        HeloBellTurbine,
        Unsupported,
        Turboprop,
        // Only used for selection (never assigned/persisted)
        All,
        Last = All
    };

    /*!
     * The precipitation state at the beginning of the flight.
     *
     * Implementation note: these values act as actual IDs that get persisted in the database.
     */
    enum struct PrecipitationState: std::uint8_t {
        First = 0,
        Unknown = First,
        None,
        Rain,
        Snow,
        Last = Snow
    };

    inline QString surfaceTypeToString(SurfaceType surfaceType) noexcept {
        QString surfaceTypeString;
        switch (surfaceType) {
        case SurfaceType::Unknown:
            surfaceTypeString = QCoreApplication::translate("SimType", "Unknown");
            break;
        case SurfaceType::Concrete:
            surfaceTypeString = QCoreApplication::translate("SimType", "Concrete");
            break;
        case SurfaceType::Grass:
            surfaceTypeString = QCoreApplication::translate("SimType", "Grass");
            break;
        case SurfaceType::Water:
            surfaceTypeString = QCoreApplication::translate("SimType", "Water");
            break;
        case SurfaceType::BumpyGrass:
            surfaceTypeString = QCoreApplication::translate("SimType", "Bumpy grass");
            break;
        case SurfaceType::Asphalt:
            surfaceTypeString = QCoreApplication::translate("SimType", "Asphalt");
            break;
        case SurfaceType::ShortGrass:
            surfaceTypeString = QCoreApplication::translate("SimType", "Short grass");
            break;
        case SurfaceType::LongGrass:
            surfaceTypeString = QCoreApplication::translate("SimType", "Long grass");
            break;
        case SurfaceType::HardTurf:
            surfaceTypeString = QCoreApplication::translate("SimType", "Hard turf");
            break;
        case SurfaceType::Snow:
            surfaceTypeString = QCoreApplication::translate("SimType", "Snow");
            break;
        case SurfaceType::Ice:
            surfaceTypeString = QCoreApplication::translate("SimType", "Ice");
            break;
        case SurfaceType::Urban:
            surfaceTypeString = QCoreApplication::translate("SimType", "Urban");
            break;
        case SurfaceType::Forest:
            surfaceTypeString = QCoreApplication::translate("SimType", "Forest");
            break;
        case SurfaceType::Dirt:
            surfaceTypeString = QCoreApplication::translate("SimType", "Dirt");
            break;
        case SurfaceType::Coral:
            surfaceTypeString = QCoreApplication::translate("SimType", "Coral");
            break;
        case SurfaceType::Gravel:
            surfaceTypeString = QCoreApplication::translate("SimType", "Gravel");
            break;
        case SurfaceType::OilTreated:
            surfaceTypeString = QCoreApplication::translate("SimType", "Oil treated");
            break;
        case SurfaceType::SteelMats:
            surfaceTypeString = QCoreApplication::translate("SimType", "Steel mats");
            break;
        case SurfaceType::Bituminus:
            surfaceTypeString = QCoreApplication::translate("SimType", "Bituminus");
            break;
        case SurfaceType::Brick:
            surfaceTypeString = QCoreApplication::translate("SimType", "Brick");
            break;
        case SurfaceType::Macadam:
            surfaceTypeString = QCoreApplication::translate("SimType", "Macadam");
            break;
        case SurfaceType::Planks:
            surfaceTypeString = QCoreApplication::translate("SimType", "Planks");
            break;
        case SurfaceType::Sand:
            surfaceTypeString = QCoreApplication::translate("SimType", "Sand");
            break;
        case SurfaceType::Shale:
            surfaceTypeString = QCoreApplication::translate("SimType", "Shale");
            break;
        case SurfaceType::Tarmac:
            surfaceTypeString = QCoreApplication::translate("SimType", "Tarmac");
            break;
        case SurfaceType::WrightFlyerTrack:
            surfaceTypeString = QCoreApplication::translate("SimType", "Wright flyer track");
            break;
        }
        return surfaceTypeString;
    }

    inline QString surfaceConditionToString(SurfaceCondition surfaceCondition) noexcept {
        QString surfaceConditionString;
        switch (surfaceCondition) {
        case SurfaceCondition::Unknown:
            surfaceConditionString = QCoreApplication::translate("SimType", "Unknown");
            break;
        case SurfaceCondition::Normal:
            surfaceConditionString =  QCoreApplication::translate("SimType", "Normal");
            break;
        case SurfaceCondition::Wet:
            surfaceConditionString =  QCoreApplication::translate("SimType", "Wet");
            break;
        case SurfaceCondition::Icy:
            surfaceConditionString =  QCoreApplication::translate("SimType", "Icy");
            break;
        case SurfaceCondition::Snow:
            surfaceConditionString =  QCoreApplication::translate("SimType", "Snow");
            break;
        }
        return surfaceConditionString;
    }

    inline QString engineTypeToString(EngineType engineType) noexcept {
        QString engineTypeString;
        switch (engineType) {
        case EngineType::Unknown:
            engineTypeString = QCoreApplication::translate("SimType", "Unknown");
            break;
        case EngineType::Piston:
            engineTypeString = QCoreApplication::translate("SimType", "Piston");
            break;
        case EngineType::Jet:
            engineTypeString = QCoreApplication::translate("SimType", "Jet");
            break;
        case EngineType::None:
            engineTypeString = QCoreApplication::translate("SimType", "No engine");
            break;
        case EngineType::HeloBellTurbine:
            engineTypeString = QCoreApplication::translate("SimType", "Helo (Bell) turbine");
            break;
        case EngineType::Unsupported:
            engineTypeString = QCoreApplication::translate("SimType", "Unsupported");
            break;
        case EngineType::Turboprop:
            engineTypeString = QCoreApplication::translate("SimType", "Turboprop");
            break;
        case EngineType::All:
            engineTypeString = QCoreApplication::translate("SimType", "All");
            break;
        }
        return engineTypeString;
    }

    inline QString precipitationStateToString(PrecipitationState precipitationState) noexcept {
        QString precipitationStateString;
        switch (precipitationState) {
        case PrecipitationState::Unknown:
            precipitationStateString = QCoreApplication::translate("SimType", "Unknown");;
            break;
        case PrecipitationState::None:
            precipitationStateString = QCoreApplication::translate("SimType", "None");
            break;
        case PrecipitationState::Rain:
            precipitationStateString = QCoreApplication::translate("SimType", "Rain");
            break;
        case PrecipitationState::Snow:
            precipitationStateString = QCoreApplication::translate("SimType", "Snow");
            break;
        }
        return precipitationStateString;
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS(SimType::LightStates)

#endif // SIMTYPE_H
