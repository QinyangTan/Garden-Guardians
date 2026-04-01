#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "raylib.h"

enum class GameState {
    Menu,
    HowToPlay,
    Playing,
    Paused,
    Win,
    Lose
};

enum class PlantType {
    Peashooter = 0,
    SunBloom = 1,
    WallRoot = 2,
    Chomper = 3,
    FrostBurst = 4,
    Repeater = 5,
    IcePea = 6,
    SpikeTrap = 7,
    BombBerry = 8,
    MagnetVine = 9,
    Count
};

enum class ZombieType {
    Basic = 0,
    Runner = 1,
    Tank = 2,
    Bucket = 3,
    Conehead = 4,
    Imp = 5,
    Spitter = 6,
    PoleJump = 7,
    Summoner = 8,
    Count
};

enum class PlacementFailReason {
    None,
    Occupied,
    NotEnoughSun,
    CoolingDown,
    InvalidTile
};

struct Projectile {
    int row = 0;
    float x = 0.0f;
    float y = 0.0f;
    float speed = 260.0f;
    float damage = 20.0f;
    bool slowing = false;
    float slowFactor = 0.55f;
    float slowDuration = 2.5f;
    bool fromZombie = false;
    bool alive = true;
};

struct SunToken {
    Vector2 position {0.0f, 0.0f};
    Vector2 velocity {0.0f, 0.0f};
    float fallSpeed = 40.0f;
    float ttl = 9.0f;
    float jumpTimer = 0.0f;
    int value = 25;
    bool fromPlant = false;
    bool alive = true;
};

struct LaneMower {
    bool available = true;
    bool active = false;
    float x = 0.0f;
};

struct FloatingText {
    Vector2 position {0.0f, 0.0f};
    float ttl = 0.0f;
    Color color {255, 255, 255, 255};
    std::string text;
};

inline constexpr std::size_t PlantTypeCount() {
    return static_cast<std::size_t>(PlantType::Count);
}

inline constexpr std::size_t ZombieTypeCount() {
    return static_cast<std::size_t>(ZombieType::Count);
}

inline constexpr std::size_t PlantIndex(PlantType type) {
    return static_cast<std::size_t>(type);
}

inline constexpr std::size_t ZombieIndex(ZombieType type) {
    return static_cast<std::size_t>(type);
}

inline std::vector<PlantType> AllPlantTypes() {
    return {
        PlantType::Peashooter, PlantType::SunBloom, PlantType::WallRoot, PlantType::Chomper, PlantType::FrostBurst,
        PlantType::Repeater, PlantType::IcePea, PlantType::SpikeTrap, PlantType::BombBerry, PlantType::MagnetVine
    };
}

inline std::vector<ZombieType> AllZombieTypes() {
    return {
        ZombieType::Basic, ZombieType::Runner, ZombieType::Tank, ZombieType::Bucket, ZombieType::Conehead,
        ZombieType::Imp, ZombieType::Spitter, ZombieType::PoleJump, ZombieType::Summoner
    };
}

inline std::string PlantName(PlantType type) {
    switch (type) {
        case PlantType::Peashooter: return "Peashooter";
        case PlantType::SunBloom: return "SunBloom";
        case PlantType::WallRoot: return "WallRoot";
        case PlantType::Chomper: return "Chomper";
        case PlantType::FrostBurst: return "FrostBurst";
        case PlantType::Repeater: return "Repeater";
        case PlantType::IcePea: return "Ice Pea";
        case PlantType::SpikeTrap: return "SpikeTrap";
        case PlantType::BombBerry: return "BombBerry";
        case PlantType::MagnetVine: return "Magnet Vine";
        default: return "Unknown";
    }
}

inline std::string ZombieName(ZombieType type) {
    switch (type) {
        case ZombieType::Basic: return "Basic";
        case ZombieType::Runner: return "Runner";
        case ZombieType::Tank: return "Tank";
        case ZombieType::Bucket: return "Bucket";
        case ZombieType::Conehead: return "Conehead";
        case ZombieType::Imp: return "Imp";
        case ZombieType::Spitter: return "Spitter";
        case ZombieType::PoleJump: return "Pole-Jump";
        case ZombieType::Summoner: return "Summoner";
        default: return "Unknown";
    }
}
