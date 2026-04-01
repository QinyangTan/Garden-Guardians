#pragma once

#include <vector>

#include "Types.hpp"
#include "raylib.h"

class ProceduralSprites {
  public:
    void Initialize();
    void Shutdown();

    [[nodiscard]] bool Ready() const;
    void DrawPlant(PlantType type, Vector2 center, float timeSeconds) const;
    void DrawZombie(ZombieType type, Rectangle bodyRect, float timeSeconds) const;

  private:
    static constexpr int kFrames = 4;

    bool ready_ = false;
    std::vector<Texture2D> plantAtlases_;
    std::vector<Texture2D> zombieAtlases_;

    Texture2D BuildPlantAtlas(PlantType type) const;
    Texture2D BuildZombieAtlas(ZombieType type) const;
};
