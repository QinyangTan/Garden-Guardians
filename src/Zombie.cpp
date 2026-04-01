#include "Zombie.hpp"

#include <algorithm>

Zombie::Zombie(ZombieType type, int row, float x, float y, float health, float armor, float speed, float dps, Color color, bool slowResistant)
    : type_(type),
      row_(row),
      x_(x),
      y_(y),
      health_(health),
      maxHealth_(health),
      armor_(armor),
      maxArmor_(armor),
      baseSpeed_(speed),
      dps_(dps),
      slowResistant_(slowResistant),
      color_(color) {}

void Zombie::Update(float dt) {
    hitFlash_ = std::max(0.0f, hitFlash_ - dt);
    if (slowTimer_ > 0.0f) {
        slowTimer_ -= dt;
        if (slowTimer_ <= 0.0f) {
            slowFactor_ = 1.0f;
            slowTimer_ = 0.0f;
        }
    }
}

void Zombie::Draw() const {
    const Rectangle body = BodyRect();
    DrawEllipse(static_cast<int>(x_), static_cast<int>(body.y + body.height + 6.0f), 23.0f, 8.0f, Color {0, 0, 0, 50});
    const Color drawColor = casting_ ? Color {static_cast<unsigned char>(std::min(255, color_.r + 30)), color_.g, color_.b, 255} : color_;
    DrawRectangleRounded(body, 0.2f, 6, drawColor);
    if (hitFlash_ > 0.0f) {
        const unsigned char alpha = static_cast<unsigned char>(std::min(170.0f, 230.0f * hitFlash_));
        DrawRectangleRounded(body, 0.2f, 6, Color {255, 255, 255, alpha});
    }
    DrawRectangle(static_cast<int>(body.x + body.width * 0.22f), static_cast<int>(body.y + 9.0f), 12, 12, WHITE);
    DrawRectangle(static_cast<int>(body.x + body.width * 0.62f), static_cast<int>(body.y + 9.0f), 12, 12, WHITE);
    DrawCircle(static_cast<int>(body.x + body.width * 0.28f), static_cast<int>(body.y + 15.0f), 3, BLACK);
    DrawCircle(static_cast<int>(body.x + body.width * 0.68f), static_cast<int>(body.y + 15.0f), 3, BLACK);
    if (type_ == ZombieType::Conehead) {
        DrawTriangle(Vector2 {body.x + 8.0f, body.y - 6.0f}, Vector2 {body.x + body.width - 8.0f, body.y - 6.0f}, Vector2 {body.x + body.width * 0.5f, body.y - 30.0f}, ORANGE);
    } else if (type_ == ZombieType::Spitter) {
        DrawCircle(static_cast<int>(body.x + body.width * 0.84f), static_cast<int>(body.y + 28.0f), 7.0f, Color {92, 235, 124, 255});
    } else if (type_ == ZombieType::Summoner) {
        DrawCircleLines(static_cast<int>(body.x + body.width * 0.5f), static_cast<int>(body.y + 28.0f), 18.0f, Color {190, 128, 255, 210});
    } else if (type_ == ZombieType::PoleJump && !jumpUsed_) {
        DrawRectangle(static_cast<int>(body.x + body.width + 3.0f), static_cast<int>(body.y - 4.0f), 4, 38, BROWN);
    } else if (type_ == ZombieType::Imp) {
        DrawRectangle(static_cast<int>(body.x + 8.0f), static_cast<int>(body.y + body.height - 8.0f), 10, 6, Color {240, 180, 180, 255});
    }

    const float hpRatio = std::max(0.0f, health_) / std::max(1.0f, maxHealth_);
    const Rectangle hpBack {body.x, body.y - 10.0f, body.width, 6.0f};
    const Rectangle hpFill {body.x, body.y - 10.0f, body.width * hpRatio, 6.0f};
    DrawRectangleRec(hpBack, Color {40, 20, 20, 180});
    DrawRectangleRec(hpFill, Color {120, 220, 90, 255});

    if (maxArmor_ > 0.0f) {
        const float armorRatio = std::max(0.0f, armor_) / std::max(1.0f, maxArmor_);
        DrawRectangle(static_cast<int>(body.x), static_cast<int>(body.y - 17.0f), static_cast<int>(body.width), 5, Color {26, 36, 60, 190});
        DrawRectangle(static_cast<int>(body.x), static_cast<int>(body.y - 17.0f), static_cast<int>(body.width * armorRatio), 5, SKYBLUE);
        if (armor_ > 0.0f && armorRatio < 0.35f) {
            DrawLineEx(Vector2 {body.x + 12.0f, body.y - 6.0f}, Vector2 {body.x + 22.0f, body.y + 6.0f}, 2.0f, Color {255, 186, 120, 240});
        }
    }
    if (IsSlowed()) {
        DrawCircle(static_cast<int>(body.x + body.width + 6.0f), static_cast<int>(body.y + 8.0f), 5.0f, SKYBLUE);
    }
}

void Zombie::ReceiveHit(float damage, bool slow, float slowFactor, float slowDuration) {
    if (armor_ > 0.0f) {
        armor_ -= damage;
        if (armor_ < 0.0f) {
            health_ += armor_;
            armor_ = 0.0f;
        }
    } else {
        health_ -= damage;
    }
    hitFlash_ = 0.16f;
    if (slow && !slowResistant_) {
        slowFactor_ = std::min(slowFactor_, slowFactor);
        slowTimer_ = std::max(slowTimer_, slowDuration);
    }
}

bool Zombie::IsAlive() const {
    return health_ > 0.0f;
}

float Zombie::X() const {
    return x_;
}

int Zombie::Row() const {
    return row_;
}

float Zombie::Dps() const {
    return dps_;
}

float Zombie::Health() const {
    return health_;
}

float Zombie::MaxHealth() const {
    return maxHealth_;
}

float Zombie::Armor() const { return armor_; }
float Zombie::MaxArmor() const { return maxArmor_; }
bool Zombie::HasArmor() const { return armor_ > 0.0f; }

ZombieType Zombie::Type() const {
    return type_;
}

Rectangle Zombie::BodyRect() const {
    return Rectangle {x_ - 26.0f, y_ - 30.0f, 52.0f, 70.0f};
}

bool Zombie::IsStopped() const {
    return stopped_;
}

bool Zombie::IsSlowed() const {
    return slowFactor_ < 0.99f && slowTimer_ > 0.0f;
}

bool Zombie::CanUseJump() const {
    return type_ == ZombieType::PoleJump && !jumpUsed_;
}

bool Zombie::HasJumped() const {
    return jumpUsed_;
}

float Zombie::SpecialTimer() const {
    return specialTimer_;
}

bool Zombie::IsCasting() const {
    return casting_;
}

void Zombie::SetX(float newX) {
    x_ = newX;
}

void Zombie::SetY(float newY) { y_ = newY; }

void Zombie::SetStopped(bool stopped) {
    stopped_ = stopped;
}

void Zombie::Move(float dt) {
    if (!stopped_) {
        x_ -= baseSpeed_ * slowFactor_ * dt;
    }
}

void Zombie::Kill() {
    health_ = 0.0f;
}

void Zombie::ApplyMultipliers(float healthScale, float speedScale, float damageScale) {
    const float clampedHealth = std::max(0.5f, healthScale);
    const float clampedSpeed = std::max(0.4f, speedScale);
    const float clampedDamage = std::max(0.5f, damageScale);
    maxHealth_ *= clampedHealth;
    health_ *= clampedHealth;
    maxArmor_ *= clampedHealth;
    armor_ *= clampedHealth;
    baseSpeed_ *= clampedSpeed;
    dps_ *= clampedDamage;
}

void Zombie::RemoveArmor(float amount) {
    armor_ = std::max(0.0f, armor_ - amount);
    hitFlash_ = std::max(hitFlash_, 0.2f);
}

void Zombie::MarkJumpUsed() {
    jumpUsed_ = true;
}

void Zombie::SetBaseSpeed(float speed) {
    baseSpeed_ = speed;
}

void Zombie::SetSpecialTimer(float time) {
    specialTimer_ = time;
}

void Zombie::SetCasting(bool casting) {
    casting_ = casting;
}

void Zombie::TickSpecial(float dt) {
    specialTimer_ = std::max(0.0f, specialTimer_ - dt);
}

std::unique_ptr<Zombie> CreateZombie(ZombieType type, int row, float spawnX, float laneY) {
    switch (type) {
        case ZombieType::Basic: return std::make_unique<Zombie>(type, row, spawnX, laneY, 185.0f, 0.0f, 17.0f, 20.0f, Color {116, 145, 109, 255});
        case ZombieType::Runner: return std::make_unique<Zombie>(type, row, spawnX, laneY, 110.0f, 0.0f, 31.0f, 16.0f, Color {225, 135, 88, 255});
        case ZombieType::Tank: return std::make_unique<Zombie>(type, row, spawnX, laneY, 360.0f, 0.0f, 11.0f, 30.0f, Color {104, 88, 146, 255});
        case ZombieType::Bucket: return std::make_unique<Zombie>(type, row, spawnX, laneY, 220.0f, 90.0f, 15.0f, 24.0f, Color {140, 140, 160, 255}, true);
        case ZombieType::Conehead: return std::make_unique<Zombie>(type, row, spawnX, laneY, 210.0f, 120.0f, 16.0f, 22.0f, Color {188, 136, 74, 255});
        case ZombieType::Imp: return std::make_unique<Zombie>(type, row, spawnX, laneY, 80.0f, 0.0f, 38.0f, 14.0f, Color {233, 93, 110, 255});
        case ZombieType::Spitter: return std::make_unique<Zombie>(type, row, spawnX, laneY, 160.0f, 0.0f, 13.0f, 10.0f, Color {84, 188, 122, 255});
        case ZombieType::PoleJump: return std::make_unique<Zombie>(type, row, spawnX, laneY, 170.0f, 0.0f, 28.0f, 18.0f, Color {215, 165, 96, 255});
        case ZombieType::Summoner:
            return std::make_unique<Zombie>(type, row, spawnX, laneY, 235.0f, 0.0f, 11.0f, 16.0f, Color {148, 96, 200, 255});
        default: return std::make_unique<Zombie>(ZombieType::Basic, row, spawnX, laneY, 185.0f, 0.0f, 17.0f, 20.0f, Color {116, 145, 109, 255});
    }
}
