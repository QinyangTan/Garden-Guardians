#include "WaveManager.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

WaveManager::WaveManager(int totalWaves, float intermission, int laneCount)
    : totalWaves_(std::max(1, totalWaves)), intermission_(intermission), laneCount_(std::max(1, laneCount)) {}

void WaveManager::Reset() {
    currentWave_ = 1;
    quota_ = std::max(4, static_cast<int>(std::round(7.0f * quotaScale_)));
    spawned_ = 0;
    spawnTimer_ = 2.0f;
    countdown_ = 3.0f;
    intermissionPhase_ = true;
}

void WaveManager::ConfigureDifficulty(float spawnIntervalScale, float quotaScale, float eliteScale) {
    spawnIntervalScale_ = std::max(0.65f, spawnIntervalScale);
    quotaScale_ = std::max(0.6f, quotaScale);
    eliteScale_ = std::max(0.7f, eliteScale);
}

void WaveManager::Update(float dt, int livingZombies, const std::function<void(ZombieType, int)>& onSpawn) {
    if (currentWave_ > totalWaves_) {
        return;
    }

    if (intermissionPhase_) {
        countdown_ -= dt;
        if (countdown_ <= 0.0f) {
            intermissionPhase_ = false;
            countdown_ = 0.0f;
        }
        return;
    }

    if (spawned_ < quota_) {
        spawnTimer_ -= dt;
        if (spawnTimer_ <= 0.0f) {
            onSpawn(RollZombieType(), RollLane());
            spawned_++;
            const float minimumInterval = 0.78f;
            const float baseInterval = (2.7f - static_cast<float>(currentWave_ - 1) * 0.14f) * spawnIntervalScale_;
            spawnTimer_ = std::max(minimumInterval, baseInterval);
        }
    }

    if (spawned_ >= quota_ && livingZombies == 0) {
        currentWave_++;
        if (currentWave_ <= totalWaves_) {
            quota_ = std::max(4, static_cast<int>(std::round((7.0f + static_cast<float>(currentWave_ - 1) * 5.0f) * quotaScale_)));
            spawned_ = 0;
            intermissionPhase_ = true;
            countdown_ = intermission_;
            spawnTimer_ = 2.0f;
        }
    }
}

bool WaveManager::IsFinalWaveCleared(int livingZombies) const {
    return currentWave_ > totalWaves_ && livingZombies == 0;
}

int WaveManager::CurrentWave() const {
    return std::min(currentWave_, totalWaves_);
}

int WaveManager::TotalWaves() const {
    return totalWaves_;
}

int WaveManager::SpawnedThisWave() const {
    return spawned_;
}

int WaveManager::WaveQuota() const {
    return quota_;
}

float WaveManager::Countdown() const {
    return countdown_;
}

bool WaveManager::InIntermission() const {
    return intermissionPhase_;
}

ZombieType WaveManager::RollZombieType() const {
    std::vector<std::pair<ZombieType, int>> weighted;
    weighted.push_back({ZombieType::Basic, 36});
    weighted.push_back({ZombieType::Runner, 14});
    if (currentWave_ >= 2) {
        weighted.push_back({ZombieType::Conehead, 12});
    }
    if (currentWave_ >= 3) {
        weighted.push_back({ZombieType::Tank, 10});
        weighted.push_back({ZombieType::Imp, 12});
    }
    if (currentWave_ >= 4) {
        weighted.push_back({ZombieType::Bucket, 9});
        weighted.push_back({ZombieType::PoleJump, 10});
    }
    if (currentWave_ >= 5) {
        weighted.push_back({ZombieType::Spitter, 9});
    }
    if (currentWave_ >= 6) {
        weighted.push_back({ZombieType::Summoner, 8});
    }

    const float eliteBoost = std::max(0.7f, eliteScale_);
    int total = 0;
    for (auto& [type, w] : weighted) {
        if (type != ZombieType::Basic) {
            w = static_cast<int>(std::round(static_cast<float>(w) * eliteBoost));
        }
        total += w;
    }
    std::uniform_int_distribution<int> dist(1, std::max(1, total));
    int pick = dist(rng_);
    for (const auto& [type, w] : weighted) {
        pick -= w;
        if (pick <= 0) {
            return type;
        }
    }
    return ZombieType::Basic;
}

int WaveManager::RollLane() {
    std::uniform_int_distribution<int> laneDist(0, laneCount_ - 1);
    return laneDist(rng_);
}
