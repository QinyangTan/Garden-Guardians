#pragma once

#include <functional>
#include <random>

#include "Types.hpp"

class WaveManager {
  public:
    WaveManager(int totalWaves, float intermission, int laneCount);

    void Reset();
    void ConfigureDifficulty(float spawnIntervalScale, float quotaScale, float eliteScale);
    void Update(float dt, int livingZombies, const std::function<void(ZombieType, int)>& onSpawn);

    [[nodiscard]] bool IsFinalWaveCleared(int livingZombies) const;
    [[nodiscard]] int CurrentWave() const;
    [[nodiscard]] int TotalWaves() const;
    [[nodiscard]] int SpawnedThisWave() const;
    [[nodiscard]] int WaveQuota() const;
    [[nodiscard]] float Countdown() const;
    [[nodiscard]] bool InIntermission() const;

  private:
    ZombieType RollZombieType() const;
    int RollLane();

    int totalWaves_ = 6;
    float intermission_ = 5.0f;
    int currentWave_ = 1;
    int quota_ = 8;
    int spawned_ = 0;
    float spawnTimer_ = 2.0f;
    float countdown_ = 3.0f;
    bool intermissionPhase_ = true;
    int laneCount_ = 5;
    float spawnIntervalScale_ = 1.0f;
    float quotaScale_ = 1.0f;
    float eliteScale_ = 1.0f;

    mutable std::mt19937 rng_ {std::random_device {}()};
};
