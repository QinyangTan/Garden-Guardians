#include "Plant.hpp"

#include <algorithm>
#include <cmath>

#include "Zombie.hpp"

namespace {
bool HasLaneTarget(const std::vector<Zombie*>& laneZombies, float minX) {
    for (const Zombie* zombie : laneZombies) {
        if (zombie->IsAlive() && zombie->X() > minX) {
            return true;
        }
    }
    return false;
}

std::vector<Zombie*>& LaneGroup(std::vector<std::vector<Zombie*>>& zombiesByLane, int row) {
    return zombiesByLane[static_cast<std::size_t>(row)];
}
}  // namespace

Plant::Plant(PlantType type, int row, int col, Vector2 center, float health, Color color)
    : type_(type), row_(row), col_(col), center_(center), health_(health), maxHealth_(health), color_(color) {}

void Plant::Draw() const {
    const Rectangle r = Rect();
    DrawEllipse(static_cast<int>(center_.x), static_cast<int>(center_.y + 26.0f), 24.0f, 8.0f, Color {0, 0, 0, 45});
    DrawRectangleRounded(r, 0.35f, 8, color_);
    DrawRectangleRounded(Rectangle {r.x + 6.0f, r.y + 6.0f, r.width - 12.0f, r.height * 0.28f}, 0.25f, 6, Color {255, 255, 255, 22});
    if (hitFlash_ > 0.0f) {
        const unsigned char alpha = static_cast<unsigned char>(std::min(160.0f, 220.0f * hitFlash_));
        DrawRectangleRounded(r, 0.35f, 8, Color {255, 255, 255, alpha});
    }
    const float hpRatio = std::max(0.0f, health_) / std::max(1.0f, maxHealth_);
    DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y - 8.0f), static_cast<int>(r.width), 5, Color {45, 20, 20, 180});
    DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y - 8.0f), static_cast<int>(r.width * hpRatio), 5, Color {90, 230, 90, 255});
}

void Plant::Damage(float amount) {
    health_ -= amount;
    hitFlash_ = 0.18f;
}

void Plant::TickVisual(float dt) {
    hitFlash_ = std::max(0.0f, hitFlash_ - dt);
}

bool Plant::IsAlive() const { return health_ > 0.0f; }
int Plant::Row() const { return row_; }
int Plant::Col() const { return col_; }
PlantType Plant::Type() const { return type_; }
Vector2 Plant::Center() const { return center_; }
Rectangle Plant::Rect() const { return Rectangle {center_.x - 30.0f, center_.y - 32.0f, 60.0f, 64.0f}; }
bool Plant::BlocksMovement() const { return true; }

PeashooterPlant::PeashooterPlant(int row, int col, Vector2 center)
    : Plant(PlantType::Peashooter, row, col, center, 220.0f, Color {76, 184, 74, 255}) {}

void PeashooterPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    fireTimer_ -= dt;
    if (fireTimer_ <= 0.0f && HasLaneTarget(laneZombies, center_.x)) {
        projectiles.push_back(Projectile {row_, center_.x + 24.0f, center_.y - 8.0f, 300.0f, 26.0f, false, 1.0f, 0.0f, false, true});
        fireTimer_ = 1.2f;
    }
}
std::string PeashooterPlant::Description() const { return "Lane ranged attacker"; }

SunBloomPlant::SunBloomPlant(int row, int col, Vector2 center)
    : Plant(PlantType::SunBloom, row, col, center, 190.0f, Color {252, 208, 57, 255}) {}

void SunBloomPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>&, std::vector<Projectile>&, std::vector<SunToken>& suns) {
    bloomPulse_ = std::max(0.0f, bloomPulse_ - dt * 1.8f);
    sunTimer_ -= dt;
    if (sunTimer_ <= 0.0f) {
        for (int i = 0; i < 2; ++i) {
            SunToken token;
            token.position = Vector2 {center_.x + static_cast<float>(GetRandomValue(-6, 6)), center_.y - 12.0f};
            token.velocity.x = static_cast<float>(GetRandomValue(65, 135));
            token.velocity.y = static_cast<float>(-GetRandomValue(220, 300));
            token.ttl = 7.0f;
            token.jumpTimer = 0.55f;
            token.value = 30;
            token.fromPlant = true;
            suns.push_back(token);
        }
        bloomPulse_ = 1.0f;
        sunTimer_ = 7.4f;
    }
}

void SunBloomPlant::Draw() const {
    Plant::Draw();
    if (bloomPulse_ > 0.0f) {
        const float p = 1.0f - bloomPulse_;
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 10.0f), 18.0f + p * 20.0f, Color {255, 228, 122, static_cast<unsigned char>(220.0f * bloomPulse_)});
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 10.0f), 28.0f + p * 24.0f, Color {255, 243, 168, static_cast<unsigned char>(150.0f * bloomPulse_)});
    }
}
std::string SunBloomPlant::Description() const { return "Creates sunlight income"; }

WallRootPlant::WallRootPlant(int row, int col, Vector2 center)
    : Plant(PlantType::WallRoot, row, col, center, 1200.0f, Color {122, 91, 63, 255}) {}
void WallRootPlant::Update(float, const Board&, std::vector<std::vector<Zombie*>>&, std::vector<Projectile>&, std::vector<SunToken>&) {}
std::string WallRootPlant::Description() const { return "Durable frontline blocker"; }

ChomperPlant::ChomperPlant(int row, int col, Vector2 center)
    : Plant(PlantType::Chomper, row, col, center, 260.0f, Color {191, 88, 204, 255}) {}

void ChomperPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>&, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    if (digestTimer_ > 0.0f) {
        digestTimer_ -= dt;
        return;
    }
    Zombie* target = nullptr;
    float nearest = 9999.0f;
    for (Zombie* zombie : laneZombies) {
        if (!zombie->IsAlive()) {
            continue;
        }
        const float d = std::fabs(zombie->X() - center_.x);
        if (d < 76.0f && d < nearest) {
            nearest = d;
            target = zombie;
        }
    }
    if (target != nullptr) {
        target->Kill();
        digestTimer_ = digestDuration_;
    }
}

void ChomperPlant::Draw() const {
    Plant::Draw();
    const float t = static_cast<float>(GetTime());
    const bool digesting = digestTimer_ > 0.0f;
    const float mouthOpen = digesting ? 0.24f : (0.55f + 0.20f * std::sin(t * 7.0f));
    const float radius = 20.0f + (digesting ? 3.0f * std::sin(t * 8.0f) : 0.0f);
    DrawCircleV(Vector2 {center_.x, center_.y - 6.0f}, radius, digesting ? Color {132, 62, 150, 255} : Color {183, 92, 212, 255});
    DrawTriangle(Vector2 {center_.x - 4.0f, center_.y - 6.0f}, Vector2 {center_.x + 24.0f, center_.y - 6.0f - 18.0f * mouthOpen}, Vector2 {center_.x + 26.0f, center_.y - 4.0f}, Color {245, 210, 247, 255});
    DrawTriangle(Vector2 {center_.x - 4.0f, center_.y - 6.0f}, Vector2 {center_.x + 24.0f, center_.y - 6.0f + 18.0f * mouthOpen}, Vector2 {center_.x + 26.0f, center_.y - 8.0f}, Color {245, 210, 247, 255});
    if (digesting) {
        const float ratio = digestTimer_ / digestDuration_;
        DrawText("DIGEST", static_cast<int>(center_.x - 30.0f), static_cast<int>(center_.y + 28.0f), 14, Color {255, 225, 155, 255});
        DrawRectangle(static_cast<int>(center_.x - 30.0f), static_cast<int>(center_.y + 44.0f), 60, 6, Color {40, 20, 20, 180});
        DrawRectangle(static_cast<int>(center_.x - 30.0f), static_cast<int>(center_.y + 44.0f), static_cast<int>(60.0f * ratio), 6, Color {255, 164, 76, 255});
    }
}
std::string ChomperPlant::Description() const { return "One-shot swallow, then digests"; }

FrostBurstPlant::FrostBurstPlant(int row, int col, Vector2 center)
    : Plant(PlantType::FrostBurst, row, col, center, 140.0f, Color {90, 199, 252, 255}) {}
void FrostBurstPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>&, std::vector<SunToken>&) {
    if (exploded_) {
        burstTimer_ -= dt;
        if (burstTimer_ <= 0.0f) {
            health_ = 0.0f;
        }
        return;
    }
    armTimer_ -= dt;
    if (armTimer_ > 0.0f) {
        return;
    }
    bool trigger = false;
    for (auto& lane : zombiesByLane) {
        for (Zombie* zombie : lane) {
            if (zombie->IsAlive()) {
                trigger = true;
                break;
            }
        }
        if (trigger) {
            trigger = true;
            break;
        }
    }
    if (!trigger) {
        return;
    }
    if (!damageApplied_) {
        for (auto& lane : zombiesByLane) {
            for (Zombie* zombie : lane) {
                if (zombie->IsAlive()) {
                    zombie->ReceiveHit(115.0f, true, 0.52f, 4.5f);
                }
            }
        }
        damageApplied_ = true;
    }
    exploded_ = true;
    burstTimer_ = 0.48f;
}
void FrostBurstPlant::Draw() const {
    Plant::Draw();
    if (!exploded_) {
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y), 34.0f, SKYBLUE);
    } else {
        const float ratio = std::max(0.0f, burstTimer_ / 0.48f);
        const float radius = 40.0f + (1.0f - ratio) * 600.0f;
        const unsigned char alpha = static_cast<unsigned char>(180.0f * ratio);
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y), radius, Color {180, 240, 255, alpha});
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y), radius * 0.72f, Color {120, 210, 255, alpha});
    }
}
std::string FrostBurstPlant::Description() const { return "Full-screen frost burst"; }

RepeaterPlant::RepeaterPlant(int row, int col, Vector2 center)
    : Plant(PlantType::Repeater, row, col, center, 240.0f, Color {44, 143, 58, 255}) {}
void RepeaterPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    recoil_ = std::max(0.0f, recoil_ - dt * 2.5f);
    fireTimer_ -= dt;
    if (fireTimer_ <= 0.0f && HasLaneTarget(laneZombies, center_.x)) {
        projectiles.push_back(Projectile {row_, center_.x + 24.0f, center_.y - 14.0f, 315.0f, 18.0f, false, 1.0f, 0.0f, false, true});
        projectiles.push_back(Projectile {row_, center_.x + 24.0f, center_.y - 4.0f, 315.0f, 18.0f, false, 1.0f, 0.0f, false, true});
        fireTimer_ = 1.25f;
        recoil_ = 1.0f;
    }
}
void RepeaterPlant::Draw() const {
    Plant::Draw();
    const float recoilX = 4.0f * recoil_;
    DrawCircleV(Vector2 {center_.x - recoilX - 2.0f, center_.y - 14.0f}, 13.0f, Color {62, 162, 72, 255});
    DrawCircleV(Vector2 {center_.x - recoilX + 10.0f, center_.y - 4.0f}, 13.0f, Color {62, 162, 72, 255});
    DrawCircleV(Vector2 {center_.x - recoilX + 8.0f, center_.y - 14.0f}, 5.0f, Color {46, 113, 45, 255});
    DrawCircleV(Vector2 {center_.x - recoilX + 20.0f, center_.y - 4.0f}, 5.0f, Color {46, 113, 45, 255});
}
std::string RepeaterPlant::Description() const { return "Shoots two peas per attack"; }

IcePeaPlant::IcePeaPlant(int row, int col, Vector2 center)
    : Plant(PlantType::IcePea, row, col, center, 220.0f, Color {116, 211, 248, 255}) {}
void IcePeaPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    chillPulse_ = std::max(0.0f, chillPulse_ - dt * 1.9f);
    fireTimer_ -= dt;
    if (fireTimer_ <= 0.0f && HasLaneTarget(laneZombies, center_.x)) {
        Projectile p {row_, center_.x + 24.0f, center_.y - 8.0f, 290.0f, 20.0f, true, 0.65f, 2.5f, false, true};
        projectiles.push_back(p);
        fireTimer_ = 1.35f;
        chillPulse_ = 1.0f;
    }
}
void IcePeaPlant::Draw() const {
    Plant::Draw();
    if (chillPulse_ > 0.0f) {
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), 20.0f + (1.0f - chillPulse_) * 14.0f, Color {170, 228, 255, static_cast<unsigned char>(220.0f * chillPulse_)});
    }
}
std::string IcePeaPlant::Description() const { return "Ranged shot that slows zombies"; }

SpikeTrapPlant::SpikeTrapPlant(int row, int col, Vector2 center)
    : Plant(PlantType::SpikeTrap, row, col, center, 500.0f, Color {126, 104, 88, 255}) {}
void SpikeTrapPlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>&, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    flash_ = std::max(0.0f, flash_ - dt * 2.4f);
    tickTimer_ -= dt;
    if (tickTimer_ > 0.0f) {
        return;
    }
    bool hit = false;
    for (Zombie* zombie : laneZombies) {
        if (!zombie->IsAlive()) {
            continue;
        }
        if (std::fabs(zombie->X() - center_.x) < 42.0f) {
            zombie->ReceiveHit(24.0f, false, 1.0f, 0.0f);
            hit = true;
        }
    }
    if (hit) {
        flash_ = 1.0f;
    }
    tickTimer_ = 0.5f;
}
void SpikeTrapPlant::Draw() const {
    const Rectangle base {center_.x - 30.0f, center_.y + 12.0f, 60.0f, 16.0f};
    DrawRectangleRounded(base, 0.3f, 5, Color {94, 74, 63, 255});
    for (int i = 0; i < 6; ++i) {
        const float x = base.x + 6.0f + static_cast<float>(i) * 9.0f;
        DrawTriangle(Vector2 {x, base.y}, Vector2 {x + 5.0f, base.y}, Vector2 {x + 2.5f, base.y - 10.0f}, flash_ > 0.0f ? ORANGE : LIGHTGRAY);
    }
    Plant::Draw();
}
std::string SpikeTrapPlant::Description() const { return "Contact damage ground trap"; }
bool SpikeTrapPlant::BlocksMovement() const { return false; }

BombBerryPlant::BombBerryPlant(int row, int col, Vector2 center)
    : Plant(PlantType::BombBerry, row, col, center, 130.0f, Color {230, 102, 89, 255}) {}
void BombBerryPlant::Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>&, std::vector<SunToken>&) {
    warningPulse_ += dt;
    if (exploded_) {
        burstTimer_ -= dt;
        if (burstTimer_ <= 0.0f) {
            health_ = 0.0f;
        }
        return;
    }
    armTimer_ -= dt;
    if (armTimer_ > 0.0f) {
        return;
    }
    bool shouldBlow = false;
    for (auto& lane : zombiesByLane) {
        for (Zombie* zombie : lane) {
            if (!zombie->IsAlive()) {
                continue;
            }
            const float dx = (zombie->X() - center_.x) / board.Config().cellWidth;
            const float zy = zombie->BodyRect().y + zombie->BodyRect().height * 0.5f;
            const float dy = (zy - center_.y) / board.Config().cellHeight;
            if ((dx * dx + dy * dy) <= (1.45f * 1.45f)) {
                shouldBlow = true;
                break;
            }
        }
        if (shouldBlow) {
            break;
        }
    }
    if (!shouldBlow) {
        return;
    }
    if (!damageApplied_) {
        for (auto& lane : zombiesByLane) {
            for (Zombie* zombie : lane) {
                if (!zombie->IsAlive()) {
                    continue;
                }
                const float dx = (zombie->X() - center_.x) / board.Config().cellWidth;
                const float zy = zombie->BodyRect().y + zombie->BodyRect().height * 0.5f;
                const float dy = (zy - center_.y) / board.Config().cellHeight;
                if ((dx * dx + dy * dy) <= (1.45f * 1.45f)) {
                    zombie->ReceiveHit(240.0f, false, 1.0f, 0.0f);
                }
            }
        }
        damageApplied_ = true;
    }
    exploded_ = true;
    burstTimer_ = 0.36f;
}
void BombBerryPlant::Draw() const {
    Plant::Draw();
    if (!exploded_) {
        const bool blink = (static_cast<int>(warningPulse_ * 6.0f) % 2) == 0;
        DrawCircleV(Vector2 {center_.x, center_.y - 8.0f}, 9.0f, blink ? YELLOW : ORANGE);
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), 28.0f, blink ? Color {255, 160, 50, 255} : Color {255, 80, 70, 220});
    } else {
        const float ratio = std::max(0.0f, burstTimer_ / 0.36f);
        const float radius = 18.0f + (1.0f - ratio) * 120.0f;
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), radius, Color {255, 180, 80, static_cast<unsigned char>(220.0f * ratio)});
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), radius * 0.65f, Color {255, 90, 60, static_cast<unsigned char>(180.0f * ratio)});
    }
}
std::string BombBerryPlant::Description() const { return "Nearby-tile explosive burst"; }

MagnetVinePlant::MagnetVinePlant(int row, int col, Vector2 center)
    : Plant(PlantType::MagnetVine, row, col, center, 180.0f, Color {151, 104, 220, 255}) {}
void MagnetVinePlant::Update(float dt, const Board&, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>&, std::vector<SunToken>&) {
    auto& laneZombies = LaneGroup(zombiesByLane, row_);
    pulseFx_ = std::max(0.0f, pulseFx_ - dt * 1.5f);
    pulseTimer_ -= dt;
    if (pulseTimer_ > 0.0f) {
        return;
    }
    Zombie* target = nullptr;
    float nearX = 99999.0f;
    for (Zombie* zombie : laneZombies) {
        if (!zombie->IsAlive() || !zombie->HasArmor()) {
            continue;
        }
        const float dx = std::fabs(zombie->X() - center_.x);
        if (dx < nearX) {
            nearX = dx;
            target = zombie;
        }
    }
    if (target != nullptr) {
        target->RemoveArmor(100.0f);
        pulseFx_ = 1.0f;
    }
    pulseTimer_ = 7.0f;
}
void MagnetVinePlant::Draw() const {
    Plant::Draw();
    DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), 12.0f, Color {214, 182, 255, 255});
    if (pulseFx_ > 0.0f) {
        DrawCircleLines(static_cast<int>(center_.x), static_cast<int>(center_.y - 8.0f), 20.0f + (1.0f - pulseFx_) * 25.0f, Color {206, 146, 255, static_cast<unsigned char>(200.0f * pulseFx_)});
    }
}
std::string MagnetVinePlant::Description() const { return "Strips armor from lane enemies"; }

PlantCard GetPlantCard(PlantType type) {
    switch (type) {
        case PlantType::Peashooter: return {type, "Peashooter", "Basic lane DPS", "26 damage every 1.2s", 100, 3.5f, 220, Color {76, 184, 74, 255}};
        case PlantType::SunBloom: return {type, "SunBloom", "Economy", "Creates 2x sunlight, 7s decay", 75, 4.5f, 190, Color {252, 208, 57, 255}};
        case PlantType::WallRoot: return {type, "WallRoot", "Tank wall", "Very high health blocker", 100, 8.0f, 1200, Color {122, 91, 63, 255}};
        case PlantType::Chomper: return {type, "Chomper", "Single-target burst", "One-shot then 7s digest", 150, 7.0f, 260, Color {191, 88, 204, 255}};
        case PlantType::FrostBurst: return {type, "FrostBurst", "Global control", "Hits all zombies, slows 4.5s", 225, 18.0f, 140, Color {90, 199, 252, 255}};
        case PlantType::Repeater: return {type, "Repeater", "High lane DPS", "2 peas x18 every 1.25s", 175, 6.0f, 240, Color {44, 143, 58, 255}};
        case PlantType::IcePea: return {type, "Ice Pea", "Slow support", "20 damage + 0.65 slow", 150, 5.0f, 220, Color {116, 211, 248, 255}};
        case PlantType::SpikeTrap: return {type, "SpikeTrap", "Contact hazard", "24 damage each 0.5s", 100, 6.0f, 500, Color {126, 104, 88, 255}};
        case PlantType::BombBerry: return {type, "BombBerry", "Burst clear", "Nearby tile blast, 240 damage", 175, 14.0f, 130, Color {230, 102, 89, 255}};
        case PlantType::MagnetVine: return {type, "Magnet Vine", "Armor utility", "Removes 100 armor / 7s", 125, 10.0f, 180, Color {151, 104, 220, 255}};
        default: return {PlantType::Peashooter, "Peashooter", "Basic lane DPS", "26 damage every 1.2s", 100, 3.5f, 220, Color {76, 184, 74, 255}};
    }
}

std::unique_ptr<Plant> CreatePlant(PlantType type, int row, int col, Vector2 center) {
    switch (type) {
        case PlantType::Peashooter: return std::make_unique<PeashooterPlant>(row, col, center);
        case PlantType::SunBloom: return std::make_unique<SunBloomPlant>(row, col, center);
        case PlantType::WallRoot: return std::make_unique<WallRootPlant>(row, col, center);
        case PlantType::Chomper: return std::make_unique<ChomperPlant>(row, col, center);
        case PlantType::FrostBurst: return std::make_unique<FrostBurstPlant>(row, col, center);
        case PlantType::Repeater: return std::make_unique<RepeaterPlant>(row, col, center);
        case PlantType::IcePea: return std::make_unique<IcePeaPlant>(row, col, center);
        case PlantType::SpikeTrap: return std::make_unique<SpikeTrapPlant>(row, col, center);
        case PlantType::BombBerry: return std::make_unique<BombBerryPlant>(row, col, center);
        case PlantType::MagnetVine: return std::make_unique<MagnetVinePlant>(row, col, center);
        default: return std::make_unique<PeashooterPlant>(row, col, center);
    }
}
