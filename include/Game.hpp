#pragma once

#include <array>
#include <memory>
#include <random>
#include <vector>

#include "Board.hpp"
#include "GameConfig.hpp"
#include "Plant.hpp"
#include "ProceduralSprites.hpp"
#include "SaveSystem.hpp"
#include "Types.hpp"
#include "WaveManager.hpp"
#include "Zombie.hpp"
#include "raylib.h"

class Game {
  public:
    Game();
    ~Game();

    void Run();

  private:
    enum class Difficulty {
        Easy = 0,
        Normal = 1,
        Hard = 2
    };

    void ResetRun();
    void ApplyDifficultyProfile();
    void Update(float dt);
    void Draw() const;

    void UpdatePlaying(float dt);
    void UpdateProjectiles(float dt);
    void UpdateSpawnTelegraphs(float dt);
    void UpdateSuns(float dt);
    void UpdatePlants(float dt);
    void UpdateZombies(float dt);
    void UpdateMowers(float dt);
    void SpawnFallingSun();
    void SpawnZombie(ZombieType type, int lane);

    void DrawMenu() const;
    void DrawHowToPlay() const;
    void DrawPlaying() const;
    void DrawPauseOverlay() const;
    void DrawEndScreen(bool won) const;
    void DrawHud() const;
    void DrawSeedPackets() const;
    void DrawPauseButtons() const;

    void HandleMenuInput();
    void HandlePlayingInput();
    void HandlePausedInput();
    void HandleEndInput();
    void TryPlantAtMouse();
    bool TryCollectSun(Vector2 mousePos);
    void FinalizeRun(bool won);

    void RebuildLaneViews();
    PlacementFailReason CheckPlacement(int row, int col, PlantType type) const;
    Plant* FindPlantBeingEaten(const Zombie& zombie, const std::vector<Plant*>& lanePlants) const;
    int CountLivingZombies() const;
    void AddScreenShake(float amplitude, float duration);
    void UpdateFloatingTexts(float dt);
    void DrawPlacementPreview() const;
    void DrawToolTip() const;
    void PushFloatingText(Vector2 pos, const std::string& text, Color color);
    void PushPlacementPulse(Vector2 pos, Color color);
    void PushHitSpark(Vector2 pos, Color color, bool armorBreak);

    GameConfig config_;
    Board board_;
    WaveManager waveManager_;
    GameState state_ = GameState::Menu;

    std::vector<std::unique_ptr<Plant>> plants_;
    std::vector<std::unique_ptr<Zombie>> zombies_;
    std::vector<Projectile> projectiles_;
    std::vector<Projectile> enemyProjectiles_;
    std::vector<SunToken> suns_;
    std::vector<FloatingText> floatingTexts_;
    std::vector<std::pair<ZombieType, int>> summonRequests_;
    struct UiPulse {
        Vector2 position {0.0f, 0.0f};
        float ttl = 0.0f;
        Color color {255, 255, 255, 255};
    };
    std::vector<UiPulse> placementPulses_;
    struct HitSpark {
        Vector2 position {0.0f, 0.0f};
        Vector2 velocity {0.0f, 0.0f};
        float ttl = 0.0f;
        Color color {255, 255, 255, 255};
        bool armorBreak = false;
    };
    std::vector<HitSpark> hitSparks_;

    std::vector<std::vector<Plant*>> plantsByLane_;
    std::vector<std::vector<Zombie*>> zombiesByLane_;
    std::vector<Plant*> boardOccupancy_;
    struct SpawnTelegraph {
        ZombieType type = ZombieType::Basic;
        int lane = 0;
        float timer = 0.8f;
        float total = 0.8f;
    };
    std::vector<SpawnTelegraph> spawnTelegraphs_;

    std::vector<LaneMower> mowers_;
    std::vector<float> cooldowns_;

    PlantType selectedPlant_ = PlantType::Peashooter;
    Difficulty selectedDifficulty_ = Difficulty::Normal;
    bool shovelSelected_ = false;
    int sunPoints_ = 150;
    int runStartingSun_ = 150;
    int runSunValue_ = 25;
    float runFallingSunInterval_ = 6.0f;
    float runSpawnTelegraphDelay_ = 0.9f;
    float zombieHealthScale_ = 1.0f;
    float zombieSpeedScale_ = 1.0f;
    float zombieDamageScale_ = 1.0f;

    float fallingSunTimer_ = 3.0f;
    float gameResultTimer_ = 0.0f;
    float screenShakeTimer_ = 0.0f;
    float screenShakeAmp_ = 0.0f;
    float pauseOverlayAnim_ = 0.0f;
    float endOverlayAnim_ = 0.0f;
    float transitionTimer_ = 0.0f;
    std::string placementMessage_;
    float placementMessageTimer_ = 0.0f;

    mutable std::mt19937 gameplayRng_ {std::random_device {}()};
    mutable std::mt19937 visualRng_ {std::random_device {}()};
    ProceduralSprites sprites_;
    SaveSystem saveSystem_;
    SaveData saveData_ {};
    bool runEnded_ = false;

    bool audioReady_ = false;
    Sound sfxPlace_ {};
    Sound sfxCollect_ {};
    Sound sfxHit_ {};
    Sound sfxExplosion_ {};
    bool shouldClose_ = false;
};
