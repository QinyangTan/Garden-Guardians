#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
std::string ApplicationDirectory() {
    const char* dir = GetApplicationDirectory();
    if (dir == nullptr || dir[0] == '\0') {
        return {};
    }
    return std::string(dir);
}

// Prefer files next to the executable (release zip layout); fall back to current working dir for dev (e.g. build/ + cwd at repo root).
std::string MakeConfigPath() {
    constexpr const char* kRel = "assets/config/game.cfg";
    const std::string base = ApplicationDirectory();
    std::error_code ec;
    if (!base.empty()) {
        const fs::path bundled = fs::path(base) / kRel;
        if (fs::exists(bundled, ec)) {
            return bundled.string();
        }
    }
    const fs::path cwdRel(kRel);
    if (fs::exists(cwdRel, ec)) {
        return fs::absolute(cwdRel, ec).string();
    }
    if (!base.empty()) {
        return (fs::path(base) / kRel).string();
    }
    return std::string(kRel);
}

std::string MakeSavePath() {
    const std::string base = ApplicationDirectory();
    if (!base.empty()) {
        return (fs::path(base) / "save" / "profile.cfg").string();
    }
    return std::string("save/profile.cfg");
}

Rectangle MenuButtonRect(int index) {
    return Rectangle {500.0f, 248.0f + static_cast<float>(index) * 78.0f, 280.0f, 58.0f};
}

Rectangle PauseIconRect() {
    return Rectangle {1234.0f, 26.0f, 42.0f, 34.0f};
}

Rectangle PauseResumeRect() {
    return Rectangle {510.0f, 324.0f, 260.0f, 56.0f};
}

Rectangle PauseMenuRect() {
    return Rectangle {510.0f, 394.0f, 260.0f, 56.0f};
}

Rectangle EndRestartRect() {
    return Rectangle {444.0f, 432.0f, 182.0f, 52.0f};
}

Rectangle EndMenuRect() {
    return Rectangle {654.0f, 432.0f, 182.0f, 52.0f};
}

Rectangle ShovelRect() {
    return Rectangle {1176.0f, 26.0f, 50.0f, 34.0f};
}

Rectangle DifficultyRect(int index) {
    return Rectangle {370.0f + static_cast<float>(index) * 180.0f, 532.0f, 160.0f, 48.0f};
}

Rectangle CardRect(int index) {
    constexpr float startX = 212.0f;
    constexpr float startY = 18.0f;
    constexpr float w = 108.0f;
    constexpr float h = 40.0f;
    constexpr float gapX = 8.0f;
    constexpr float gapY = 8.0f;
    constexpr int perRow = 5;
    const int row = index / perRow;
    const int col = index % perRow;
    return Rectangle {
        startX + static_cast<float>(col) * (w + gapX),
        startY + static_cast<float>(row) * (h + gapY),
        w, h
    };
}

Color PlacementColor(PlacementFailReason reason) {
    if (reason == PlacementFailReason::None) {
        return Color {88, 220, 88, 120};
    }
    return Color {235, 88, 88, 120};
}

Color ButtonColor(const Rectangle& rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect) ? Color {96, 181, 255, 255} : Color {65, 130, 232, 255};
}

void DrawUiButton(Rectangle rect, const char* label, int fontSize, Color fill, Color border, Color textColor) {
    const bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    const bool pressed = hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const float scale = pressed ? 0.97f : (hover ? 1.02f : 1.0f);
    const Rectangle scaled {
        rect.x + rect.width * (1.0f - scale) * 0.5f,
        rect.y + rect.height * (1.0f - scale) * 0.5f + (pressed ? 2.0f : 0.0f),
        rect.width * scale,
        rect.height * scale
    };
    DrawRectangleRounded(Rectangle {scaled.x + 3.0f, scaled.y + 4.0f, scaled.width, scaled.height}, 0.22f, 6, Color {5, 10, 28, 115});
    DrawRectangleRounded(scaled, 0.22f, 6, fill);
    DrawRectangleRoundedLinesEx(scaled, 0.22f, 6, 2.0f, border);
    const int textW = MeasureText(label, fontSize);
    DrawText(label, static_cast<int>(scaled.x + (scaled.width - static_cast<float>(textW)) * 0.5f), static_cast<int>(scaled.y + (scaled.height - static_cast<float>(fontSize)) * 0.5f - 2.0f), fontSize, textColor);
}

void DrawCardGlyph(PlantType type, Rectangle area) {
    const Vector2 c {area.x + area.width * 0.5f, area.y + area.height * 0.5f};
    const Color outline {18, 32, 18, 90};
    switch (type) {
        case PlantType::Peashooter:
            DrawCircleV(Vector2 {c.x - 1.0f, c.y + 1.0f}, 10.0f, outline);
            DrawCircleV(c, 9.0f, Color {58, 155, 60, 255});
            DrawCircleV(Vector2 {c.x + 10.0f, c.y - 2.0f}, 5.0f, Color {45, 128, 51, 255});
            DrawCircleV(Vector2 {c.x + 4.0f, c.y - 3.0f}, 2.0f, Color {156, 230, 156, 220});
            break;
        case PlantType::SunBloom:
            DrawCircleV(c, 6.0f, GOLD);
            for (int i = 0; i < 6; ++i) {
                const float a = static_cast<float>(i) * 2.0f * PI / 6.0f;
                DrawCircleV(Vector2 {c.x + std::cos(a) * 10.0f, c.y + std::sin(a) * 10.0f}, 4.0f, YELLOW);
            }
            break;
        case PlantType::WallRoot:
            DrawRectangleRounded(Rectangle {c.x - 12.0f, c.y - 11.0f, 24.0f, 22.0f}, 0.2f, 4, outline);
            DrawRectangleRounded(Rectangle {c.x - 11.0f, c.y - 10.0f, 22.0f, 20.0f}, 0.2f, 4, Color {121, 84, 59, 255});
            break;
        case PlantType::Chomper:
            DrawCircleV(Vector2 {c.x - 1.0f, c.y + 1.0f}, 10.0f, outline);
            DrawCircleV(c, 9.0f, Color {179, 94, 208, 255});
            DrawTriangle(Vector2 {c.x - 2.0f, c.y}, Vector2 {c.x + 13.0f, c.y - 8.0f}, Vector2 {c.x + 13.0f, c.y + 8.0f}, Color {247, 220, 248, 255});
            break;
        case PlantType::FrostBurst:
            DrawCircleV(Vector2 {c.x, c.y + 1.0f}, 9.0f, outline);
            DrawCircleV(c, 8.0f, SKYBLUE);
            DrawCircleLines(static_cast<int>(c.x), static_cast<int>(c.y), 12.0f, WHITE);
            break;
        case PlantType::Repeater:
            DrawCircleV(Vector2 {c.x - 6.0f, c.y + 1.0f}, 8.0f, outline);
            DrawCircleV(Vector2 {c.x + 7.0f, c.y + 1.0f}, 8.0f, outline);
            DrawCircleV(Vector2 {c.x - 5.0f, c.y}, 7.0f, Color {40, 133, 50, 255});
            DrawCircleV(Vector2 {c.x + 8.0f, c.y}, 7.0f, Color {58, 155, 60, 255});
            break;
        case PlantType::IcePea:
            DrawCircleV(Vector2 {c.x, c.y + 1.0f}, 9.0f, outline);
            DrawCircleV(c, 8.0f, Color {140, 225, 255, 255});
            DrawCircleV(Vector2 {c.x + 6.0f, c.y - 5.0f}, 3.0f, WHITE);
            break;
        case PlantType::SpikeTrap:
            DrawRectangleRounded(Rectangle {area.x + 8.0f, area.y + area.height - 7.0f, 30.0f, 4.0f}, 0.3f, 3, outline);
            for (int i = 0; i < 4; ++i) {
                const float x = area.x + 10.0f + static_cast<float>(i) * 7.0f;
                DrawTriangle(Vector2 {x, area.y + area.height - 4.0f}, Vector2 {x + 6.0f, area.y + area.height - 4.0f}, Vector2 {x + 3.0f, area.y + 6.0f}, LIGHTGRAY);
            }
            break;
        case PlantType::BombBerry:
            DrawCircleV(Vector2 {c.x, c.y + 1.0f}, 9.0f, outline);
            DrawCircleV(c, 8.0f, Color {220, 95, 90, 255});
            DrawCircleV(Vector2 {c.x + 3.0f, c.y - 4.0f}, 3.0f, YELLOW);
            break;
        case PlantType::MagnetVine:
            DrawRectangleRounded(Rectangle {c.x - 11.0f, c.y - 9.0f, 22.0f, 18.0f}, 0.5f, 4, outline);
            DrawRectangleRounded(Rectangle {c.x - 10.0f, c.y - 8.0f, 20.0f, 16.0f}, 0.5f, 4, Color {171, 112, 233, 255});
            DrawRectangle(static_cast<int>(c.x - 8.0f), static_cast<int>(c.y - 2.0f), 16, 4, Color {225, 184, 255, 255});
            break;
        default: break;
    }
}

Sound CreateTone(float frequency, float durationSeconds, float volume) {
    const int sampleRate = 22050;
    const int frameCount = static_cast<int>(sampleRate * durationSeconds);
    std::vector<short> samples(static_cast<std::size_t>(frameCount));
    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        samples[static_cast<std::size_t>(i)] = static_cast<short>(std::sin(2.0f * PI * frequency * t) * 30000.0f);
    }

    Wave wave {};
    wave.frameCount = static_cast<unsigned int>(frameCount);
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples.data();
    Sound sfx = LoadSoundFromWave(wave);
    SetSoundVolume(sfx, volume);
    return sfx;
}
}  // namespace

Game::Game()
    : config_(ConfigLoader::Load(MakeConfigPath())),
      board_(config_),
      waveManager_(config_.totalWaves, config_.waveIntermission, config_.lanes),
      state_(GameState::Menu),
      selectedPlant_(PlantType::Peashooter),
      sunPoints_(config_.startingSun),
      fallingSunTimer_(2.5f),
      saveSystem_(MakeSavePath()) {
    InitWindow(config_.screenWidth, config_.screenHeight, "Garden Guardians");
    SetTargetFPS(60);

    InitAudioDevice();
    audioReady_ = IsAudioDeviceReady();
    if (audioReady_) {
        sfxPlace_ = CreateTone(600.0f, 0.08f, 0.35f);
        sfxCollect_ = CreateTone(900.0f, 0.06f, 0.30f);
        sfxHit_ = CreateTone(320.0f, 0.06f, 0.30f);
        sfxExplosion_ = CreateTone(180.0f, 0.14f, 0.28f);
    }

    sprites_.Initialize();
    saveData_ = saveSystem_.Load();
    plantsByLane_.assign(config_.lanes, {});
    zombiesByLane_.assign(config_.lanes, {});
    boardOccupancy_.assign(static_cast<std::size_t>(config_.lanes * config_.cols), nullptr);
    mowers_.assign(config_.lanes, {});
    cooldowns_.assign(PlantTypeCount(), 0.0f);
    ApplyDifficultyProfile();
    ResetRun();
}

Game::~Game() {
    sprites_.Shutdown();
    if (audioReady_) {
        UnloadSound(sfxPlace_);
        UnloadSound(sfxCollect_);
        UnloadSound(sfxHit_);
        UnloadSound(sfxExplosion_);
        CloseAudioDevice();
    }
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose() && !shouldClose_) {
        const float dt = GetFrameTime();
        Update(dt);
        if (shouldClose_) {
            break;
        }
        Draw();
    }
}

void Game::ResetRun() {
    plants_.clear();
    zombies_.clear();
    projectiles_.clear();
    suns_.clear();
    spawnTelegraphs_.clear();
    floatingTexts_.clear();
    placementPulses_.clear();
    hitSparks_.clear();
    summonRequests_.clear();
    waveManager_.Reset();

    std::fill(cooldowns_.begin(), cooldowns_.end(), 0.0f);

    sunPoints_ = runStartingSun_;
    selectedPlant_ = PlantType::Peashooter;
    shovelSelected_ = false;
    fallingSunTimer_ = std::max(2.0f, runFallingSunInterval_ * 0.55f);
    gameResultTimer_ = 0.0f;
    screenShakeTimer_ = 0.0f;
    screenShakeAmp_ = 0.0f;
    pauseOverlayAnim_ = 0.0f;
    endOverlayAnim_ = 0.0f;
    transitionTimer_ = 0.2f;
    placementMessage_.clear();
    placementMessageTimer_ = 0.0f;
    runEnded_ = false;

    for (int lane = 0; lane < config_.lanes; ++lane) {
        mowers_[static_cast<std::size_t>(lane)] = LaneMower {};
        mowers_[static_cast<std::size_t>(lane)].x = board_.LeftDangerX() - 18.0f;
    }
    RebuildLaneViews();
}

void Game::ApplyDifficultyProfile() {
    int waves = config_.totalWaves;
    float intermission = config_.waveIntermission;
    float spawnIntervalScale = 1.0f;
    float quotaScale = 1.0f;
    float eliteScale = 1.0f;

    switch (selectedDifficulty_) {
        case Difficulty::Easy:
            runStartingSun_ = config_.startingSun + 75;
            runSunValue_ = config_.fallingSunValue + 5;
            runFallingSunInterval_ = std::max(7.0f, config_.fallingSunInterval - 0.3f);
            runSpawnTelegraphDelay_ = 1.20f;
            zombieHealthScale_ = 0.82f;
            zombieSpeedScale_ = 0.9f;
            zombieDamageScale_ = 0.86f;
            waves = std::max(4, config_.totalWaves - 1);
            intermission += 0.8f;
            spawnIntervalScale = 1.24f;
            quotaScale = 0.85f;
            eliteScale = 0.82f;
            break;
        case Difficulty::Hard:
            runStartingSun_ = std::max(50, config_.startingSun - 40);
            runSunValue_ = std::max(15, config_.fallingSunValue - 5);
            runFallingSunInterval_ = config_.fallingSunInterval + 1.8f;
            runSpawnTelegraphDelay_ = 0.80f;
            zombieHealthScale_ = 1.20f;
            zombieSpeedScale_ = 1.14f;
            zombieDamageScale_ = 1.16f;
            waves = config_.totalWaves + 1;
            intermission = std::max(2.5f, config_.waveIntermission - 0.7f);
            spawnIntervalScale = 1.02f;
            quotaScale = 1.16f;
            eliteScale = 1.18f;
            break;
        case Difficulty::Normal:
        default:
            runStartingSun_ = config_.startingSun;
            runSunValue_ = config_.fallingSunValue;
            runFallingSunInterval_ = config_.fallingSunInterval + 0.8f;
            runSpawnTelegraphDelay_ = 1.0f;
            zombieHealthScale_ = 1.0f;
            zombieSpeedScale_ = 1.0f;
            zombieDamageScale_ = 1.0f;
            waves = config_.totalWaves;
            intermission = config_.waveIntermission;
            spawnIntervalScale = 1.15f;
            quotaScale = 1.0f;
            eliteScale = 1.0f;
            break;
    }

    waveManager_ = WaveManager(waves, intermission, config_.lanes);
    waveManager_.ConfigureDifficulty(spawnIntervalScale, quotaScale, eliteScale);
}

void Game::Update(float dt) {
    const GameState before = state_;
    switch (state_) {
        case GameState::Menu: HandleMenuInput(); break;
        case GameState::HowToPlay:
            if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                state_ = GameState::Menu;
            }
            break;
        case GameState::Playing:
            if (IsKeyPressed(KEY_ESCAPE)) {
                state_ = GameState::Paused;
            } else {
                HandlePlayingInput();
                UpdatePlaying(dt);
            }
            break;
        case GameState::Paused:
            HandlePausedInput();
            break;
        case GameState::Win:
        case GameState::Lose: HandleEndInput(); break;
    }

    if (state_ == GameState::Paused) {
        pauseOverlayAnim_ = std::min(1.0f, pauseOverlayAnim_ + dt * 5.0f);
    } else {
        pauseOverlayAnim_ = std::max(0.0f, pauseOverlayAnim_ - dt * 6.0f);
    }
    if (state_ == GameState::Win || state_ == GameState::Lose) {
        endOverlayAnim_ = std::min(1.0f, endOverlayAnim_ + dt * 4.0f);
    } else {
        endOverlayAnim_ = std::max(0.0f, endOverlayAnim_ - dt * 6.0f);
    }
    transitionTimer_ = std::max(0.0f, transitionTimer_ - dt);
    placementMessageTimer_ = std::max(0.0f, placementMessageTimer_ - dt);
    if (before != state_) {
        transitionTimer_ = 0.22f;
    }
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground(Color {33, 58, 31, 255});

    switch (state_) {
        case GameState::Menu: DrawMenu(); break;
        case GameState::HowToPlay: DrawHowToPlay(); break;
        case GameState::Playing: DrawPlaying(); break;
        case GameState::Paused:
            DrawPlaying();
            DrawPauseOverlay();
            break;
        case GameState::Win: DrawEndScreen(true); break;
        case GameState::Lose: DrawEndScreen(false); break;
    }

    if (transitionTimer_ > 0.0f) {
        const float ratio = transitionTimer_ / 0.22f;
        DrawRectangle(0, 0, config_.screenWidth, config_.screenHeight, Color {0, 0, 0, static_cast<unsigned char>(ratio * 120.0f)});
    }

    EndDrawing();
}

void Game::UpdatePlaying(float dt) {
    for (float& cooldown : cooldowns_) {
        cooldown = std::max(0.0f, cooldown - dt);
    }

    fallingSunTimer_ -= dt;
    if (fallingSunTimer_ <= 0.0f) {
        SpawnFallingSun();
        fallingSunTimer_ = runFallingSunInterval_;
    }

    waveManager_.Update(dt, CountLivingZombies(), [this](ZombieType type, int lane) {
        SpawnTelegraph telegraph;
        telegraph.type = type;
        telegraph.lane = lane;
        telegraph.total = runSpawnTelegraphDelay_;
        telegraph.timer = telegraph.total;
        spawnTelegraphs_.push_back(telegraph);
    });

    screenShakeTimer_ = std::max(0.0f, screenShakeTimer_ - dt);
    if (screenShakeTimer_ <= 0.0f) {
        screenShakeAmp_ = 0.0f;
    }

    RebuildLaneViews();
    UpdateSpawnTelegraphs(dt);
    UpdatePlants(dt);
    UpdateProjectiles(dt);
    UpdateZombies(dt);
    UpdateMowers(dt);
    UpdateSuns(dt);
    UpdateFloatingTexts(dt);

    for (const auto& summon : summonRequests_) {
        SpawnZombie(summon.first, summon.second);
    }
    summonRequests_.clear();

    plants_.erase(std::remove_if(plants_.begin(), plants_.end(), [](const auto& plant) { return !plant->IsAlive(); }), plants_.end());
    zombies_.erase(std::remove_if(zombies_.begin(), zombies_.end(), [](const auto& zombie) { return !zombie->IsAlive(); }), zombies_.end());
    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), [](const Projectile& projectile) { return !projectile.alive; }), projectiles_.end());
    enemyProjectiles_.erase(std::remove_if(enemyProjectiles_.begin(), enemyProjectiles_.end(), [](const Projectile& projectile) { return !projectile.alive; }), enemyProjectiles_.end());
    suns_.erase(std::remove_if(suns_.begin(), suns_.end(), [](const SunToken& token) { return !token.alive; }), suns_.end());
    floatingTexts_.erase(std::remove_if(floatingTexts_.begin(), floatingTexts_.end(), [](const FloatingText& text) { return text.ttl <= 0.0f; }), floatingTexts_.end());
    RebuildLaneViews();

    if (waveManager_.IsFinalWaveCleared(CountLivingZombies())) {
        gameResultTimer_ += dt;
        if (gameResultTimer_ >= config_.victoryDelay) {
            FinalizeRun(true);
        }
    }
}

void Game::UpdateProjectiles(float dt) {
    const Rectangle boardBounds = board_.GridBounds();
    for (auto& projectile : projectiles_) {
        if (!projectile.alive) {
            continue;
        }
        projectile.x += projectile.speed * dt;
        if (projectile.x > boardBounds.x + boardBounds.width + 100.0f) {
            projectile.alive = false;
            continue;
        }

        const int lane = projectile.row;
        if (lane < 0 || lane >= static_cast<int>(zombiesByLane_.size())) {
            projectile.alive = false;
            continue;
        }

        for (Zombie* zombie : zombiesByLane_[static_cast<std::size_t>(lane)]) {
            if (!zombie->IsAlive()) {
                continue;
            }
            const Rectangle body = zombie->BodyRect();
            if (CheckCollisionPointRec(Vector2 {projectile.x, projectile.y}, body)) {
                const float armorBefore = zombie->Armor();
                zombie->ReceiveHit(projectile.damage, projectile.slowing, projectile.slowFactor, projectile.slowDuration);
                const bool armorBreak = armorBefore > 0.0f && zombie->Armor() <= 0.0f;
                PushHitSpark(Vector2 {projectile.x, projectile.y}, armorBreak ? Color {255, 196, 116, 255} : (projectile.slowing ? SKYBLUE : Color {255, 246, 220, 255}), armorBreak);
                AddScreenShake(projectile.slowing ? 2.2f : 1.4f, 0.08f);
                if (audioReady_) {
                    PlaySound(sfxHit_);
                }
                if (projectile.damage >= 18.0f) {
                    PushFloatingText(Vector2 {projectile.x, projectile.y - 8.0f}, TextFormat("-%d", static_cast<int>(projectile.damage)), Color {255, 240, 240, 255});
                }
                projectile.alive = false;
                break;
            }
        }
    }

    for (auto& projectile : enemyProjectiles_) {
        if (!projectile.alive) {
            continue;
        }
        projectile.x -= projectile.speed * dt;
        if (projectile.x < boardBounds.x - 120.0f) {
            projectile.alive = false;
            continue;
        }
        const int lane = projectile.row;
        if (lane < 0 || lane >= static_cast<int>(plantsByLane_.size())) {
            projectile.alive = false;
            continue;
        }
        for (Plant* plant : plantsByLane_[static_cast<std::size_t>(lane)]) {
            if (!plant->IsAlive()) {
                continue;
            }
            if (CheckCollisionPointRec(Vector2 {projectile.x, projectile.y}, plant->Rect())) {
                plant->Damage(projectile.damage);
                PushHitSpark(Vector2 {projectile.x, projectile.y}, Color {140, 255, 170, 255}, false);
                projectile.alive = false;
                PushFloatingText(Vector2 {plant->Center().x, plant->Center().y - 18.0f}, TextFormat("-%d", static_cast<int>(projectile.damage)), Color {255, 180, 160, 255});
                break;
            }
        }
    }
}

void Game::UpdateSpawnTelegraphs(float dt) {
    for (auto& telegraph : spawnTelegraphs_) {
        telegraph.timer -= dt;
        if (telegraph.timer <= 0.0f) {
            SpawnZombie(telegraph.type, telegraph.lane);
        }
    }
    spawnTelegraphs_.erase(std::remove_if(spawnTelegraphs_.begin(), spawnTelegraphs_.end(), [](const SpawnTelegraph& telegraph) { return telegraph.timer <= 0.0f; }), spawnTelegraphs_.end());
}

void Game::UpdateSuns(float dt) {
    const float now = static_cast<float>(GetTime());
    for (auto& sun : suns_) {
        if (!sun.alive) {
            continue;
        }
        if (!sun.fromPlant) {
            sun.position.y += sun.fallSpeed * dt;
            const float floorY = board_.GridBounds().y + board_.GridBounds().height - 30.0f;
            if (sun.position.y > floorY) {
                sun.position.y = floorY;
            }
        } else {
            if (sun.jumpTimer > 0.0f) {
                // Short ballistic jump from SunBloom to a nearby random spot.
                sun.velocity.y += 520.0f * dt;
                sun.position.x += sun.velocity.x * dt;
                sun.position.y += sun.velocity.y * dt;
                sun.jumpTimer -= dt;
                if (sun.jumpTimer <= 0.0f) {
                    sun.jumpTimer = 0.0f;
                    sun.velocity = Vector2 {0.0f, 0.0f};
                }
            } else {
                sun.position.y += std::sin(now * 4.0f) * 4.0f * dt;
            }
        }

        sun.ttl -= dt;
        if (sun.ttl <= 0.0f) {
            sun.alive = false;
        }
    }
}

void Game::UpdatePlants(float dt) {
    for (auto& plant : plants_) {
        if (!plant->IsAlive()) {
            continue;
        }
        plant->TickVisual(dt);
        plant->Update(dt, board_, zombiesByLane_, projectiles_, suns_);
        if (plant->Type() == PlantType::FrostBurst && !plant->IsAlive() && audioReady_) {
            PlaySound(sfxExplosion_);
            AddScreenShake(5.0f, 0.22f);
        }
    }
}

void Game::UpdateZombies(float dt) {
    for (int lane = 0; lane < config_.lanes; ++lane) {
        auto& laneZombies = zombiesByLane_[static_cast<std::size_t>(lane)];
        auto& lanePlants = plantsByLane_[static_cast<std::size_t>(lane)];
        for (Zombie* zombie : laneZombies) {
            if (!zombie->IsAlive()) {
                continue;
            }

            zombie->Update(dt);
            zombie->TickSpecial(dt);
            bool handledSpecial = false;

            if (zombie->Type() == ZombieType::PoleJump && zombie->CanUseJump()) {
                for (Plant* plant : lanePlants) {
                    if (!plant->IsAlive() || !plant->BlocksMovement()) {
                        continue;
                    }
                    if (zombie->X() > plant->Center().x && std::fabs(zombie->X() - plant->Center().x) < 38.0f) {
                        zombie->SetX(plant->Center().x - board_.Config().cellWidth * 0.55f);
                        zombie->MarkJumpUsed();
                        zombie->SetBaseSpeed(18.0f);
                        AddScreenShake(2.6f, 0.1f);
                        handledSpecial = true;
                        break;
                    }
                }
            }

            if (zombie->Type() == ZombieType::Spitter) {
                Plant* rangedTarget = nullptr;
                float rangeDist = 9999.0f;
                for (Plant* plant : lanePlants) {
                    if (!plant->IsAlive() || !plant->BlocksMovement()) {
                        continue;
                    }
                    const float d = zombie->X() - plant->Center().x;
                    if (d > 40.0f && d < 300.0f && d < rangeDist) {
                        rangeDist = d;
                        rangedTarget = plant;
                    }
                }
                if (rangedTarget != nullptr && zombie->SpecialTimer() <= 0.0f) {
                    Projectile spit;
                    spit.row = lane;
                    spit.x = zombie->X() - 12.0f;
                    spit.y = zombie->BodyRect().y + 18.0f;
                    spit.speed = 230.0f;
                    spit.damage = 16.0f;
                    spit.fromZombie = true;
                    enemyProjectiles_.push_back(spit);
                    zombie->SetSpecialTimer(1.6f);
                    zombie->SetCasting(true);
                    handledSpecial = true;
                } else {
                    zombie->SetCasting(false);
                }
            }

            if (zombie->Type() == ZombieType::Summoner && zombie->SpecialTimer() <= 0.0f) {
                int laneAlive = 0;
                for (Zombie* z : laneZombies) {
                    if (z->IsAlive()) {
                        laneAlive++;
                    }
                }
                if (laneAlive < 12) {
                    summonRequests_.push_back({ZombieType::Imp, lane});
                    zombie->SetSpecialTimer(8.0f);
                    zombie->SetCasting(true);
                }
            } else if (zombie->Type() == ZombieType::Summoner && zombie->SpecialTimer() < 7.2f) {
                zombie->SetCasting(false);
            }

            Plant* target = FindPlantBeingEaten(*zombie, lanePlants);
            zombie->SetStopped(target != nullptr || handledSpecial);
            if (target != nullptr) {
                target->Damage(zombie->Dps() * dt);
            } else if (!handledSpecial) {
                zombie->Move(dt);
            }

            if (zombie->X() <= board_.LeftDangerX() + 20.0f) {
                LaneMower& mower = mowers_[static_cast<std::size_t>(lane)];
                if (mower.available && !mower.active) {
                    mower.active = true;
                    mower.available = false;
                    mower.x = board_.LeftDangerX() - 10.0f;
                    AddScreenShake(4.0f, 0.25f);
                } else if (!mower.active && zombie->X() < board_.LeftDangerX()) {
                    FinalizeRun(false);
                }
            }
        }
    }
}

void Game::UpdateMowers(float dt) {
    for (int lane = 0; lane < config_.lanes; ++lane) {
        LaneMower& mower = mowers_[static_cast<std::size_t>(lane)];
        if (!mower.active) {
            continue;
        }

        mower.x += 520.0f * dt;
        bool hit = false;
        for (Zombie* zombie : zombiesByLane_[static_cast<std::size_t>(lane)]) {
            if (!zombie->IsAlive()) {
                continue;
            }
            if (std::fabs(zombie->X() - mower.x) < 40.0f) {
                zombie->Kill();
                hit = true;
            }
        }
        if (hit) {
            AddScreenShake(3.8f, 0.14f);
        }

        if (mower.x > board_.ZombieSpawnX() + 100.0f) {
            mower.active = false;
        }
    }
}

void Game::SpawnFallingSun() {
    std::uniform_real_distribution<float> xDist(board_.GridBounds().x + 40.0f, board_.GridBounds().x + board_.GridBounds().width - 40.0f);
    std::uniform_real_distribution<float> yDist(board_.GridBounds().y - 80.0f, board_.GridBounds().y - 20.0f);

    SunToken token;
    token.position = Vector2 {xDist(gameplayRng_), yDist(gameplayRng_)};
    token.fallSpeed = 45.0f;
    token.ttl = 8.0f;
    token.value = runSunValue_;
    token.fromPlant = false;
    suns_.push_back(token);
}

void Game::SpawnZombie(ZombieType type, int lane) {
    if (lane < 0 || lane >= config_.lanes) {
        return;
    }
    const float laneY = board_.CellCenter(lane, config_.cols - 1).y + 8.0f;
    auto zombie = CreateZombie(type, lane, board_.ZombieSpawnX(), laneY);
    zombie->ApplyMultipliers(zombieHealthScale_, zombieSpeedScale_, zombieDamageScale_);
    zombies_.push_back(std::move(zombie));
}

void Game::DrawMenu() const {
    const float now = static_cast<float>(GetTime());
    DrawRectangleGradientV(0, 0, config_.screenWidth, config_.screenHeight, Color {19, 72, 31, 255}, Color {14, 54, 24, 255});
    for (int i = 0; i < 24; ++i) {
        const float x = 60.0f + static_cast<float>(i) * 50.0f + std::sin(now * 0.4f + static_cast<float>(i)) * 18.0f;
        const float y = 580.0f + std::sin(now * 0.8f + static_cast<float>(i) * 0.5f) * 12.0f;
        DrawCircleV(Vector2 {x, y}, 12.0f + static_cast<float>(i % 3) * 2.0f, Color {32, 108, 44, 100});
    }
    DrawRectangleRounded(Rectangle {320.0f, 82.0f, 640.0f, 470.0f}, 0.04f, 8, Color {8, 25, 12, 90});
    DrawRectangleRoundedLinesEx(Rectangle {320.0f, 82.0f, 640.0f, 470.0f}, 0.04f, 8, 2.0f, Color {85, 140, 95, 120});

    DrawText("Garden Guardians", 334, 114, 66, Color {236, 255, 190, 255});
    DrawText("A lane-defense strategy game in modern C++", 360, 192, 24, Color {224, 240, 214, 255});

    const Rectangle startBtn = MenuButtonRect(0);
    const Rectangle howBtn = MenuButtonRect(1);
    const Rectangle quitBtn = MenuButtonRect(2);
    DrawUiButton(startBtn, "Start Game", 31, ButtonColor(startBtn), Color {165, 221, 255, 160}, WHITE);
    DrawUiButton(howBtn, "How to Play", 31, ButtonColor(howBtn), Color {165, 221, 255, 160}, WHITE);
    DrawUiButton(quitBtn, "Quit", 31, ButtonColor(quitBtn), Color {165, 221, 255, 160}, WHITE);

    DrawText("Difficulty", 541, 506, 30, Color {255, 245, 194, 255});
    DrawRectangleRounded(Rectangle {292.0f, 534.0f, 428.0f, 60.0f}, 0.25f, 6, Color {8, 24, 45, 145});
    const char* labels[3] = {"Easy", "Normal", "Hard"};
    for (int i = 0; i < 3; ++i) {
        const Rectangle rect = DifficultyRect(i);
        const bool selected = static_cast<int>(selectedDifficulty_) == i;
        const Color fill = selected ? Color {255, 211, 124, 255} : (CheckCollisionPointRec(GetMousePosition(), rect) ? Color {84, 145, 228, 255} : Color {54, 124, 214, 255});
        DrawUiButton(rect, labels[i], 28, fill, selected ? Color {255, 241, 184, 255} : Color {180, 220, 255, 90}, WHITE);
    }
}

void Game::DrawHowToPlay() const {
    DrawText("How to Play", 500, 70, 54, Color {236, 255, 190, 255});
    DrawText("- Collect suns by clicking them", 260, 180, 30, WHITE);
    DrawText("- Pick a seed packet, then click a lawn tile to plant", 260, 230, 30, WHITE);
    DrawText("- Plants attack automatically by lane or range", 260, 280, 30, WHITE);
    DrawText("- Stop zombies before they enter the house", 260, 330, 30, WHITE);
    DrawText("- Survive all waves to win", 260, 380, 30, WHITE);
    DrawText("- Pick difficulty in main menu before starting", 260, 430, 30, WHITE);
    DrawText("Mouse: select/plant/collect   Esc: pause   R: restart", 250, 500, 30, Color {255, 228, 140, 255});
    DrawText("Press Esc or click to return", 430, 600, 28, SKYBLUE);
}

void Game::DrawPlaying() const {
    DrawRectangleGradientV(0, 0, config_.screenWidth, config_.screenHeight, Color {124, 191, 255, 255}, Color {90, 144, 90, 255});
    float shakeX = 0.0f;
    float shakeY = 0.0f;
    if (screenShakeTimer_ > 0.0f && screenShakeAmp_ > 0.0f) {
        std::uniform_real_distribution<float> dist(-screenShakeAmp_, screenShakeAmp_);
        shakeX = dist(visualRng_);
        shakeY = dist(visualRng_);
    }
    const Camera2D camera {Vector2 {shakeX, shakeY}, Vector2 {0.0f, 0.0f}, 0.0f, 1.0f};
    BeginMode2D(camera);
    board_.Draw();

    const float now = static_cast<float>(GetTime());
    for (int lane = 0; lane < config_.lanes; ++lane) {
        const LaneMower& mower = mowers_[static_cast<std::size_t>(lane)];
        if (!mower.available && !mower.active) {
            continue;
        }
        const float mowerY = board_.CellCenter(lane, 0).y + 18.0f;
        DrawRectangleRounded(Rectangle {mower.x - 25.0f, mowerY - 18.0f, 50.0f, 36.0f}, 0.2f, 3, Color {180, 20, 20, 255});
        DrawCircle(static_cast<int>(mower.x - 14.0f), static_cast<int>(mowerY + 16.0f), 7.0f, BLACK);
        DrawCircle(static_cast<int>(mower.x + 14.0f), static_cast<int>(mowerY + 16.0f), 7.0f, BLACK);
    }

    for (const auto& sun : suns_) {
        if (sun.fromPlant) {
            // SunBloom suns use a greener-yellow tint and a pulse ring for quick visual distinction.
            const float pulse = 1.0f + 0.16f * std::sin(now * 8.0f);
            const float outerR = 21.0f * pulse;
            const bool warning = sun.ttl <= 2.0f;
            const bool blinkOn = (static_cast<int>(now * 10.0f) % 2) == 0;
            const Color core = warning && blinkOn ? Color {255, 165, 120, 255} : Color {225, 255, 128, 255};
            const Color ring = warning ? Color {255, 96, 88, 255} : Color {126, 222, 110, 255};
            DrawCircleV(sun.position, outerR, Color {170, 255, 150, 65});
            DrawCircleV(sun.position, 19.0f, core);
            DrawCircleLines(static_cast<int>(sun.position.x), static_cast<int>(sun.position.y), 19.0f, ring);
            DrawCircleLines(static_cast<int>(sun.position.x), static_cast<int>(sun.position.y), outerR, warning ? Color {255, 112, 102, 230} : Color {145, 255, 170, 220});
            DrawText("+", static_cast<int>(sun.position.x - 6.0f), static_cast<int>(sun.position.y - 12.0f), 22, warning ? Color {185, 65, 55, 255} : Color {90, 170, 70, 255});
            if (warning && blinkOn) {
                DrawText("!", static_cast<int>(sun.position.x + 14.0f), static_cast<int>(sun.position.y - 26.0f), 20, Color {255, 80, 70, 255});
            }
        } else {
            DrawCircleV(sun.position, 19.0f, Color {255, 220, 80, 255});
            DrawCircleLines(static_cast<int>(sun.position.x), static_cast<int>(sun.position.y), 19.0f, GOLD);
            DrawText("+", static_cast<int>(sun.position.x - 6.0f), static_cast<int>(sun.position.y - 12.0f), 22, ORANGE);
        }
    }

    for (const auto& telegraph : spawnTelegraphs_) {
        const float t = std::max(0.0f, telegraph.timer / telegraph.total);
        const Vector2 c = board_.CellCenter(telegraph.lane, config_.cols - 1);
        const float pulse = 1.0f + 0.16f * std::sin(now * 18.0f);
        const float r = (26.0f + (1.0f - t) * 10.0f) * pulse;
        DrawCircleLines(static_cast<int>(board_.ZombieSpawnX() - 18.0f), static_cast<int>(c.y), r, RED);
        DrawText(TextFormat("%s", ZombieName(telegraph.type).c_str()), static_cast<int>(board_.ZombieSpawnX() - 78.0f), static_cast<int>(c.y - 44.0f), 16, Color {255, 220, 220, 255});
    }

    for (const auto& plant : plants_) {
        plant->Draw();
        if (plant->Type() != PlantType::Chomper && plant->Type() != PlantType::SunBloom) {
            sprites_.DrawPlant(plant->Type(), plant->Center(), now);
        }
    }
    for (const auto& pulse : placementPulses_) {
        const float ratio = std::max(0.0f, pulse.ttl / 0.42f);
        const float radius = 16.0f + (1.0f - ratio) * 36.0f;
        const unsigned char alpha = static_cast<unsigned char>(190.0f * ratio);
        DrawCircleLines(static_cast<int>(pulse.position.x), static_cast<int>(pulse.position.y), radius, Color {pulse.color.r, pulse.color.g, pulse.color.b, alpha});
    }
    for (const auto& projectile : projectiles_) {
        const Color projectileColor = projectile.slowing ? SKYBLUE : Color {70, 180, 70, 255};
        DrawCircle(static_cast<int>(projectile.x), static_cast<int>(projectile.y), 8.0f, projectileColor);
        DrawCircleLines(static_cast<int>(projectile.x), static_cast<int>(projectile.y), 10.0f, Color {255, 255, 255, 90});
    }
    for (const auto& zombie : zombies_) {
        zombie->Draw();
        sprites_.DrawZombie(zombie->Type(), zombie->BodyRect(), now);
    }
    for (const auto& spit : enemyProjectiles_) {
        DrawCircle(static_cast<int>(spit.x), static_cast<int>(spit.y), 7.0f, Color {95, 250, 130, 255});
        DrawCircleLines(static_cast<int>(spit.x), static_cast<int>(spit.y), 7.0f, Color {40, 120, 60, 255});
    }
    for (const auto& spark : hitSparks_) {
        const float ratio = std::max(0.0f, spark.ttl / (spark.armorBreak ? 0.34f : 0.22f));
        const unsigned char alpha = static_cast<unsigned char>(220.0f * ratio);
        DrawCircleV(spark.position, spark.armorBreak ? 3.5f : 2.2f, Color {spark.color.r, spark.color.g, spark.color.b, alpha});
        if (spark.armorBreak) {
            DrawLineEx(spark.position, Vector2 {spark.position.x + spark.velocity.x * 0.02f, spark.position.y + spark.velocity.y * 0.02f}, 2.0f, Color {255, 201, 140, alpha});
        }
    }
    for (const auto& text : floatingTexts_) {
        DrawText(text.text.c_str(), static_cast<int>(text.position.x), static_cast<int>(text.position.y), 18, text.color);
    }
    EndMode2D();

    DrawHud();
    DrawSeedPackets();
    DrawPlacementPreview();
    DrawToolTip();
    if (placementMessageTimer_ > 0.0f) {
        DrawRectangleRounded(Rectangle {424.0f, 664.0f, 442.0f, 38.0f}, 0.22f, 5, Color {18, 18, 18, 170});
        DrawRectangleRoundedLinesEx(Rectangle {424.0f, 664.0f, 442.0f, 38.0f}, 0.22f, 5, 1.5f, Color {255, 223, 151, 160});
        DrawText(placementMessage_.c_str(), 444, 673, 19, Color {255, 236, 192, 255});
    }
}

void Game::DrawPauseOverlay() const {
    const unsigned char shade = static_cast<unsigned char>(130.0f + pauseOverlayAnim_ * 70.0f);
    DrawRectangle(0, 0, config_.screenWidth, config_.screenHeight, Color {0, 0, 0, shade});
    const Rectangle panel {430.0f, 222.0f, 420.0f, 268.0f};
    DrawRectangleRounded(panel, 0.08f, 8, Color {18, 24, 34, 228});
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, Color {123, 174, 255, 170});
    const int titleY = static_cast<int>(262.0f - (1.0f - pauseOverlayAnim_) * 14.0f);
    DrawText("PAUSED", 542, titleY, 48, WHITE);
    DrawPauseButtons();
    DrawText("Esc to resume", 547, 462, 22, Color {236, 255, 190, 255});
}

void Game::DrawEndScreen(bool won) const {
    DrawPlaying();
    const unsigned char alpha = static_cast<unsigned char>(120.0f + endOverlayAnim_ * 70.0f);
    DrawRectangle(0, 0, config_.screenWidth, config_.screenHeight, Color {0, 0, 0, alpha});
    const Rectangle panel {352.0f, 176.0f, 576.0f, 368.0f};
    DrawRectangleRounded(panel, 0.08f, 8, Color {18, 24, 34, 240});
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.5f, won ? Color {176, 255, 170, 190} : Color {255, 168, 168, 190});
    const int titleY = static_cast<int>(214.0f + (1.0f - endOverlayAnim_) * 10.0f);
    DrawText(won ? "YOU SURVIVED!" : "THE HOUSE FELL!", 400, titleY, 52, won ? Color {200, 255, 170, 255} : Color {255, 148, 148, 255});
    DrawRectangleRounded(Rectangle {panel.x + 36.0f, 276.0f, panel.width - 72.0f, 2.0f}, 0.5f, 4, Color {255, 255, 255, 35});
    DrawText(won ? "The garden is safe for now." : "Zombies broke through the home line.", 392, 288, 22, Color {232, 238, 248, 255});
    DrawText(won ? "Plant wisely to beat even tougher waves next time." : "Try stronger defense and earlier lane control next round.", 372, 318, 18, Color {176, 192, 212, 255});
    DrawRectangleRounded(Rectangle {panel.x + 40.0f, 348.0f, panel.width - 80.0f, 58.0f}, 0.16f, 6, Color {11, 17, 24, 200});
    DrawRectangleRoundedLinesEx(Rectangle {panel.x + 40.0f, 348.0f, panel.width - 80.0f, 58.0f}, 0.16f, 6, 1.0f, Color {80, 100, 130, 90});
    DrawText(TextFormat("Best wave %d", saveData_.bestWaveReached), 378, 364, 22, Color {255, 235, 120, 255});
    DrawText(TextFormat("Wins %d", saveData_.totalWins), 558, 364, 22, Color {220, 245, 220, 255});
    DrawText(TextFormat("Games %d", saveData_.gamesPlayed), 718, 364, 22, Color {220, 236, 255, 255});
    DrawUiButton(EndRestartRect(), "Restart", 28, Color {65, 130, 232, 255}, Color {165, 221, 255, 160}, WHITE);
    DrawUiButton(EndMenuRect(), "Main Menu", 28, Color {65, 130, 232, 255}, Color {165, 221, 255, 160}, WHITE);
    DrawText("R restart   ·   M main menu", 508, 508, 18, Color {220, 236, 180, 255});
}

void Game::DrawHud() const {
    DrawRectangleRounded(Rectangle {8.0f, 8.0f, static_cast<float>(config_.screenWidth - 16), 122.0f}, 0.04f, 8, Color {20, 38, 56, 118});
    DrawRectangleRounded(Rectangle {14.0f, 14.0f, 168.0f, 52.0f}, 0.16f, 6, Color {24, 32, 42, 196});
    DrawCircle(42, 40, 15.0f, GOLD);
    DrawCircleLines(42, 40, 15.0f, Color {255, 246, 183, 255});
    DrawText("Sun", 66, 20, 18, Color {255, 236, 180, 220});
    DrawText(TextFormat("%d", sunPoints_), 65, 37, 28, WHITE);

    // Status block ends before tools column so stats never overlap shovel / pause hit targets
    constexpr float statX = 1018.0f;
    constexpr float statW = 150.0f;
    DrawRectangleRounded(Rectangle {statX, 14.0f, statW, 108.0f}, 0.12f, 6, Color {22, 28, 36, 220});
    DrawRectangleRoundedLinesEx(Rectangle {statX, 14.0f, statW, 108.0f}, 0.12f, 6, 1.0f, Color {90, 120, 160, 90});
    const int waveShown = waveManager_.CurrentWave();
    const int sx = static_cast<int>(statX + 14.0f);
    DrawText("WAVE", sx, 18, 14, Color {198, 218, 245, 255});
    DrawText(TextFormat("%d / %d", waveShown, waveManager_.TotalWaves()), sx + 52, 14, 28, WHITE);
    const char* difficultyText = "Normal";
    if (selectedDifficulty_ == Difficulty::Easy) {
        difficultyText = "Easy";
    } else if (selectedDifficulty_ == Difficulty::Hard) {
        difficultyText = "Hard";
    }
    const float ratio = static_cast<float>(waveManager_.SpawnedThisWave()) / static_cast<float>(std::max(1, waveManager_.WaveQuota()));
    DrawRectangleRounded(Rectangle {statX + 12.0f, 48.0f, statW - 24.0f, 12.0f}, 0.4f, 6, Color {35, 25, 25, 220});
    DrawRectangleRounded(Rectangle {statX + 12.0f, 48.0f, (statW - 24.0f) * ratio, 12.0f}, 0.4f, 6, Color {218, 74, 69, 255});
    DrawText("ALIVE", sx, 64, 13, Color {198, 218, 245, 255});
    DrawText(TextFormat("%d", CountLivingZombies()), sx + 54, 60, 22, Color {255, 240, 220, 255});
    DrawText("MODE", sx, 82, 13, Color {198, 218, 245, 255});
    DrawText(difficultyText, sx + 52, 78, 18, Color {223, 236, 255, 255});
    DrawText("BEST WAVE", sx, 100, 12, Color {198, 218, 245, 255});
    DrawText(TextFormat("%d", saveData_.bestWaveReached), sx + 92, 98, 18, Color {255, 235, 120, 255});

    constexpr float toolsX = 1176.0f;
    constexpr float toolsW = 104.0f;
    DrawRectangleRounded(Rectangle {toolsX, 14.0f, toolsW, 108.0f}, 0.12f, 6, Color {26, 34, 44, 220});
    DrawRectangleRoundedLinesEx(Rectangle {toolsX, 14.0f, toolsW, 108.0f}, 0.12f, 6, 1.0f, Color {100, 130, 170, 100});
    DrawText("TOOLS", static_cast<int>(toolsX + 34.0f), 18, 11, Color {160, 186, 220, 220});

    const Rectangle shovelRect = ShovelRect();
    const Rectangle pauseRect = PauseIconRect();
    const bool shovelHover = CheckCollisionPointRec(GetMousePosition(), shovelRect);
    const bool pauseHover = CheckCollisionPointRec(GetMousePosition(), pauseRect);
    DrawRectangleRounded(shovelRect, 0.18f, 5, shovelSelected_ ? Color {255, 212, 140, 255} : (shovelHover ? Color {225, 234, 239, 255} : Color {205, 216, 222, 255}));
    DrawRectangleRounded(pauseRect, 0.18f, 5, pauseHover ? Color {96, 181, 255, 255} : Color {65, 130, 232, 255});
    // Shovel glyph (blade + handle) — no letter that can be misread as a digit
    {
        const float bx = shovelRect.x + shovelRect.width * 0.5f;
        const float by = shovelRect.y + shovelRect.height * 0.5f;
        DrawRectangleRounded(Rectangle {bx - 10.0f, by - 10.0f, 14.0f, 12.0f}, 0.2f, 3, Color {165, 172, 180, 255});
        DrawRectangleRounded(Rectangle {bx - 2.0f, by + 0.0f, 5.0f, 12.0f}, 0.25f, 3, Color {110, 78, 48, 255});
    }
    DrawRectangle(static_cast<int>(pauseRect.x + 12.0f), static_cast<int>(pauseRect.y + 9.0f), 5, 16, WHITE);
    DrawRectangle(static_cast<int>(pauseRect.x + 23.0f), static_cast<int>(pauseRect.y + 9.0f), 5, 16, WHITE);
    DrawText("dig", static_cast<int>(shovelRect.x + 14.0f), static_cast<int>(shovelRect.y + shovelRect.height + 1.0f), 9, Color {188, 206, 230, 210});
    DrawText("pause", static_cast<int>(pauseRect.x + 2.0f), static_cast<int>(pauseRect.y + pauseRect.height + 1.0f), 9, Color {188, 206, 230, 210});

    if (waveManager_.InIntermission()) {
        DrawText(TextFormat("Next in %.1fs", waveManager_.Countdown()), sx, 128, 15, Color {255, 240, 192, 255});
    }
}

void Game::DrawSeedPackets() const {
    DrawRectangleRounded(Rectangle {194.0f, 12.0f, 600.0f, 94.0f}, 0.08f, 8, Color {248, 244, 232, 242});
    DrawRectangleRoundedLinesEx(Rectangle {194.0f, 12.0f, 600.0f, 94.0f}, 0.08f, 8, 2.5f, Color {120, 98, 62, 160});
    DrawRectangleRounded(Rectangle {198.0f, 16.0f, 592.0f, 86.0f}, 0.1f, 8, Color {0, 0, 0, 14});
    int index = 0;
    for (const auto type : AllPlantTypes()) {
        const PlantCard card = GetPlantCard(type);
        Rectangle rect = CardRect(index);

        const bool selected = selectedPlant_ == type;
        const bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
        const bool affordable = sunPoints_ >= card.cost;
        DrawRectangleRounded(Rectangle {rect.x, rect.y + 2.0f, rect.width, rect.height}, 0.2f, 5, Color {0, 0, 0, 32});
        DrawRectangleRounded(rect, 0.2f, 5, selected ? Color {255, 248, 205, 255} : Color {250, 247, 237, 255});
        DrawRectangleRounded(Rectangle {rect.x + 4.0f, rect.y + 4.0f, rect.width - 8.0f, 18.0f}, 0.2f, 5, card.color);
        DrawCardGlyph(type, Rectangle {rect.x + 7.0f, rect.y + 23.0f, 26.0f, 15.0f});
        DrawText(card.name.c_str(), static_cast<int>(rect.x + 37.0f), static_cast<int>(rect.y + 23.0f), 13, Color {36, 36, 36, 255});
        DrawRectangleRounded(Rectangle {rect.x + rect.width - 28.0f, rect.y + 3.0f, 24.0f, 17.0f}, 0.25f, 4, affordable ? Color {255, 233, 120, 255} : Color {195, 188, 170, 255});
        DrawText(TextFormat("%d", card.cost), static_cast<int>(rect.x + rect.width - 24.0f), static_cast<int>(rect.y + 4.0f), 12, affordable ? Color {65, 50, 9, 255} : Color {90, 90, 90, 255});
        if (selected) {
            DrawRectangleRoundedLinesEx(rect, 0.2f, 5, 2.5f, Color {255, 210, 80, 255});
        } else if (hovered) {
            DrawRectangleRoundedLinesEx(rect, 0.2f, 5, 2.0f, Color {128, 188, 255, 210});
        } else {
            DrawRectangleRoundedLinesEx(rect, 0.2f, 5, 1.5f, Color {0, 0, 0, 24});
        }

        const float cooldown = cooldowns_.at(PlantIndex(type));
        if (cooldown > 0.0f) {
            const float ratio = cooldown / card.cooldown;
            DrawRectangleRounded(Rectangle {rect.x, rect.y, rect.width, rect.height * ratio}, 0.2f, 3, Color {10, 14, 20, 145});
            DrawText(TextFormat("%.1f", cooldown), static_cast<int>(rect.x + 38.0f), static_cast<int>(rect.y + 12.0f), 20, WHITE);
        } else if (sunPoints_ < card.cost) {
            DrawRectangleRounded(rect, 0.2f, 5, Color {40, 40, 40, 55});
        }
        ++index;
    }
}

void Game::DrawPauseButtons() const {
    const Rectangle resume = PauseResumeRect();
    const Rectangle menu = PauseMenuRect();
    DrawUiButton(resume, "Resume", 30, ButtonColor(resume), Color {165, 221, 255, 160}, WHITE);
    DrawUiButton(menu, "Main Menu", 30, ButtonColor(menu), Color {165, 221, 255, 160}, WHITE);
}

void Game::HandleMenuInput() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 3; ++i) {
        if (CheckCollisionPointRec(mouse, DifficultyRect(i))) {
            selectedDifficulty_ = static_cast<Difficulty>(i);
            ApplyDifficultyProfile();
            return;
        }
    }

    if (CheckCollisionPointRec(mouse, MenuButtonRect(0))) {
        ApplyDifficultyProfile();
        ResetRun();
        state_ = GameState::Playing;
    } else if (CheckCollisionPointRec(mouse, MenuButtonRect(1))) {
        state_ = GameState::HowToPlay;
    } else if (CheckCollisionPointRec(mouse, MenuButtonRect(2))) {
        shouldClose_ = true;
    }
}

void Game::HandlePlayingInput() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, PauseIconRect())) {
        state_ = GameState::Paused;
        return;
    }

    if (CheckCollisionPointRec(mouse, ShovelRect())) {
        shovelSelected_ = !shovelSelected_;
        return;
    }

    if (TryCollectSun(mouse)) {
        if (audioReady_) {
            PlaySound(sfxCollect_);
        }
        return;
    }

    int idx = 0;
    for (const auto type : AllPlantTypes()) {
        const Rectangle card = CardRect(idx);
        if (CheckCollisionPointRec(mouse, card)) {
            selectedPlant_ = type;
            shovelSelected_ = false;
            return;
        }
        ++idx;
    }

    TryPlantAtMouse();
}

void Game::HandlePausedInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        state_ = GameState::Playing;
        return;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, PauseResumeRect())) {
        state_ = GameState::Playing;
    } else if (CheckCollisionPointRec(mouse, PauseMenuRect())) {
        state_ = GameState::Menu;
    }
}

void Game::HandleEndInput() {
    if (IsKeyPressed(KEY_R)) {
        ResetRun();
        state_ = GameState::Playing;
    }
    if (IsKeyPressed(KEY_M)) {
        state_ = GameState::Menu;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }
    const Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, EndRestartRect())) {
        ResetRun();
        state_ = GameState::Playing;
    } else if (CheckCollisionPointRec(mouse, EndMenuRect())) {
        state_ = GameState::Menu;
    }
}

void Game::TryPlantAtMouse() {
    const auto maybeCell = board_.CellFromPoint(GetMousePosition());
    if (!maybeCell.has_value()) {
        return;
    }

    const int row = maybeCell->first;
    const int col = maybeCell->second;
    if (shovelSelected_) {
        Plant* occupant = boardOccupancy_[static_cast<std::size_t>(row * config_.cols + col)];
        if (occupant != nullptr && occupant->IsAlive()) {
            occupant->Damage(99999.0f);
            AddScreenShake(2.2f, 0.10f);
        }
        return;
    }

    const PlacementFailReason failReason = CheckPlacement(row, col, selectedPlant_);
    if (failReason != PlacementFailReason::None) {
        placementMessageTimer_ = 1.2f;
        switch (failReason) {
            case PlacementFailReason::Occupied: placementMessage_ = "Tile occupied"; break;
            case PlacementFailReason::NotEnoughSun: placementMessage_ = "Not enough sun"; break;
            case PlacementFailReason::CoolingDown: placementMessage_ = "Plant is cooling down"; break;
            case PlacementFailReason::InvalidTile: placementMessage_ = "Cannot plant there"; break;
            default: break;
        }
        return;
    }

    const PlantCard card = GetPlantCard(selectedPlant_);

    plants_.push_back(CreatePlant(selectedPlant_, row, col, board_.CellCenter(row, col)));
    sunPoints_ -= card.cost;
    cooldowns_[PlantIndex(selectedPlant_)] = card.cooldown;
    PushPlacementPulse(board_.CellCenter(row, col), Color {140, 255, 140, 255});
    if (audioReady_) {
        PlaySound(sfxPlace_);
    }
    RebuildLaneViews();
}

bool Game::TryCollectSun(Vector2 mousePos) {
    for (auto& sun : suns_) {
        if (!sun.alive) {
            continue;
        }
        if (CheckCollisionPointCircle(mousePos, sun.position, 20.0f)) {
            sun.alive = false;
            sunPoints_ += sun.value;
            PushPlacementPulse(sun.position, sun.fromPlant ? Color {170, 255, 148, 255} : Color {255, 226, 122, 255});
            return true;
        }
    }
    return false;
}

Plant* Game::FindPlantBeingEaten(const Zombie& zombie, const std::vector<Plant*>& lanePlants) const {
    Plant* candidate = nullptr;
    float nearest = 9999.0f;
    for (Plant* plant : lanePlants) {
        if (!plant->IsAlive() || !plant->BlocksMovement()) {
            continue;
        }
        const Rectangle body = zombie.BodyRect();
        if (CheckCollisionRecs(body, plant->Rect())) {
            const float distance = std::fabs(plant->Center().x - zombie.X());
            if (distance < nearest) {
                nearest = distance;
                candidate = plant;
            }
        }
    }
    return candidate;
}

int Game::CountLivingZombies() const {
    int count = 0;
    for (const auto& zombie : zombies_) {
        if (zombie->IsAlive()) {
            ++count;
        }
    }
    return count;
}

void Game::AddScreenShake(float amplitude, float duration) {
    screenShakeAmp_ = std::max(screenShakeAmp_, amplitude);
    screenShakeTimer_ = std::max(screenShakeTimer_, duration);
}

void Game::RebuildLaneViews() {
    for (auto& lane : plantsByLane_) {
        lane.clear();
    }
    for (auto& lane : zombiesByLane_) {
        lane.clear();
    }
    std::fill(boardOccupancy_.begin(), boardOccupancy_.end(), nullptr);

    for (auto& plantPtr : plants_) {
        if (!plantPtr->IsAlive()) {
            continue;
        }
        const int lane = plantPtr->Row();
        if (lane >= 0 && lane < config_.lanes) {
            plantsByLane_[static_cast<std::size_t>(lane)].push_back(plantPtr.get());
            const int idx = lane * config_.cols + plantPtr->Col();
            if (idx >= 0 && idx < static_cast<int>(boardOccupancy_.size())) {
                boardOccupancy_[static_cast<std::size_t>(idx)] = plantPtr.get();
            }
        }
    }
    for (auto& zombiePtr : zombies_) {
        if (!zombiePtr->IsAlive()) {
            continue;
        }
        const int lane = zombiePtr->Row();
        if (lane >= 0 && lane < config_.lanes) {
            zombiesByLane_[static_cast<std::size_t>(lane)].push_back(zombiePtr.get());
        }
    }
}

PlacementFailReason Game::CheckPlacement(int row, int col, PlantType type) const {
    if (!board_.InBounds(row, col)) {
        return PlacementFailReason::InvalidTile;
    }
    const std::size_t idx = static_cast<std::size_t>(row * config_.cols + col);
    if (idx >= boardOccupancy_.size()) {
        return PlacementFailReason::InvalidTile;
    }
    if (boardOccupancy_[idx] != nullptr && boardOccupancy_[idx]->IsAlive()) {
        return PlacementFailReason::Occupied;
    }
    const PlantCard card = GetPlantCard(type);
    if (sunPoints_ < card.cost) {
        return PlacementFailReason::NotEnoughSun;
    }
    if (cooldowns_.at(PlantIndex(type)) > 0.0f) {
        return PlacementFailReason::CoolingDown;
    }
    return PlacementFailReason::None;
}

void Game::UpdateFloatingTexts(float dt) {
    for (auto& text : floatingTexts_) {
        text.ttl -= dt;
        text.position.y -= 20.0f * dt;
    }
    for (auto& pulse : placementPulses_) {
        pulse.ttl -= dt;
    }
    for (auto& spark : hitSparks_) {
        spark.ttl -= dt;
        spark.position.x += spark.velocity.x * dt;
        spark.position.y += spark.velocity.y * dt;
        spark.velocity.y += 220.0f * dt;
    }
    placementPulses_.erase(std::remove_if(placementPulses_.begin(), placementPulses_.end(), [](const UiPulse& pulse) { return pulse.ttl <= 0.0f; }), placementPulses_.end());
    hitSparks_.erase(std::remove_if(hitSparks_.begin(), hitSparks_.end(), [](const HitSpark& spark) { return spark.ttl <= 0.0f; }), hitSparks_.end());
}

void Game::DrawPlacementPreview() const {
    if (state_ != GameState::Playing || shovelSelected_) {
        return;
    }
    const auto maybeCell = board_.CellFromPoint(GetMousePosition());
    if (!maybeCell.has_value()) {
        return;
    }
    const int row = maybeCell->first;
    const int col = maybeCell->second;
    const PlacementFailReason reason = CheckPlacement(row, col, selectedPlant_);
    const Rectangle cell = board_.CellRect(row, col);
    DrawRectangleRounded(cell, 0.1f, 5, PlacementColor(reason));
    DrawRectangleRoundedLinesEx(cell, 0.1f, 5, 2.0f, reason == PlacementFailReason::None ? Color {146, 255, 146, 255} : Color {255, 132, 132, 255});
    if (reason == PlacementFailReason::Occupied) {
        DrawLineEx(Vector2 {cell.x + 12.0f, cell.y + 12.0f}, Vector2 {cell.x + cell.width - 12.0f, cell.y + cell.height - 12.0f}, 3.0f, Color {255, 180, 180, 200});
        DrawLineEx(Vector2 {cell.x + cell.width - 12.0f, cell.y + 12.0f}, Vector2 {cell.x + 12.0f, cell.y + cell.height - 12.0f}, 3.0f, Color {255, 180, 180, 200});
    }
}

void Game::DrawToolTip() const {
    int idx = 0;
    for (const auto type : AllPlantTypes()) {
        const Rectangle cardRect = CardRect(idx);
        if (CheckCollisionPointRec(GetMousePosition(), cardRect)) {
            const PlantCard card = GetPlantCard(type);
            float x = cardRect.x + cardRect.width + 10.0f;
            if (x + 268.0f > config_.screenWidth - 12.0f) {
                x = cardRect.x - 278.0f;
            }
            x = std::max(12.0f, x);
            float y = cardRect.y + cardRect.height + 8.0f;
            if (y + 116.0f > config_.screenHeight - 12.0f) {
                y = cardRect.y - 124.0f;
            }
            y = std::max(116.0f, y);
            const Rectangle tip {x, y, 268.0f, 116.0f};
            DrawRectangleRounded(Rectangle {tip.x + 4.0f, tip.y + 5.0f, tip.width, tip.height}, 0.12f, 6, Color {0, 0, 0, 60});
            DrawRectangleRounded(tip, 0.12f, 6, Color {22, 30, 34, 235});
            DrawRectangleRounded(Rectangle {tip.x, tip.y, tip.width, 26.0f}, 0.12f, 6, Color {card.color.r, card.color.g, card.color.b, 255});
            DrawText(card.name.c_str(), static_cast<int>(tip.x + 10.0f), static_cast<int>(tip.y + 4.0f), 18, WHITE);
            DrawText(TextFormat("Cost %d", card.cost), static_cast<int>(tip.x + 10.0f), static_cast<int>(tip.y + 34.0f), 16, Color {255, 232, 170, 255});
            DrawText(TextFormat("Cooldown %.1fs", card.cooldown), static_cast<int>(tip.x + 90.0f), static_cast<int>(tip.y + 34.0f), 16, WHITE);
            DrawText(TextFormat("Health %d", card.health), static_cast<int>(tip.x + 190.0f), static_cast<int>(tip.y + 34.0f), 16, WHITE);
            DrawText(card.role.c_str(), static_cast<int>(tip.x + 10.0f), static_cast<int>(tip.y + 58.0f), 16, SKYBLUE);
            DrawText(card.effectSummary.c_str(), static_cast<int>(tip.x + 10.0f), static_cast<int>(tip.y + 82.0f), 16, Color {211, 233, 200, 255});
            break;
        }
        idx++;
    }
}

void Game::PushFloatingText(Vector2 pos, const std::string& text, Color color) {
    floatingTexts_.push_back(FloatingText {pos, 0.65f, color, text});
}

void Game::PushPlacementPulse(Vector2 pos, Color color) {
    placementPulses_.push_back(UiPulse {pos, 0.42f, color});
}

void Game::PushHitSpark(Vector2 pos, Color color, bool armorBreak) {
    const int count = armorBreak ? 8 : 4;
    std::uniform_real_distribution<float> xDist(-90.0f, 90.0f);
    std::uniform_real_distribution<float> yDist(armorBreak ? -220.0f : -150.0f, armorBreak ? -70.0f : -40.0f);
    for (int i = 0; i < count; ++i) {
        const float vx = xDist(visualRng_);
        const float vy = yDist(visualRng_);
        hitSparks_.push_back(HitSpark {pos, Vector2 {vx, vy}, armorBreak ? 0.34f : 0.22f, color, armorBreak});
    }
}

void Game::FinalizeRun(bool won) {
    if (runEnded_) {
        state_ = won ? GameState::Win : GameState::Lose;
        return;
    }
    runEnded_ = true;
    saveData_.gamesPlayed += 1;

    const int reached = won ? waveManager_.TotalWaves() : waveManager_.CurrentWave();
    saveData_.bestWaveReached = std::max(saveData_.bestWaveReached, reached);
    if (won) {
        saveData_.totalWins += 1;
    }
    saveSystem_.Save(saveData_);
    state_ = won ? GameState::Win : GameState::Lose;
}
